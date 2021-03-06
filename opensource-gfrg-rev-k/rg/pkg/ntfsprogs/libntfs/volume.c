/**
 * volume.c - NTFS volume handling code. Part of the Linux-NTFS project.
 *
 * Copyright (c) 2000-2006 Anton Altaparmakov
 * Copyright (c) 2002-2005 Szabolcs Szakacsits
 * Copyright (c) 2004-2005 Richard Russon
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-NTFS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include "volume.h"
#include "attrib.h"
#include "mft.h"
#include "bootsect.h"
#include "device.h"
#include "debug.h"
#include "inode.h"
#include "runlist.h"
#include "logfile.h"
#include "dir.h"
#include "logging.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/**
 * ntfs_volume_alloc - Create an NTFS volume object and initialise it
 *
 * Description...
 *
 * Returns:
 */
ntfs_volume *ntfs_volume_alloc(void)
{
	return calloc(1, sizeof(ntfs_volume));
}

/**
 * __ntfs_volume_release - Destroy an NTFS volume object
 * @v:
 *
 * Description...
 *
 * Returns:
 */
static void __ntfs_volume_release(ntfs_volume *v)
{
	if (v->lcnbmp_ni && NInoDirty(v->lcnbmp_ni))
		ntfs_inode_sync(v->lcnbmp_ni);
	if (v->vol_ni)
		ntfs_inode_close(v->vol_ni);
	if (v->lcnbmp_na)
		ntfs_attr_close(v->lcnbmp_na);
	if (v->lcnbmp_ni)
		ntfs_inode_close(v->lcnbmp_ni);
	if (v->mft_ni && NInoDirty(v->mft_ni))
		ntfs_inode_sync(v->mft_ni);
	if (v->mftbmp_na)
		ntfs_attr_close(v->mftbmp_na);
	if (v->mft_na)
		ntfs_attr_close(v->mft_na);
	if (v->mft_ni)
		ntfs_inode_close(v->mft_ni);
	if (v->mftmirr_ni && NInoDirty(v->mftmirr_ni))
		ntfs_inode_sync(v->mftmirr_ni);
	if (v->mftmirr_na)
		ntfs_attr_close(v->mftmirr_na);
	if (v->mftmirr_ni)
		ntfs_inode_close(v->mftmirr_ni);
	if (v->dev) {
		struct ntfs_device *dev = v->dev;

		if (NDevDirty(dev))
			dev->d_ops->sync(dev);
		if (dev->d_ops->close(dev))
			ntfs_log_perror("Eeek! Failed to close the device.  Error: ");
	}
	free(v->vol_name);
	free(v->upcase);
	free(v->attrdef);
	free(v);
}

/**
 * ntfs_mft_load - load the $MFT and setup the ntfs volume with it
 * @vol:	ntfs volume whose $MFT to load
 *
 * Load $MFT from @vol and setup @vol with it. After calling this function the
 * volume @vol is ready for use by all read access functions provided by the
 * ntfs library.
 *
 * Return 0 on success and -1 on error with errno set to the error code.
 */
static int ntfs_mft_load(ntfs_volume *vol)
{
	VCN next_vcn, last_vcn, highest_vcn;
	s64 l;
	MFT_RECORD *mb = NULL;
	ntfs_attr_search_ctx *ctx = NULL;
	ATTR_RECORD *a;
	int eo;

	/* Manually setup an ntfs_inode. */
	vol->mft_ni = ntfs_inode_allocate(vol);
	mb = (MFT_RECORD*)malloc(vol->mft_record_size);
	if (!vol->mft_ni || !mb) {
		ntfs_log_perror("Error allocating memory for $MFT");
		goto error_exit;
	}
	vol->mft_ni->mft_no = 0;
	vol->mft_ni->mrec = mb;
	/* Can't use any of the higher level functions yet! */
	l = ntfs_mst_pread(vol->dev, vol->mft_lcn << vol->cluster_size_bits, 1,
			vol->mft_record_size, mb);
	if (l != 1) {
		if (l != -1)
			errno = EIO;
		ntfs_log_perror("Error reading $MFT");
		goto error_exit;
	}
	if (ntfs_is_baad_record(mb->magic)) {
		ntfs_log_debug("Error: Incomplete multi sector transfer detected in "
				"$MFT.\n");
		goto io_error_exit;
	}
	if (!ntfs_is_mft_record(mb->magic)) {
		ntfs_log_debug("Error: $MFT has invalid magic.\n");
		goto io_error_exit;
	}
	ctx = ntfs_attr_get_search_ctx(vol->mft_ni, NULL);
	if (!ctx) {
		ntfs_log_perror("Failed to allocate attribute search context");
		goto error_exit;
	}
	if (p2n(ctx->attr) < p2n(mb) ||
			(char*)ctx->attr > (char*)mb + vol->mft_record_size) {
		ntfs_log_debug("Error: $MFT is corrupt.\n");
		goto io_error_exit;
	}
	/* Find the $ATTRIBUTE_LIST attribute in $MFT if present. */
	if (ntfs_attr_lookup(AT_ATTRIBUTE_LIST, AT_UNNAMED, 0, 0, 0, NULL, 0,
			ctx)) {
		if (errno != ENOENT) {
			ntfs_log_debug("Error: $MFT has corrupt attribute list.\n");
			goto io_error_exit;
		}
		goto mft_has_no_attr_list;
	}
	NInoSetAttrList(vol->mft_ni);
	l = ntfs_get_attribute_value_length(ctx->attr);
	if (l <= 0 || l > 0x40000) {
		ntfs_log_debug("Error: $MFT/$ATTRIBUTE_LIST has invalid length.\n");
		goto io_error_exit;
	}
	vol->mft_ni->attr_list_size = l;
	vol->mft_ni->attr_list = malloc(l);
	if (!vol->mft_ni->attr_list) {
		ntfs_log_debug("Error: failed to allocate buffer for attribute "
				"list.\n");
		goto error_exit;
	}
	l = ntfs_get_attribute_value(vol, ctx->attr, vol->mft_ni->attr_list);
	if (!l) {
		ntfs_log_debug("Error: failed to get value of "
				"$MFT/$ATTRIBUTE_LIST.\n");
		goto io_error_exit;
	}
	if (l != vol->mft_ni->attr_list_size) {
		ntfs_log_debug("Error: got unexpected amount of data when reading "
				"$MFT/$ATTRIBUTE_LIST.\n");
		goto io_error_exit;
	}
mft_has_no_attr_list:
	/* We now have a fully setup ntfs inode for $MFT in vol->mft_ni. */

	/* Get an ntfs attribute for $MFT/$DATA and set it up, too. */
	vol->mft_na = ntfs_attr_open(vol->mft_ni, AT_DATA, AT_UNNAMED, 0);
	if (!vol->mft_na) {
		ntfs_log_perror("Failed to open ntfs attribute");
		goto error_exit;
	}
	/* Read all extents from the $DATA attribute in $MFT. */
	ntfs_attr_reinit_search_ctx(ctx);
	last_vcn = vol->mft_na->allocated_size >> vol->cluster_size_bits;
	highest_vcn = next_vcn = 0;
	a = NULL;
	while (!ntfs_attr_lookup(AT_DATA, AT_UNNAMED, 0, 0, next_vcn, NULL, 0,
			ctx)) {
		runlist_element *nrl;

		a = ctx->attr;
		/* $MFT must be non-resident. */
		if (!a->non_resident) {
			ntfs_log_debug("$MFT must be non-resident but a resident "
					"extent was found. $MFT is corrupt. Run "
					"chkdsk.\n");
			goto io_error_exit;
		}
		/* $MFT must be uncompressed and unencrypted. */
		if (a->flags & ATTR_COMPRESSION_MASK ||
				a->flags & ATTR_IS_ENCRYPTED) {
			ntfs_log_debug("$MFT must be uncompressed and unencrypted "
					"but a compressed/encrypted extent was "
					"found. $MFT is corrupt. Run chkdsk.\n");
			goto io_error_exit;
		}
		/*
		 * Decompress the mapping pairs array of this extent and merge
		 * the result into the existing runlist. No need for locking
		 * as we have exclusive access to the inode at this time and we
		 * are a mount in progress task, too.
		 */
		nrl = ntfs_mapping_pairs_decompress(vol, a, vol->mft_na->rl);
		if (!nrl) {
			ntfs_log_perror("ntfs_mapping_pairs_decompress() failed");
			goto error_exit;
		}
		vol->mft_na->rl = nrl;

		/* Get the lowest vcn for the next extent. */
		highest_vcn = sle64_to_cpu(a->highest_vcn);
		next_vcn = highest_vcn + 1;

		/* Only one extent or error, which we catch below. */
		if (next_vcn <= 0)
			break;

		/* Avoid endless loops due to corruption. */
		if (next_vcn < sle64_to_cpu(a->lowest_vcn)) {
			ntfs_log_debug("$MFT has corrupt attribute list attribute. "
					"Run chkdsk.\n");
			goto io_error_exit;
		}
	}
	if (!a) {
		ntfs_log_debug("$MFT/$DATA attribute not found. $MFT is corrupt. Run "
				"chkdsk.\n");
		goto io_error_exit;
	}
	if (highest_vcn && highest_vcn != last_vcn - 1) {
		ntfs_log_debug("Failed to load the complete runlist for $MFT/$DATA. "
				"Bug or corrupt $MFT. Run chkdsk.\n");
		ntfs_log_debug("highest_vcn = 0x%llx, last_vcn - 1 = 0x%llx\n",
				(long long)highest_vcn, (long long)last_vcn - 1);
		goto io_error_exit;
	}
	/* Done with the $Mft mft record. */
	ntfs_attr_put_search_ctx(ctx);
	ctx = NULL;
	/*
	 * The volume is now setup so we can use all read access functions.
	 */
	vol->mftbmp_na = ntfs_attr_open(vol->mft_ni, AT_BITMAP, AT_UNNAMED, 0);
	if (!vol->mftbmp_na) {
		ntfs_log_perror("Failed to open $MFT/$BITMAP");
		goto error_exit;
	}
	return 0;
io_error_exit:
	errno = EIO;
error_exit:
	eo = errno;
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	if (vol->mft_na) {
		ntfs_attr_close(vol->mft_na);
		vol->mft_na = NULL;
	}
	if (vol->mft_ni) {
		ntfs_inode_close(vol->mft_ni);
		vol->mft_ni = NULL;
	}
	errno = eo;
	return -1;
}

/**
 * ntfs_mftmirr_load - load the $MFTMirr and setup the ntfs volume with it
 * @vol:	ntfs volume whose $MFTMirr to load
 *
 * Load $MFTMirr from @vol and setup @vol with it. After calling this function
 * the volume @vol is ready for use by all write access functions provided by
 * the ntfs library (assuming ntfs_mft_load() has been called successfully
 * beforehand).
 *
 * Return 0 on success and -1 on error with errno set to the error code.
 */
static int ntfs_mftmirr_load(ntfs_volume *vol)
{
	int i;
	runlist_element rl[2];

	vol->mftmirr_ni = ntfs_inode_open(vol, FILE_MFTMirr);
	if (!vol->mftmirr_ni) {
		ntfs_log_perror("Failed to open inode $MFTMirr");
		return -1;
	}
	/* Get an ntfs attribute for $MFTMirr/$DATA, too. */
	vol->mftmirr_na = ntfs_attr_open(vol->mftmirr_ni, AT_DATA, AT_UNNAMED, 0);
	if (!vol->mftmirr_na) {
		ntfs_log_perror("Failed to open $MFTMirr/$DATA");
		goto error_exit;
	}
	if (ntfs_attr_map_runlist(vol->mftmirr_na, 0) < 0) {
		ntfs_log_perror("Failed to map runlist of $MFTMirr/$DATA");
		goto error_exit;
	}
	/* Construct the mft mirror runlist. */
	rl[0].vcn = 0;
	rl[0].lcn = vol->mftmirr_lcn;
	rl[0].length = (vol->mftmirr_size * vol->mft_record_size +
			vol->cluster_size - 1) / vol->cluster_size;
	rl[1].vcn = rl[0].length;
	rl[1].lcn = LCN_ENOENT;
	rl[1].length = 0;
	/* Compare the two runlists. They must be identical. */
	i = 0;
	do {
		if (rl[i].vcn != vol->mftmirr_na->rl[i].vcn ||
				rl[i].lcn != vol->mftmirr_na->rl[i].lcn ||
				rl[i].length != vol->mftmirr_na->rl[i].length) {
			ntfs_log_debug("Error: $MFTMirr location mismatch! Run "
					"chkdsk.\n");
			errno = EIO;
			goto error_exit;
		}
	} while (rl[i++].length);
	return 0;
error_exit:
	i = errno;
	if (vol->mftmirr_na) {
		ntfs_attr_close(vol->mftmirr_na);
		vol->mftmirr_na = NULL;
	}
	ntfs_inode_close(vol->mftmirr_ni);
	vol->mftmirr_ni = NULL;
	errno = i;
	return -1;
}

/**
 * ntfs_volume_startup - allocate and setup an ntfs volume
 * @dev:	device to open
 * @flags:	optional mount flags
 *
 * Load, verify, and parse bootsector; load and setup $MFT and $MFTMirr. After
 * calling this function, the volume is setup sufficiently to call all read
 * and write access functions provided by the library.
 *
 * Return the allocated volume structure on success and NULL on error with
 * errno set to the error code.
 */
ntfs_volume *ntfs_volume_startup(struct ntfs_device *dev, unsigned long flags)
{
	LCN mft_zone_size, mft_lcn;
	s64 br;
	ntfs_volume *vol;
	NTFS_BOOT_SECTOR *bs;
	int eo;
#ifndef NTFS_DISABLE_DEBUG_LOGGING
	const char *OK = "OK\n";
	const char *FAILED = "FAILED\n";
	BOOL debug = 1;
#else
	BOOL debug = 0;
#endif

	if (!dev || !dev->d_ops || !dev->d_name) {
		errno = EINVAL;
		return NULL;
	}

	/* Allocate the boot sector structure. */
	if (!(bs = (NTFS_BOOT_SECTOR *)malloc(sizeof(NTFS_BOOT_SECTOR))))
		return NULL;
	/* Allocate the volume structure. */
	vol = ntfs_volume_alloc();
	if (!vol)
		goto error_exit;
	/* Create the default upcase table. */
	vol->upcase_len = 65536;
	vol->upcase = (ntfschar*)malloc(vol->upcase_len * sizeof(ntfschar));
	if (!vol->upcase) {
		ntfs_log_perror("Error allocating memory for upcase table.");
		goto error_exit;
	}
	ntfs_upcase_table_build(vol->upcase,
			vol->upcase_len * sizeof(ntfschar));
	if (flags & MS_RDONLY)
		NVolSetReadOnly(vol);
	if (flags & MS_NOATIME)
		NVolSetNoATime(vol);
	ntfs_log_debug("Reading bootsector... ");
	if (dev->d_ops->open(dev, NVolReadOnly(vol) ? O_RDONLY: O_RDWR)) {
		ntfs_log_debug(FAILED);
		ntfs_log_perror("Error opening partition device");
		goto error_exit;
	}
	/* Attach the device to the volume. */
	vol->dev = dev;
	/* Now read the bootsector. */
	br = ntfs_pread(dev, 0, sizeof(NTFS_BOOT_SECTOR), bs);
	if (br != sizeof(NTFS_BOOT_SECTOR)) {
		ntfs_log_debug(FAILED);
		if (br != -1)
			errno = EINVAL;
		if (!br)
			ntfs_log_debug("Error: partition is smaller than bootsector "
					"size. Weird!\n");
		else
			ntfs_log_perror("Error reading bootsector");
		goto error_exit;
	}
	ntfs_log_debug(OK);
	if (!ntfs_boot_sector_is_ntfs(bs, !debug)) {
		ntfs_log_debug("Error: %s is not a valid NTFS partition!\n",
				dev->d_name);
		errno = EINVAL;
		goto error_exit;
	}
	if (ntfs_boot_sector_parse(vol, bs) < 0) {
		ntfs_log_perror("Failed to parse ntfs bootsector");
		goto error_exit;
	}
	free(bs);
	bs = NULL;
	/* Now set the device block size to the sector size. */
	if (ntfs_device_block_size_set(vol->dev, vol->sector_size))
		ntfs_log_debug("Failed to set the device block size to the "
				"sector size.  This may affect performance "
				"but should be harmless otherwise.  Error: "
				"%s\n", strerror(errno));
	/*
	 * We now initialize the cluster allocator.
	 *
	 * FIXME: Move this to its own function? (AIA)
	 */

	// TODO: Make this tunable at mount time. (AIA)
	vol->mft_zone_multiplier = 1;

	/* Determine the size of the MFT zone. */
	mft_zone_size = vol->nr_clusters;
	switch (vol->mft_zone_multiplier) {  /* % of volume size in clusters */
	case 4:
		mft_zone_size >>= 1;			/* 50%   */
		break;
	case 3:
		mft_zone_size = mft_zone_size * 3 >> 3;	/* 37.5% */
		break;
	case 2:
		mft_zone_size >>= 2;			/* 25%   */
		break;
	/* case 1: */
	default:
		mft_zone_size >>= 3;			/* 12.5% */
		break;
	}

	/* Setup the mft zone. */
	vol->mft_zone_start = vol->mft_zone_pos = vol->mft_lcn;
	ntfs_log_debug("mft_zone_pos = 0x%llx\n", (long long)vol->mft_zone_pos);

	/*
	 * Calculate the mft_lcn for an unmodified NTFS volume (see mkntfs
	 * source) and if the actual mft_lcn is in the expected place or even
	 * further to the front of the volume, extend the mft_zone to cover the
	 * beginning of the volume as well. This is in order to protect the
	 * area reserved for the mft bitmap as well within the mft_zone itself.
	 * On non-standard volumes we don't protect it as the overhead would be
	 * higher than the speed increase we would get by doing it.
	 */
	mft_lcn = (8192 + 2 * vol->cluster_size - 1) / vol->cluster_size;
	if (mft_lcn * vol->cluster_size < 16 * 1024)
		mft_lcn = (16 * 1024 + vol->cluster_size - 1) /
				vol->cluster_size;
	if (vol->mft_zone_start <= mft_lcn)
		vol->mft_zone_start = 0;
	ntfs_log_debug("mft_zone_start = 0x%llx\n", (long long)vol->mft_zone_start);

	/*
	 * Need to cap the mft zone on non-standard volumes so that it does
	 * not point outside the boundaries of the volume. We do this by
	 * halving the zone size until we are inside the volume.
	 */
	vol->mft_zone_end = vol->mft_lcn + mft_zone_size;
	while (vol->mft_zone_end >= vol->nr_clusters) {
		mft_zone_size >>= 1;
		vol->mft_zone_end = vol->mft_lcn + mft_zone_size;
	}
	ntfs_log_debug("mft_zone_end = 0x%llx\n", (long long)vol->mft_zone_end);

	/*
	 * Set the current position within each data zone to the start of the
	 * respective zone.
	 */
	vol->data1_zone_pos = vol->mft_zone_end;
	ntfs_log_debug("data1_zone_pos = 0x%llx\n", vol->data1_zone_pos);
	vol->data2_zone_pos = 0;
	ntfs_log_debug("data2_zone_pos = 0x%llx\n", vol->data2_zone_pos);

	/* Set the mft data allocation position to mft record 24. */
	vol->mft_data_pos = 24;

	/*
	 * The cluster allocator is now fully operational.
	 */

	/* Need to setup $MFT so we can use the library read functions. */
	ntfs_log_debug("Loading $MFT... ");
	if (ntfs_mft_load(vol) < 0) {
		ntfs_log_debug(FAILED);
		ntfs_log_perror("Failed to load $MFT");
		goto error_exit;
	}
	ntfs_log_debug(OK);

	/* Need to setup $MFTMirr so we can use the write functions, too. */
	ntfs_log_debug("Loading $MFTMirr... ");
	if (ntfs_mftmirr_load(vol) < 0) {
		ntfs_log_debug(FAILED);
		ntfs_log_perror("Failed to load $MFTMirr");
		goto error_exit;
	}
	ntfs_log_debug(OK);
	return vol;
error_exit:
	eo = errno;
	free(bs);
	if (vol)
		__ntfs_volume_release(vol);
	errno = eo;
	return NULL;
}

/**
 * ntfs_volume_check_logfile - check logfile on target volume
 * @vol:	volume on which to check logfile
 *
 * Return 0 on success and -1 on error with errno set error code.
 */
static int ntfs_volume_check_logfile(ntfs_volume *vol)
{
	ntfs_inode *ni;
	ntfs_attr *na = NULL;
	RESTART_PAGE_HEADER *rp = NULL;
	int err = 0;

	if ((ni = ntfs_inode_open(vol, FILE_LogFile)) == NULL) {
		ntfs_log_debug("Failed to open inode FILE_LogFile.\n");
		errno = EIO;
		return -1;
	}
	if ((na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0)) == NULL) {
		ntfs_log_debug("Failed to open $FILE_LogFile/$DATA\n");
		err = EIO;
		goto exit;
	}
	if (!ntfs_check_logfile(na, &rp) || !ntfs_is_logfile_clean(na, rp))
		err = EOPNOTSUPP;
	free(rp);
exit:
	if (na)
		ntfs_attr_close(na);
	ntfs_inode_close(ni);
	if (err) {
		errno = err;
		return -1;
	}
	return 0;
}

/**
 * ntfs_hiberfile_open - Find and open '/hiberfil.sys'
 * @vol:    An ntfs volume obtained from ntfs_mount
 *
 * Return:  inode  Success, hiberfil.sys is valid
 *	    NULL   hiberfil.sys doesn't exist or some other error occurred
 */
static ntfs_inode *ntfs_hiberfile_open(ntfs_volume *vol)
{
	u64 inode;
	ntfs_inode *ni_root;
	ntfs_inode *ni_hibr = NULL;
	ntfschar   *unicode = NULL;
	int unicode_len;
	const char *hiberfile = "hiberfil.sys";

	if (!vol) {
		errno = EINVAL;
		return NULL;
	}

	ni_root = ntfs_inode_open(vol, FILE_root);
	if (!ni_root) {
		ntfs_log_debug("Couldn't open the root directory.\n");
		return NULL;
	}

	unicode_len = ntfs_mbstoucs(hiberfile, &unicode, 0);
	if (unicode_len < 0) {
		ntfs_log_perror("Couldn't convert 'hiberfil.sys' to Unicode");
		goto out;
	}

	inode = ntfs_inode_lookup_by_name(ni_root, unicode, unicode_len);
	if (inode == (u64)-1) {
		ntfs_log_debug("Couldn't find file '%s'.\n", hiberfile);
		goto out;
	}

	inode = MREF(inode);
	ni_hibr = ntfs_inode_open(vol, inode);
	if (!ni_hibr) {
		ntfs_log_debug("Couldn't open inode %lld.\n", (long long)inode);
		goto out;
	}
out:
	ntfs_inode_close(ni_root);
	free(unicode);
	return ni_hibr;
}


#define NTFS_HIBERFILE_HEADER_SIZE	4096

/**
 * ntfs_volume_check_hiberfile - check hiberfil.sys whether Windows is
 *                               hibernated on the target volume
 * @vol:    volume on which to check hiberfil.sys
 *
 * Return:  0 if Windows isn't hibernated for sure
 *         -1 otherwise and errno is set to the appropriate value
 */
static int ntfs_volume_check_hiberfile(ntfs_volume *vol)
{
	ntfs_inode *ni;
	ntfs_attr *na = NULL;
	int i, bytes_read, ret = -1;
	char *buf = NULL;

	ni = ntfs_hiberfile_open(vol);
	if (!ni) {
		if (errno == ENOENT)
			return 0;
		return -1;
	}

	buf = malloc(NTFS_HIBERFILE_HEADER_SIZE);
	if (!buf) {
		ntfs_log_perror("Error allocating memory for hiberfile.sys header");
		goto out;
	}

	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
	if (!na) {
		ntfs_log_perror("Failed to open hiberfil.sys data attribute");
		goto out;
	}

	bytes_read = ntfs_attr_pread(na, 0, NTFS_HIBERFILE_HEADER_SIZE, buf);
	if (bytes_read == -1) {
		ntfs_log_perror("Failed to read hiberfil.sys");
		goto out;
	}
	if (bytes_read < NTFS_HIBERFILE_HEADER_SIZE) {
		ntfs_log_debug("Hibernated non-system partition, refused to "
				"mount!\n");
		errno = EPERM;
		goto out;
	}
	if (memcmp(buf, "hibr", 4) == 0) {
		ntfs_log_debug("Windows is hibernated, refused to mount!\n");
		errno = EPERM;
		goto out;
	}
	for (i = 0; i < NTFS_HIBERFILE_HEADER_SIZE; i++) {
		if (buf[i]) {
			ntfs_log_debug("Windows is hibernated, won't mount!\n");
			errno = EPERM;
			goto out;
		}
	}
        /* All right, all header bytes are zero */
	ret = 0;
out:
	if (na)
		ntfs_attr_close(na);
	free(buf);
	ntfs_inode_close(ni);
	return ret;
}

/**
 * ntfs_device_mount - open ntfs volume
 * @dev:	device to open
 * @flags:	optional mount flags
 *
 * This function mounts an ntfs volume. @dev should describe the device which
 * to mount as the ntfs volume.
 *
 * @flags is an optional second parameter. The same flags are used as for
 * the mount system call (man 2 mount). Currently only the following flags
 * are implemented:
 *	MS_RDONLY	- mount volume read-only
 *	MS_NOATIME	- do not update access time
 *
 * The function opens the device @dev and verifies that it contains a valid
 * bootsector. Then, it allocates an ntfs_volume structure and initializes
 * some of the values inside the structure from the information stored in the
 * bootsector. It proceeds to load the necessary system files and completes
 * setting up the structure.
 *
 * Return the allocated volume structure on success and NULL on error with
 * errno set to the error code.
 */
ntfs_volume *ntfs_device_mount(struct ntfs_device *dev, unsigned long flags)
{
	s64 l;
#ifndef NTFS_DISABLE_DEBUG_LOGGING
	const char *OK = "OK\n";
	const char *FAILED = "FAILED\n";
#endif
	ntfs_volume *vol;
	u8 *m = NULL, *m2 = NULL;
	ntfs_attr_search_ctx *ctx = NULL;
	ntfs_inode *ni;
	ntfs_attr *na;
	ATTR_RECORD *a;
	VOLUME_INFORMATION *vinf;
	ntfschar *vname;
	int i, j, eo;
	u32 u;

	vol = ntfs_volume_startup(dev, flags);
	if (!vol) {
		ntfs_log_perror("Failed to startup volume");
		return NULL;
	}

	/* Load data from $MFT and $MFTMirr and compare the contents. */
	m = (u8*)malloc(vol->mftmirr_size << vol->mft_record_size_bits);
	m2 = (u8*)malloc(vol->mftmirr_size << vol->mft_record_size_bits);
	if (!m || !m2) {
		ntfs_log_perror("Failed to allocate memory");
		goto error_exit;
	}

	l = ntfs_attr_mst_pread(vol->mft_na, 0, vol->mftmirr_size,
			vol->mft_record_size, m);
	if (l != vol->mftmirr_size) {
		if (l == -1)
			ntfs_log_perror("Failed to read $MFT");
		else {
			ntfs_log_debug("Failed to read $MFT, unexpected length "
				       "(%d != %lld).\n", vol->mftmirr_size, l);
			errno = EIO;
		}
		goto error_exit;
	}
	l = ntfs_attr_mst_pread(vol->mftmirr_na, 0, vol->mftmirr_size,
			vol->mft_record_size, m2);
	if (l != vol->mftmirr_size) {
		if (l == -1)
			ntfs_log_perror("Failed to read $MFTMirr");
		else {
			ntfs_log_debug("Failed to read $MFTMirr, unexpected "
				       "length (%d != %lld).\n", 
				       vol->mftmirr_size, l);
			errno = EIO;
		}
		goto error_exit;
	}
	ntfs_log_debug("Comparing $MFTMirr to $MFT... ");
	for (i = 0; i < vol->mftmirr_size; ++i) {
		MFT_RECORD *mrec, *mrec2;
		const char *ESTR[12] = { "$MFT", "$MFTMirr", "$LogFile",
			"$Volume", "$AttrDef", "root directory", "$Bitmap",
			"$Boot", "$BadClus", "$Secure", "$UpCase", "$Extend" };
		const char *s;

		if (i < 12)
			s = ESTR[i];
		else if (i < 16)
			s = "system file";
		else
			s = "mft record";

		mrec = (MFT_RECORD*)(m + i * vol->mft_record_size);
		if (mrec->flags & MFT_RECORD_IN_USE) {
			if (ntfs_is_baad_recordp(mrec)) {
				ntfs_log_debug("FAILED\n");
				ntfs_log_debug("$MFT error: Incomplete multi "
						"sector transfer detected in "
						"%s.\n", s);
				goto io_error_exit;
			}
			if (!ntfs_is_mft_recordp(mrec)) {
				ntfs_log_debug("FAILED\n");
				ntfs_log_debug("$MFT error: Invalid mft "
						"record for %s.\n", s);
				goto io_error_exit;
			}
		}
		mrec2 = (MFT_RECORD*)(m2 + i * vol->mft_record_size);
		if (mrec2->flags & MFT_RECORD_IN_USE) {
			if (ntfs_is_baad_recordp(mrec2)) {
				ntfs_log_debug("FAILED\n");
				ntfs_log_debug("$MFTMirr error: Incomplete "
						"multi sector transfer "
						"detected in %s.\n", s);
				goto io_error_exit;
			}
			if (!ntfs_is_mft_recordp(mrec2)) {
				ntfs_log_debug("FAILED\n");
				ntfs_log_debug("$MFTMirr error: Invalid mft "
						"record for %s.\n", s);
				goto io_error_exit;
			}
		}
		if (memcmp(mrec, mrec2, ntfs_mft_record_get_data_size(mrec))) {
			ntfs_log_debug(FAILED);
			ntfs_log_debug("$MFTMirr does not match $MFT. Run "
					"chkdsk.\n");
			goto io_error_exit;
		}
	}
	ntfs_log_debug(OK);

	free(m2);
	free(m);
	m = m2 = NULL;

	/* Now load the bitmap from $Bitmap. */
	ntfs_log_debug("Loading $Bitmap... ");
	vol->lcnbmp_ni = ntfs_inode_open(vol, FILE_Bitmap);
	if (!vol->lcnbmp_ni) {
		ntfs_log_debug(FAILED);
		ntfs_log_perror("Failed to open inode");
		goto error_exit;
	}
	/* Get an ntfs attribute for $Bitmap/$DATA. */
	vol->lcnbmp_na = ntfs_attr_open(vol->lcnbmp_ni, AT_DATA, AT_UNNAMED, 0);
	if (!vol->lcnbmp_na) {
		ntfs_log_debug(FAILED);
		ntfs_log_perror("Failed to open ntfs attribute");
		goto error_exit;
	}
	/* Done with the $Bitmap mft record. */
	ntfs_log_debug(OK);

	/* Now load the upcase table from $UpCase. */
	ntfs_log_debug("Loading $UpCase... ");
	ni = ntfs_inode_open(vol, FILE_UpCase);
	if (!ni) {
		ntfs_log_debug(FAILED);
		ntfs_log_perror("Failed to open inode");
		goto error_exit;
	}
	/* Get an ntfs attribute for $UpCase/$DATA. */
	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
	if (!na) {
		ntfs_log_debug(FAILED);
		ntfs_log_perror("Failed to open ntfs attribute");
		goto error_exit;
	}
	/*
	 * Note: Normally, the upcase table has a length equal to 65536
	 * 2-byte Unicode characters but allow for different cases, so no
	 * checks done. Just check we don't overflow 32-bits worth of Unicode
	 * characters.
	 */
	if (na->data_size & ~0x1ffffffffULL) {
		ntfs_log_debug(FAILED);
		ntfs_log_debug("Error: Upcase table is too big (max 32-bit "
				"allowed).\n");
		errno = EINVAL;
		goto error_exit;
	}
	if (vol->upcase_len != na->data_size >> 1) {
		vol->upcase_len = na->data_size >> 1;
		/* Throw away default table. */
		free(vol->upcase);
		vol->upcase = (ntfschar*)malloc(na->data_size);
		if (!vol->upcase) {
			ntfs_log_debug(FAILED);
			ntfs_log_debug("Not enough memory to load $UpCase.\n");
			goto error_exit;
		}
	}
	/* Read in the $DATA attribute value into the buffer. */
	l = ntfs_attr_pread(na, 0, na->data_size, vol->upcase);
	if (l != na->data_size) {
		ntfs_log_debug(FAILED);
		ntfs_log_debug("Amount of data read does not correspond to expected "
				"length!\n");
		errno = EIO;
		goto error_exit;
	}
	/* Done with the $UpCase mft record. */
	ntfs_log_debug(OK);
	ntfs_attr_close(na);
	if (ntfs_inode_close(ni))
		ntfs_log_perror("Failed to close inode, leaking memory");

	/*
	 * Now load $Volume and set the version information and flags in the
	 * vol structure accordingly.
	 */
	ntfs_log_debug("Loading $Volume... ");
	vol->vol_ni = ntfs_inode_open(vol, FILE_Volume);
	if (!vol->vol_ni) {
		ntfs_log_debug(FAILED);
		ntfs_log_perror("Failed to open inode");
		goto error_exit;
	}
	/* Get a search context for the $Volume/$VOLUME_INFORMATION lookup. */
	ctx = ntfs_attr_get_search_ctx(vol->vol_ni, NULL);
	if (!ctx) {
		ntfs_log_debug(FAILED);
		ntfs_log_perror("Failed to allocate attribute search context");
		goto error_exit;
	}
	/* Find the $VOLUME_INFORMATION attribute. */
	if (ntfs_attr_lookup(AT_VOLUME_INFORMATION, AT_UNNAMED, 0, 0, 0, NULL,
			0, ctx)) {
		ntfs_log_debug(FAILED);
		ntfs_log_debug("$VOLUME_INFORMATION attribute not found in "
				"$Volume?!?\n");
		goto error_exit;
	}
	a = ctx->attr;
	/* Has to be resident. */
	if (a->non_resident) {
		ntfs_log_debug(FAILED);
		ntfs_log_debug("Error: Attribute $VOLUME_INFORMATION must be "
				"resident (and it isn't)!\n");
		errno = EIO;
		goto error_exit;
	}
	/* Get a pointer to the value of the attribute. */
	vinf = (VOLUME_INFORMATION*)(le16_to_cpu(a->value_offset) + (char*)a);
	/* Sanity checks. */
	if ((char*)vinf + le32_to_cpu(a->value_length) > (char*)ctx->mrec +
			le32_to_cpu(ctx->mrec->bytes_in_use) ||
			le16_to_cpu(a->value_offset) + le32_to_cpu(
			a->value_length) > le32_to_cpu(a->length)) {
		ntfs_log_debug(FAILED);
		ntfs_log_debug("Error: Attribute $VOLUME_INFORMATION in $Volume is "
				"corrupt!\n");
		errno = EIO;
		goto error_exit;
	}
	/* Setup vol from the volume information attribute value. */
	vol->major_ver = vinf->major_ver;
	vol->minor_ver = vinf->minor_ver;
	/* Do not use le16_to_cpu() macro here as our VOLUME_FLAGS are
	   defined using cpu_to_le16() macro and hence are consistent. */
	vol->flags = vinf->flags;
	/*
	 * Reinitialize the search context for the $Volume/$VOLUME_NAME lookup.
	 */
	ntfs_attr_reinit_search_ctx(ctx);
	if (ntfs_attr_lookup(AT_VOLUME_NAME, AT_UNNAMED, 0, 0, 0, NULL, 0,
			ctx)) {
		if (errno != ENOENT) {
			ntfs_log_debug(FAILED);
			ntfs_log_debug("Error: Lookup of $VOLUME_NAME attribute in "
					"$Volume failed.  This probably means "
					"something is corrupt.  Run chkdsk.\n");
			goto error_exit;
		}
		/*
		 * Attribute not present.  This has been seen in the field.
		 * Treat this the same way as if the attribute was present but
		 * had zero length.
		 */
		vol->vol_name = malloc(1);
		if (!vol->vol_name) {
			ntfs_log_debug(FAILED);
			ntfs_log_debug("Error: Unable to allocate memory for volume "
					"name!\n");
			goto error_exit;
		}
		vol->vol_name[0] = '\0';
	} else {
		a = ctx->attr;
		/* Has to be resident. */
		if (a->non_resident) {
			ntfs_log_debug(FAILED);
			ntfs_log_debug("Error: Attribute $VOLUME_NAME must be "
					"resident!\n");
			errno = EIO;
			goto error_exit;
		}
		/* Get a pointer to the value of the attribute. */
		vname = (ntfschar*)(le16_to_cpu(a->value_offset) + (char*)a);
		u = le32_to_cpu(a->value_length) / 2;
		/*
		 * Convert Unicode volume name to current locale multibyte
		 * format.
		 */
		vol->vol_name = NULL;
		if (ntfs_ucstombs(vname, u, &vol->vol_name, 0) == -1) {
			ntfs_log_perror("Error: Volume name could not be converted "
					"to current locale");
			ntfs_log_debug("Forcing name into ASCII by replacing "
				"non-ASCII characters with underscores.\n");
			vol->vol_name = malloc(u + 1);
			if (!vol->vol_name) {
				ntfs_log_debug(FAILED);
				ntfs_log_debug("Error: Unable to allocate memory for "
						"volume name!\n");
				goto error_exit;
			}
			for (j = 0; j < (s32)u; j++) {
				ntfschar uc = le16_to_cpu(vname[j]);
				if (uc > 0xff)
					uc = (ntfschar)'_';
				vol->vol_name[j] = (char)uc;
			}
			vol->vol_name[u] = '\0';
		}
	}
	ntfs_log_debug(OK);
	ntfs_attr_put_search_ctx(ctx);
	ctx = NULL;
	/* Now load the attribute definitions from $AttrDef. */
	ntfs_log_debug("Loading $AttrDef... ");
	ni = ntfs_inode_open(vol, FILE_AttrDef);
	if (!ni) {
		ntfs_log_debug(FAILED);
		ntfs_log_perror("Failed to open inode");
		goto error_exit;
	}
	/* Get an ntfs attribute for $AttrDef/$DATA. */
	na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0);
	if (!na) {
		ntfs_log_debug(FAILED);
		ntfs_log_perror("Failed to open ntfs attribute");
		goto error_exit;
	}
	/* Check we don't overflow 32-bits. */
	if (na->data_size > 0xffffffffLL) {
		ntfs_log_debug(FAILED);
		ntfs_log_debug("Error: Attribute definition table is too big (max "
				"32-bit allowed).\n");
		errno = EINVAL;
		goto error_exit;
	}
	vol->attrdef_len = na->data_size;
	vol->attrdef = (ATTR_DEF*)malloc(na->data_size);
	if (!vol->attrdef) {
		ntfs_log_debug(FAILED);
		ntfs_log_debug("Not enough memory to load $AttrDef.\n");
		goto error_exit;
	}
	/* Read in the $DATA attribute value into the buffer. */
	l = ntfs_attr_pread(na, 0, na->data_size, vol->attrdef);
	if (l != na->data_size) {
		ntfs_log_debug(FAILED);
		ntfs_log_debug("Amount of data read does not correspond to expected "
				"length!\n");
		errno = EIO;
		goto error_exit;
	}
	/* Done with the $AttrDef mft record. */
	ntfs_log_debug(OK);
	ntfs_attr_close(na);
	if (ntfs_inode_close(ni))
		ntfs_log_perror("Failed to close inode, leaking memory");
	/*
	 * Check for dirty logfile and hibernated Windows.
	 * We care only about read-write mounts.
	 */
	if (!(flags & MS_RDONLY)) {
		if (ntfs_volume_check_logfile(vol) < 0)
			goto error_exit;
		if (ntfs_volume_check_hiberfile(vol) < 0)
			goto error_exit;
	}

	return vol;
io_error_exit:
	errno = EIO;
error_exit:
	eo = errno;
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	free(m);
	free(m2);
	__ntfs_volume_release(vol);
	errno = eo;
	return NULL;
}

/**
 * ntfs_mount - open ntfs volume
 * @name:	name of device/file to open
 * @flags:	optional mount flags
 *
 * This function mounts an ntfs volume. @name should contain the name of the
 * device/file to mount as the ntfs volume.
 *
 * @flags is an optional second parameter. The same flags are used as for
 * the mount system call (man 2 mount). Currently only the following flags
 * are implemented:
 *	MS_RDONLY	- mount volume read-only
 *	MS_NOATIME	- do not update access time
 *
 * The function opens the device or file @name and verifies that it contains a
 * valid bootsector. Then, it allocates an ntfs_volume structure and initializes
 * some of the values inside the structure from the information stored in the
 * bootsector. It proceeds to load the necessary system files and completes
 * setting up the structure.
 *
 * Return the allocated volume structure on success and NULL on error with
 * errno set to the error code.
 *
 * Note, that a copy is made of @name, and hence it can be discarded as
 * soon as the function returns.
 */
ntfs_volume *ntfs_mount(const char *name __attribute__((unused)),
		unsigned long flags __attribute__((unused)))
{
#ifndef NO_NTFS_DEVICE_DEFAULT_IO_OPS
	struct ntfs_device *dev;
	ntfs_volume *vol;

	/* Allocate an ntfs_device structure. */
	dev = ntfs_device_alloc(name, 0, &ntfs_device_default_io_ops, NULL);
	if (!dev)
		return NULL;
	/* Call ntfs_device_mount() to do the actual mount. */
	vol = ntfs_device_mount(dev, flags);
	if (!vol) {
		int eo = errno;
		ntfs_device_free(dev);
		errno = eo;
	}
	return vol;
#else
	/*
	 * ntfs_mount() makes no sense if NO_NTFS_DEVICE_DEFAULT_IO_OPS is
	 * defined as there are no device operations available in libntfs in
	 * this case.
	 */
	errno = EOPNOTSUPP;
	return NULL;
#endif
}

/**
 * ntfs_device_umount - close ntfs volume
 * @vol: address of ntfs_volume structure of volume to close
 * @force: if true force close the volume even if it is busy
 *
 * Deallocate all structures (including @vol itself) associated with the ntfs
 * volume @vol.
 *
 * Note it is up to the caller to destroy the device associated with the volume
 * being unmounted after this function returns.
 *
 * Return 0 on success. On error return -1 with errno set appropriately
 * (most likely to one of EAGAIN, EBUSY or EINVAL). The EAGAIN error means that
 * an operation is in progress and if you try the close later the operation
 * might be completed and the close succeed.
 *
 * If @force is true (i.e. not zero) this function will close the volume even
 * if this means that data might be lost.
 *
 * @vol must have previously been returned by a call to ntfs_device_mount().
 *
 * @vol itself is deallocated and should no longer be dereferenced after this
 * function returns success. If it returns an error then nothing has been done
 * so it is safe to continue using @vol.
 */
int ntfs_device_umount(ntfs_volume *vol,
		const BOOL force __attribute__((unused)))
{
	if (!vol) {
		errno = EINVAL;
		return -1;
	}
	__ntfs_volume_release(vol);
	return 0;
}

/**
 * ntfs_umount - close ntfs volume
 * @vol: address of ntfs_volume structure of volume to close
 * @force: if true force close the volume even if it is busy
 *
 * Deallocate all structures (including @vol itself) associated with the ntfs
 * volume @vol.
 *
 * Return 0 on success. On error return -1 with errno set appropriately
 * (most likely to one of EAGAIN, EBUSY or EINVAL). The EAGAIN error means that
 * an operation is in progress and if you try the close later the operation
 * might be completed and the close succeed.
 *
 * If @force is true (i.e. not zero) this function will close the volume even
 * if this means that data might be lost.
 *
 * @vol must have previously been returned by a call to ntfs_mount().
 *
 * @vol itself is deallocated and should no longer be dereferenced after this
 * function returns success. If it returns an error then nothing has been done
 * so it is safe to continue using @vol.
 */
int ntfs_umount(ntfs_volume *vol,
		const BOOL force __attribute__((unused)))
{
	struct ntfs_device *dev;

	if (!vol) {
		errno = EINVAL;
		return -1;
	}
	dev = vol->dev;
	__ntfs_volume_release(vol);
	ntfs_device_free(dev);
	return 0;
}

#ifdef HAVE_MNTENT_H

#ifndef HAVE_REALPATH
/**
 * realpath - If there is no realpath on the system
 */
static char *realpath(const char *path, char *resolved_path)
{
	strncpy(resolved_path, path, PATH_MAX);
	resolved_path[PATH_MAX] = '\0';
	return resolved_path;
}
#endif

/**
 * ntfs_mntent_check - desc
 *
 * If you are wanting to use this, you actually wanted to use
 * ntfs_check_if_mounted(), you just didn't realize. (-:
 *
 * See description of ntfs_check_if_mounted(), below.
 */
static int ntfs_mntent_check(const char *file, unsigned long *mnt_flags)
{
	struct mntent *mnt;
	char *real_file = NULL, *real_fsname = NULL;
	FILE *f;
	int err = 0;

	real_file = malloc(PATH_MAX + 1);
	if (!real_file)
		return -1;
	real_fsname = malloc(PATH_MAX + 1);
	if (!real_fsname) {
		err = errno;
		goto exit;
	}
	if (!realpath(file, real_file)) {
		err = errno;
		goto exit;
	}
	if (!(f = setmntent(MOUNTED, "r"))) {
		err = errno;
		goto exit;
	}
	while ((mnt = getmntent(f))) {
		if (!realpath(mnt->mnt_fsname, real_fsname))
			continue;
		if (!strcmp(real_file, real_fsname))
			break;
	}
	endmntent(f);
	if (!mnt)
		goto exit;
	*mnt_flags = NTFS_MF_MOUNTED;
	if (!strcmp(mnt->mnt_dir, "/"))
		*mnt_flags |= NTFS_MF_ISROOT;
#ifdef HAVE_HASMNTOPT
	if (hasmntopt(mnt, "ro") && !hasmntopt(mnt, "rw"))
		*mnt_flags |= NTFS_MF_READONLY;
#endif
exit:
	free(real_file);
	free(real_fsname);
	if (err) {
		errno = err;
		return -1;
	}
	return 0;
}
#endif /* HAVE_MNTENT_H */

/**
 * ntfs_check_if_mounted - check if an ntfs volume is currently mounted
 * @file:	device file to check
 * @mnt_flags:	pointer into which to return the ntfs mount flags (see volume.h)
 *
 * If the running system does not support the {set,get,end}mntent() calls,
 * just return 0 and set *@mnt_flags to zero.
 *
 * When the system does support the calls, ntfs_check_if_mounted() first tries
 * to find the device @file in /etc/mtab (or wherever this is kept on the
 * running system). If it is not found, assume the device is not mounted and
 * return 0 and set *@mnt_flags to zero.
 *
 * If the device @file is found, set the NTFS_MF_MOUNTED flags in *@mnt_flags.
 *
 * Further if @file is mounted as the file system root ("/"), set the flag
 * NTFS_MF_ISROOT in *@mnt_flags.
 *
 * Finally, check if the file system is mounted read-only, and if so set the
 * NTFS_MF_READONLY flag in *@mnt_flags.
 *
 * On success return 0 with *@mnt_flags set to the ntfs mount flags.
 *
 * On error return -1 with errno set to the error code.
 */
int ntfs_check_if_mounted(const char *file __attribute__((unused)),
		unsigned long *mnt_flags)
{
	*mnt_flags = 0;
#ifdef HAVE_MNTENT_H
	return ntfs_mntent_check(file, mnt_flags);
#else
	return 0;
#endif
}

/**
 * ntfs_version_is_supported - check if NTFS version is supported.
 * @vol:	ntfs volume whose version we're interested in.
 *
 * The function checks if the NTFS volume version is known or not.
 * Version 1.1 and 1.2 are used by Windows NT3.x and NT4.
 * Version 2.x is used by Windows 2000 Betas.
 * Version 3.0 is used by Windows 2000.
 * Version 3.1 is used by Windows XP, Windows Server 2003 and Longhorn.
 *
 * Return 0 if NTFS version is supported otherwise -1 with errno set.
 *
 * The following error codes are defined:
 *	EOPNOTSUPP - Unknown NTFS version
 *	EINVAL	   - Invalid argument
 */
int ntfs_version_is_supported(ntfs_volume *vol)
{
	u8 major, minor;

	if (!vol) {
		errno = EINVAL;
		return -1;
	}

	major = vol->major_ver;
	minor = vol->minor_ver;

	if (NTFS_V1_1(major, minor) || NTFS_V1_2(major, minor))
		return 0;

	if (NTFS_V2_X(major, minor))
		return 0;

	if (NTFS_V3_0(major, minor) || NTFS_V3_1(major, minor))
		return 0;

	errno = EOPNOTSUPP;
	return -1;
}

/**
 * ntfs_logfile_reset - "empty" $LogFile data attribute value
 * @vol:	ntfs volume whose $LogFile we intend to reset.
 *
 * Fill the value of the $LogFile data attribute, i.e. the contents of
 * the file, with 0xff's, thus marking the journal as empty.
 *
 * FIXME(?): We might need to zero the LSN field of every single mft
 * record as well. (But, first try without doing that and see what
 * happens, since chkdsk might pickup the pieces and do it for us...)
 *
 * On success return 0.
 *
 * On error return -1 with errno set to the error code.
 */
int ntfs_logfile_reset(ntfs_volume *vol)
{
	ntfs_inode *ni;
	ntfs_attr *na;
	int eo;

	if (!vol) {
		errno = EINVAL;
		return -1;
	}

	if ((ni = ntfs_inode_open(vol, FILE_LogFile)) == NULL) {
		ntfs_log_perror("Failed to open inode FILE_LogFile.");
		return -1;
	}

	if ((na = ntfs_attr_open(ni, AT_DATA, AT_UNNAMED, 0)) == NULL) {
		eo = errno;
		ntfs_log_perror("Failed to open $FILE_LogFile/$DATA");
		goto error_exit;
	}

	if (ntfs_empty_logfile(na)) {
		eo = errno;
		ntfs_log_perror("Failed to empty $FILE_LogFile/$DATA");
		ntfs_attr_close(na);
		goto error_exit;
	}
	ntfs_attr_close(na);
	return ntfs_inode_close(ni);

error_exit:
	ntfs_inode_close(ni);
	errno = eo;
	return -1;
}

/**
 * ntfs_volume_write_flags - set the flags of an ntfs volume
 * @vol:	ntfs volume where we set the volume flags
 * @flags:	new flags
 *
 * Set the on-disk volume flags in the mft record of $Volume and
 * on volume @vol to @flags.
 *
 * Return 0 if successful and -1 if not with errno set to the error code.
 */
int ntfs_volume_write_flags(ntfs_volume *vol, const u16 flags)
{
	ATTR_RECORD *a;
	VOLUME_INFORMATION *c;
	ntfs_attr_search_ctx *ctx;
	int ret = -1;	/* failure */

	if (!vol || !vol->vol_ni) {
		errno = EINVAL;
		return -1;
	}
	/* Get a pointer to the volume information attribute. */
	ctx = ntfs_attr_get_search_ctx(vol->vol_ni, NULL);
	if (!ctx) {
		ntfs_log_perror("Failed to allocate attribute search context");
		return -1;
	}
	if (ntfs_attr_lookup(AT_VOLUME_INFORMATION, AT_UNNAMED, 0, 0, 0, NULL,
			0, ctx)) {
		ntfs_log_debug("Error: Attribute $VOLUME_INFORMATION was not found "
				"in $Volume!\n");
		goto err_out;
	}
	a = ctx->attr;
	/* Sanity check. */
	if (a->non_resident) {
		ntfs_log_debug("Error: Attribute $VOLUME_INFORMATION must be "
				"resident (and it isn't)!\n");
		errno = EIO;
		goto err_out;
	}
	/* Get a pointer to the value of the attribute. */
	c = (VOLUME_INFORMATION*)(le16_to_cpu(a->value_offset) + (char*)a);
	/* Sanity checks. */
	if ((char*)c + le32_to_cpu(a->value_length) > (char*)ctx->mrec +
			le32_to_cpu(ctx->mrec->bytes_in_use) ||
			le16_to_cpu(a->value_offset) +
			le32_to_cpu(a->value_length) > le32_to_cpu(a->length)) {
		ntfs_log_debug("Error: Attribute $VOLUME_INFORMATION in $Volume is "
				"corrupt!\n");
		errno = EIO;
		goto err_out;
	}
	/* Set the volume flags. */
	vol->flags = c->flags = flags & VOLUME_FLAGS_MASK;
	/* Write them to disk. */
	ntfs_inode_mark_dirty(vol->vol_ni);
	if (ntfs_inode_sync(vol->vol_ni)) {
		ntfs_log_perror("Error writing $Volume");
		goto err_out;
	}
	ret = 0; /* success */
err_out:
	ntfs_attr_put_search_ctx(ctx);
	return ret;
}


#ifdef NTFS_RICH

#include "tree.h"
#include "rich.h"

/**
 * utils_valid_device - Perform some safety checks on the device before we start
 * @name:   Full pathname of the device/file to work with
 * @force:  Continue regardless of problems
 *
 * Check that the name refers to a device and that is isn't already mounted.
 * These checks can be overridden by using the force option.
 *
 * Return:  1  Success, we can continue
 *	    0  Error, we cannot use this device
 */
int utils_valid_device(const char *name, int force)
{
	unsigned long mnt_flags = 0;
	struct stat st;

#ifdef __CYGWIN32__
	/* FIXME: This doesn't work for Cygwin, so just return success for now... */
	return 1;
#endif
	if (!name) {
		errno = EINVAL;
		return 0;
	}
	ntfs_log_trace ("\n");

	if (stat(name, &st) == -1) {
		if (errno == ENOENT) {
			ntfs_log_error("The device %s doesn't exist\n", name);
		} else {
			ntfs_log_perror("Error getting information about %s", name);
		}
		return 0;
	}

	if (!S_ISBLK(st.st_mode) && !S_ISREG(st.st_mode)) {
		ntfs_log_warning("%s is not a block device, "
				 "nor regular file.\n", name);
		if (!force) {
			ntfs_log_error("Use the force option to work with other"
				       " file types, for your own risk!\n");
			return 0;
		}
		ntfs_log_warning("Forced to continue.\n");
	}

	/* Make sure the file system is not mounted. */
	if (ntfs_check_if_mounted(name, &mnt_flags)) {
		ntfs_log_perror("Failed to determine whether %s is mounted", name);
		if (!force) {
			ntfs_log_error("Use the force option to ignore this error.\n");
			return 0;
		}
		ntfs_log_warning("Forced to continue.\n");
	} else if (mnt_flags & NTFS_MF_MOUNTED) {
		ntfs_log_warning("The device %s, is mounted.\n", name);
		if (!force) {
			ntfs_log_error("Use the force option to work a mounted filesystem.\n");
			return 0;
		}
		ntfs_log_warning("Forced to continue.\n");
	}

	return 1;
}

/**
 * utils_mount_volume - Mount an NTFS volume
 * @device:
 * @flags:
 * @force:
 *
 * Description...
 *
 * Returns:
 */
ntfs_volume * utils_mount_volume(const char *device, unsigned long flags, BOOL force)
{
	ntfs_volume *vol;

	if (!device) {
		errno = EINVAL;
		return NULL;
	}
	ntfs_log_trace ("\n");

	if (!utils_valid_device(device, force))
		return NULL;

	vol = ntfs_mount(device, flags);
	if (!vol) {
		int err;

		err = errno;
		ntfs_log_perror("Couldn't mount device '%s'", device);
		if (err == EPERM)
			ntfs_log_error("Windows was hibernated.  Try to mount "
					"volume in windows, shut down and try "
					"again.\n");
		if (err == EOPNOTSUPP)
			ntfs_log_error("Windows did not shut down properly.  "
					"Try to mount volume in windows, "
					"shut down and try again.\n");
		return NULL;
	}

	if (vol->flags & VOLUME_IS_DIRTY) {
		ntfs_log_warning("Volume is dirty.\n");
		if (!force) {
			ntfs_log_error("Run chkdsk and try again, or use the "
					"force option.\n");
			ntfs_umount(vol, FALSE);
			return NULL;
		}
		ntfs_log_quiet("Forced to continue.\n");
	}

	return vol;
}

/**
 * ntfs_volume_commit - Write to disk the in-memory volume changes
 * @vol:
 *
 * Description...
 *
 * Returns:
 */
int ntfs_volume_commit(ntfs_volume *vol)
{
	if (!vol)
		return -1;

	ntfs_log_trace ("\n");
	ntfs_log_debug("commit volume\n");
	if (ntfs_bmp_commit(vol->private_bmp1) < 0)
		return -1;

	if (ntfs_bmp_commit(vol->private_bmp2) < 0)
		return -1;

	if (ntfs_dir_commit(vol->private_data) < 0)
		return -1;

	return 0;
}

/**
 * ntfs_volume_rollback - Discard the in-memory volume changes
 * @vol:
 *
 * Description...
 *
 * Returns:
 */
int ntfs_volume_rollback(ntfs_volume *vol)
{
	if (!vol)
		return -1;
	ntfs_log_trace ("\n");

	if (ntfs_bmp_rollback(vol->private_bmp1) < 0)
		return -1;

	if (ntfs_bmp_rollback(vol->private_bmp2) < 0)
		return -1;

	if (ntfs_dir_rollback(vol->private_data) < 0)
		return -1;

	return 0;
}

/**
 * ntfs_volume_umount2 - Unmount an NTFS volume, using the new directory support
 * @vol:
 * @force:
 *
 * Description...
 *
 * Returns:
 */
int ntfs_volume_umount2(ntfs_volume *vol, const BOOL force)
{
	struct ntfs_dir *dir;
	struct ntfs_bmp *bmp;

	if (!vol)
		return 0;
	ntfs_log_trace ("\n");

	ntfs_volume_rollback(vol);

	dir = (struct ntfs_dir *) vol->private_data;
	vol->private_data = NULL;
	ntfs_dir_free(dir);

	bmp = (struct ntfs_bmp *) vol->private_bmp1;
	vol->private_bmp1 = NULL;
	ntfs_bmp_free(bmp);

	bmp = (struct ntfs_bmp *) vol->private_bmp2;
	vol->private_bmp2 = NULL;
	ntfs_bmp_free(bmp);

	return ntfs_umount(vol, force);
}

/**
 * ntfs_volume_mount2 - Mount an NTFS volume, using the new directory support
 * @device:
 * @flags:
 * @force:
 *
 * Description...
 *
 * Returns:
 */
ntfs_volume * ntfs_volume_mount2(const char *device, unsigned long flags, BOOL force)
{
	// XXX can we replace these and search by mft number?  Hmm... NO.
	// unless I have a recursive search for an MFT number
	static ntfschar bmp[8] = {
		const_cpu_to_le16('$'),
		const_cpu_to_le16('B'),
		const_cpu_to_le16('i'),
		const_cpu_to_le16('t'),
		const_cpu_to_le16('m'),
		const_cpu_to_le16('a'),
		const_cpu_to_le16('p'),
		const_cpu_to_le16(0)
	};

	static ntfschar mft[5] = {
		const_cpu_to_le16('$'),
		const_cpu_to_le16('M'),
		const_cpu_to_le16('F'),
		const_cpu_to_le16('T'),
		const_cpu_to_le16(0)
	};

	static ntfschar mftmirr[9] = {
		const_cpu_to_le16('$'),
		const_cpu_to_le16('M'),
		const_cpu_to_le16('F'),
		const_cpu_to_le16('T'),
		const_cpu_to_le16('M'),
		const_cpu_to_le16('i'),
		const_cpu_to_le16('r'),
		const_cpu_to_le16('r'),
		const_cpu_to_le16(0)
	};

	static ntfschar dot[2] = {
		const_cpu_to_le16('.'),
		const_cpu_to_le16(0)
	};

	ntfs_volume *vol;
	struct ntfs_dir *dir;
	struct ntfs_dt *root;
	struct ntfs_dt *found;
	int num;

	ntfs_log_trace ("\n");
	vol = utils_mount_volume(device, flags, force);
	if (!vol)
		return NULL;

	vol->lcnbmp_ni ->ref_count = 1;
	vol->mft_ni    ->ref_count = 1;
	vol->mftmirr_ni->ref_count = 1;

	vol->lcnbmp_ni ->private_data = NULL;
	vol->mft_ni    ->private_data = NULL;
	vol->mftmirr_ni->private_data = NULL;

	dir = ntfs_dir_create(vol, FILE_root);
	if (!dir) {
		ntfs_volume_umount2(vol, FALSE);
		vol = NULL;
		goto done;
	}

	dir->index = ntfs_dt_create(dir, NULL, -1);

	root = dir->index;

	//$Bitmap
	num = -1;
	found = ntfs_dt_find2(root, bmp, (sizeof(bmp)/sizeof(ntfschar)) - 1, &num);
	if ((!found) || (num < 0)) {
		ntfs_log_debug("can't find $Bitmap\n");
		ntfs_volume_umount2(vol, FALSE);
		vol = NULL;
		goto done;
	}
	vol->lcnbmp_ni->ref_count++;
	vol->lcnbmp_ni->private_data = found->dir;
	found->inodes[num] = vol->lcnbmp_ni;

	//$MFT
	num = -1;
	found = ntfs_dt_find2(root, mft, (sizeof(mft)/sizeof(ntfschar)) - 1, &num);
	if ((!found) || (num < 0)) {
		ntfs_log_debug("can't find $MFT\n");
		ntfs_volume_umount2(vol, FALSE);
		vol = NULL;
		goto done;
	}
	vol->mft_ni->ref_count++;
	vol->mft_ni->private_data = found->dir;
	found->inodes[num] = vol->mft_ni;

	//$MFTMirr
	num = -1;
	found = ntfs_dt_find2(root, mftmirr, (sizeof(mftmirr)/sizeof(ntfschar)) - 1, &num);
	if ((!found) || (num < 0)) {
		ntfs_log_debug("can't find $MFTMirr\n");
		ntfs_volume_umount2(vol, FALSE);
		vol = NULL;
		goto done;
	}
	vol->mftmirr_ni->ref_count++;
	vol->mftmirr_ni->private_data = found->dir;
	found->inodes[num] = vol->mftmirr_ni;

	// root directory
	num = -1;
	found = ntfs_dt_find2(root, dot, (sizeof(dot)/sizeof(ntfschar)) - 1, &num);
	if ((!found) || (num < 0)) {
		ntfs_log_debug("can't find the root directory\n");
		ntfs_volume_umount2(vol, FALSE);
		vol = NULL;
		goto done;
	}

	vol->private_data = found->dir;
	found->inodes[num] = dir->inode;
	dir->inode->private_data = found;
	dir->inode->ref_count = 2;

	vol->private_bmp1 = ntfs_bmp_create(vol->mft_ni,    AT_BITMAP, NULL, 0);
	vol->private_bmp2 = ntfs_bmp_create(vol->lcnbmp_ni, AT_DATA,   NULL, 0);

	if (!vol->private_bmp1 || !vol->private_bmp2) {
		ntfs_log_debug("can't find the bitmaps\n");
		ntfs_volume_umount2(vol, FALSE);
		vol = NULL;
		goto done;
	}

done:
	return vol;
}


#endif /* NTFS_RICH */

