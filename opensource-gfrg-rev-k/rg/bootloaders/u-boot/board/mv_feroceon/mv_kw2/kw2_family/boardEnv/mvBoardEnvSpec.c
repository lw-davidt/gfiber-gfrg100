/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * 	Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

    *	Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.

    *	Neither the name of Marvell nor the names of its contributors may be
	used to endorse or promote products derived from this software without
	specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
#include "mvCommon.h"
#include "mvBoardEnvLib.h"
#include "mvBoardEnvSpec.h"
#include "twsi/mvTwsi.h"

/***************************************************************************
** RD-88F6510-SFU
****************************************************************************/

#define RD_88F6510_BOARD_NAND_READ_PARAMS	0x000C0282
#define RD_88F6510_BOARD_NAND_WRITE_PARAMS	0x00010305
/* NAND care support for small page chips */
#define RD_88F6510_BOARD_NAND_CONTROL		0x01c00543

MV_BOARD_TWSI_INFO rd88f6510InfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
};

MV_BOARD_MAC_INFO rd88f6510InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_1000M, 0x0}
	,
	{BOARD_MAC_SPEED_1000M, 0x9}
/* 	{BOARD_MAC_SPEED_AUTO, 0x9} */

};

MV_BOARD_MPP_TYPE_INFO rd88f6510InfoBoardMppTypeInfo[] = {
	{
	 .boardMppTdm = MV_BOARD_TDM_2CH,
	 .ethSataComplexOpt = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_FE3PHY | ESC_OPT_GEPHY_SW_P0,
	 .ethPortsMode = 0x0
	 }
};

MV_BOARD_GPP_INFO rd88f6510InfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
	{BOARD_GPP_SYS_LED, 44},
	{BOARD_GPP_PON_LED, 45},
	{BOARD_GPP_PEX_RESET, 67},
	{BOARD_GPP_PON_XVR_TX, 68}
};

MV_DEV_CS_INFO rd88f6510InfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#if defined(MTD_NAND_LNC)
	{NAND_NOR_CS, N_A, BOARD_DEV_NAND_FLASH, 8}	/* NAND DEV */
#elif defined(MV_SPI)
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8}		/* SPI DEV */
#else
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO rd88f6510InfoBoardMppConfigValue[] = {
	{{
		RD_88F6510_MPP0_7,
		RD_88F6510_MPP8_15,
		RD_88F6510_MPP16_23,
		RD_88F6510_MPP24_31,
		RD_88F6510_MPP32_39,
		RD_88F6510_MPP40_47,
		RD_88F6510_MPP48_55,
		RD_88F6510_MPP56_63,
		RD_88F6510_MPP64_71,
		RD_88F6510_MPP72_79,
		RD_88F6510_MPP80_87,
		RD_88F6510_MPP88_88
	  }
	 }
};

MV_BOARD_SWITCH_INFO rd88f6510InfoBoardSwitchValue[] = {
	{
	 .switchIrq = 29,	/* set to -1 for timer operation */
	 .switchPort = {1, 2, 3, 0, -1, -1, -1, -1},
	 .cpuPort = 4,
	 .connectedPort = {4, 5, -1},
	 .smiScanMode = 1,
	 .quadPhyAddr = 0,
	 .forceLinkMask = 0x0
	 }
};

MV_BOARD_TDM_INFO rd88f6510Tdm880[] = { {1} };

MV_BOARD_TDM_SPI_INFO rd88f6510TdmSpiInfo[] = { {0} };

MV_BOARD_INFO rd88f6510Info = {
	.boardName = "RD-88F6510-SFU",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(rd88f6510InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = rd88f6510InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(rd88f6510InfoBoardMppConfigValue),
	.pBoardMppConfigValue = rd88f6510InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(rd88f6510InfoBoardDeCsInfo),
	.pDevCsInfo = rd88f6510InfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(rd88f6510InfoBoardTwsiDev),
	.pBoardTwsiDev = rd88f6510InfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(rd88f6510InfoBoardMacInfo),
	.pBoardMacInfo = rd88f6510InfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(rd88f6510InfoBoardGppInfo),
	.pBoardGppInfo = rd88f6510InfoBoardGppInfo,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = RD_88F6510_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = RD_88F6510_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = RD_88F6510_GPP_OUT_ENA_HIGH,
	.gppOutValLow = RD_88F6510_GPP_OUT_VAL_LOW,
	.gppOutValMid = RD_88F6510_GPP_OUT_VAL_MID,
	.gppOutValHigh = RD_88F6510_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = RD_88F6510_GPP_POL_LOW,
	.gppPolarityValMid = RD_88F6510_GPP_POL_MID,
	.gppPolarityValHigh = RD_88F6510_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = rd88f6510InfoBoardSwitchValue,
	.switchInfoNum = MV_ARRAY_SIZE(rd88f6510InfoBoardSwitchValue),

	/* PON configuration. */
	.ponConfigValue = BOARD_PON_AUTO,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {1}
	,
	.pBoardTdmInt2CsInfo = {rd88f6510Tdm880,
				}
	,
	.boardTdmInfoIndex = 0,

	.deepIdlePwrUpDelay = 0x80000,

	/* NAND init params */
	.nandFlashParamsValid = MV_TRUE,
	.nandFlashReadParams = RD_88F6510_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = RD_88F6510_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = RD_88F6510_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = rd88f6510TdmSpiInfo,

	/* Disable modules auto-detection. */
	.moduleAutoDetect = MV_FALSE
};

/***************************************************************************
** RD-88F6560-GW
****************************************************************************/

#define RD_88F6560_BOARD_NAND_READ_PARAMS	0x000C0282
#define RD_88F6560_BOARD_NAND_WRITE_PARAMS	0x00010305
/* NAND care support for small page chips */
#define RD_88F6560_BOARD_NAND_CONTROL		0x01c00543

MV_BOARD_TWSI_INFO rd88f6560InfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
};

MV_BOARD_MAC_INFO rd88f6560InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_1000M, 0x8}
	,
	{BOARD_MAC_SPEED_AUTO, 0x9}
};

MV_BOARD_MPP_TYPE_INFO rd88f6560InfoBoardMppTypeInfo[] = {
	{
	 .boardMppTdm = MV_BOARD_TDM_2CH,
	 .ethSataComplexOpt = ESC_OPT_QSGMII | ESC_OPT_GEPHY_MAC1 | ESC_OPT_MAC0_2_SW_P4,
	 .ethPortsMode = 0x0
	}
};

MV_BOARD_GPP_INFO rd88f6560InfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
	{BOARD_GPP_SDIO_WP, 31},
	{BOARD_GPP_FAN_POWER, 40},
	{BOARD_GPP_WPS_BUTTON, 47},
	{BOARD_GPP_SDIO_DETECT, 48},
	{BOARD_GPP_PON_LED, 49},
	{BOARD_GPP_USB_VBUS, 58},
	{BOARD_GPP_USB_OC, 59},
	{BOARD_GPP_CONF, 60},
	{BOARD_GPP_CONF, 61},
	{BOARD_GPP_CONF, 62},
	{BOARD_GPP_SYS_LED, 68},
	{BOARD_GPP_PEX_RESET, 77}
};

MV_DEV_CS_INFO rd88f6560InfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#if defined(MTD_NAND_LNC)
	{NAND_NOR_CS, N_A, BOARD_DEV_NAND_FLASH, 8}	/* NAND DEV */
#elif defined(MV_SPI)
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8}	/* SPI DEV */
#else
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO rd88f6560InfoBoardMppConfigValue[] = {
	{{
	  RD_88F6560_MPP0_7,
	  RD_88F6560_MPP8_15,
	  RD_88F6560_MPP16_23,
	  RD_88F6560_MPP24_31,
	  RD_88F6560_MPP32_39,
	  RD_88F6560_MPP40_47,
	  RD_88F6560_MPP48_55,
	  RD_88F6560_MPP56_63,
	  RD_88F6560_MPP64_71,
	  RD_88F6560_MPP72_79,
	  RD_88F6560_MPP80_87,
	  RD_88F6560_MPP88_88
	  }
	 }
};

MV_BOARD_SWITCH_INFO rd88f6560InfoBoardSwitchValue[] = {
	{
	 .switchIrq = 29,	/* set to -1 for timer operation */
	 .switchPort = {1, 2, 3, 0, -1, -1, -1, -1},
	 .cpuPort = 4,
	 .connectedPort = {4, -1, -1},
	 .smiScanMode = 2,
	 .quadPhyAddr = 0,
	 .forceLinkMask = 0x0
	 }
};

MV_U8 rd88f6560LedGppPin[] = { 68 };

MV_BOARD_TDM_INFO rd88f6560Tdm880[] = { {0}, {1} };

MV_BOARD_TDM_SPI_INFO rd88f6560TdmSpiInfo[] = { {1} };

MV_BOARD_SPEC_INIT rd88f6560BoardSpecInit[] = {
	{
		.reg = PMU_POWER_IF_POLARITY_REG,
		.mask = (BIT1),
		.val = 0
	},
	{
		.reg = TBL_TERM,
		.val = TBL_TERM
	}
};

MV_BOARD_INFO rd88f6560Info = {
	.boardName = "RD-88F6560-GW",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(rd88f6560InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = rd88f6560InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(rd88f6560InfoBoardMppConfigValue),
	.pBoardMppConfigValue = rd88f6560InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(rd88f6560InfoBoardDeCsInfo),
	.pDevCsInfo = rd88f6560InfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(rd88f6560InfoBoardTwsiDev),
	.pBoardTwsiDev = rd88f6560InfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(rd88f6560InfoBoardMacInfo),
	.pBoardMacInfo = rd88f6560InfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(rd88f6560InfoBoardGppInfo),
	.pBoardGppInfo = rd88f6560InfoBoardGppInfo,
	.activeLedsNumber = MV_ARRAY_SIZE(rd88f6560LedGppPin),
	.pLedGppPin = rd88f6560LedGppPin,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = RD_88F6560_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = RD_88F6560_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = RD_88F6560_GPP_OUT_ENA_HIGH,
	.gppOutValLow = RD_88F6560_GPP_OUT_VAL_LOW,
	.gppOutValMid = RD_88F6560_GPP_OUT_VAL_MID,
	.gppOutValHigh = RD_88F6560_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = RD_88F6560_GPP_POL_LOW,
	.gppPolarityValMid = RD_88F6560_GPP_POL_MID,
	.gppPolarityValHigh = RD_88F6560_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = rd88f6560InfoBoardSwitchValue,
	.switchInfoNum = MV_ARRAY_SIZE(rd88f6560InfoBoardSwitchValue),

	/* PON configuration. */
	.ponConfigValue = BOARD_GPON_CONFIG,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {2},
	.pBoardTdmInt2CsInfo = { rd88f6560Tdm880 },
	.boardTdmInfoIndex = 0,

	.pBoardSpecInit = rd88f6560BoardSpecInit,

	.deepIdlePwrUpDelay = 2400, /* 12 uS */

	/* NAND init params */
	.nandFlashParamsValid = MV_TRUE,
	.nandFlashReadParams = RD_88F6560_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = RD_88F6560_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = RD_88F6560_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = rd88f6560TdmSpiInfo,

	/* Disable modules auto-detection. */
	.moduleAutoDetect = MV_FALSE
};

/***************************************************************************
** RD-88F6530-MDU
****************************************************************************/
#define RD_88F6530_BOARD_NAND_READ_PARAMS	0x000C0282
#define RD_88F6530_BOARD_NAND_WRITE_PARAMS	0x00010305
#define RD_88F6530_BOARD_NAND_CONTROL		0x01c00543

MV_BOARD_TWSI_INFO rd88f6530InfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
};

MV_BOARD_MAC_INFO rd88f6530InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_1000M, 0x8}
	,			/* This connected to SGMII to xcat ,right now forced SGMII 1G */
	{BOARD_MAC_SPEED_AUTO, 0x9}
};

MV_BOARD_MPP_TYPE_INFO rd88f6530InfoBoardMppTypeInfo[] = {
	{
	 .boardMppTdm = MV_BOARD_TDM_2CH,
	 .ethSataComplexOpt = ESC_OPT_SGMII | ESC_OPT_GEPHY_MAC1,
	 .ethPortsMode = 0x0
	}
};

MV_BOARD_GPP_INFO rd88f6530InfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
};

MV_DEV_CS_INFO rd88f6530InfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#if defined(MTD_NAND_LNC)
	{NAND_NOR_CS, N_A, BOARD_DEV_NAND_FLASH, 8}	/* NAND DEV */
#elif defined(MV_SPI)
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8}	/* SPI DEV */
#else
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO rd88f6530InfoBoardMppConfigValue[] = {
	{{
	  RD_88F6530_MPP0_7,
	  RD_88F6530_MPP8_15,
	  RD_88F6530_MPP16_23,
	  RD_88F6530_MPP24_31,
	  RD_88F6530_MPP32_39,
	  RD_88F6530_MPP40_47,
	  RD_88F6530_MPP48_55,
	  RD_88F6530_MPP56_63,
	  RD_88F6530_MPP64_69,
	  RD_88F6530_MPP72_79,
	  RD_88F6530_MPP80_87,
	  RD_88F6530_MPP88_88
	  }
	 }
};

MV_BOARD_TDM_INFO rd88f6530Tdm880[] = { {1}
};

MV_BOARD_TDM_SPI_INFO rd88f6530TdmSpiInfo[] = { {0} };

MV_BOARD_INFO rd88f6530Info = {
	.boardName = "RD-88F6530-MDU",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(rd88f6530InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = rd88f6530InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(rd88f6530InfoBoardMppConfigValue),
	.pBoardMppConfigValue = rd88f6530InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(rd88f6530InfoBoardDeCsInfo),
	.pDevCsInfo = rd88f6530InfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(rd88f6530InfoBoardTwsiDev),
	.pBoardTwsiDev = rd88f6530InfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(rd88f6530InfoBoardMacInfo),
	.pBoardMacInfo = rd88f6530InfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(rd88f6530InfoBoardGppInfo),
	.pBoardGppInfo = rd88f6530InfoBoardGppInfo,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = RD_88F6530_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = RD_88F6530_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = RD_88F6530_GPP_OUT_ENA_HIGH,
	.gppOutValLow = RD_88F6530_GPP_OUT_VAL_LOW,
	.gppOutValMid = RD_88F6530_GPP_OUT_VAL_MID,
	.gppOutValHigh = RD_88F6530_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = RD_88F6530_GPP_POL_LOW,
	.gppPolarityValMid = RD_88F6530_GPP_POL_MID,
	.gppPolarityValHigh = RD_88F6530_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = NULL,
	.switchInfoNum = 0,

	/* PON configuration. */
	.ponConfigValue = BOARD_GPON_CONFIG,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {1}
	,
	.pBoardTdmInt2CsInfo = {rd88f6530Tdm880,
				}
	,
	.boardTdmInfoIndex = 0,

	.deepIdlePwrUpDelay = 0x80000,

	/* NAND init params */
	.nandFlashParamsValid = MV_TRUE,
	.nandFlashReadParams = RD_88F6530_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = RD_88F6530_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = RD_88F6530_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = rd88f6530TdmSpiInfo,

	/* Disable modules auto-detection. */
	.moduleAutoDetect = MV_FALSE
};

/***************************************************************************
** DB-88F6560-PCAC
****************************************************************************/

#define DB_88F6560_PCAC_BOARD_NAND_READ_PARAMS	0x000C0282
#define DB_88F6560_PCAC_BOARD_NAND_WRITE_PARAMS	0x00010305
/* NAND care support for small page chips */
#define DB_88F6560_PCAC_BOARD_NAND_CONTROL	0x01c00543

MV_BOARD_TWSI_INFO db88f6560PcacInfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
};

MV_BOARD_MAC_INFO db88f6560PcacInfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_AUTO, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9}
};

MV_BOARD_MPP_TYPE_INFO db88f6560PcacInfoBoardMppTypeInfo[] = {
	{
	 .boardMppTdm = MV_BOARD_OTHER,
	 .ethSataComplexOpt = ESC_OPT_GEPHY_MAC1,
	 .ethPortsMode = 0x0
	}
};

MV_BOARD_GPP_INFO db88f6560PcacInfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
	{BOARD_GPP_USB_VBUS, 58}
};

MV_DEV_CS_INFO db88f6560PcacInfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#if defined(MTD_NAND_LNC)
	{NAND_NOR_CS, N_A, BOARD_DEV_NAND_FLASH, 8}	/* NAND DEV */
#elif defined(MV_SPI)
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8}	/* SPI DEV */
#else
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO db88f6560PcacInfoBoardMppConfigValue[] = {
	{ {
		 DB_88F6560_PCAC_MPP0_7,
		 DB_88F6560_PCAC_MPP8_15,
		 DB_88F6560_PCAC_MPP16_23,
		 DB_88F6560_PCAC_MPP24_31,
		 DB_88F6560_PCAC_MPP32_39,
		 DB_88F6560_PCAC_MPP40_47,
		 DB_88F6560_PCAC_MPP48_55,
		 DB_88F6560_PCAC_MPP56_63,
		 DB_88F6560_PCAC_MPP64_71,
		 DB_88F6560_PCAC_MPP72_79,
		 DB_88F6560_PCAC_MPP80_87,
		 DB_88F6560_PCAC_MPP88_88
	 } }
};

MV_U8 db88f6560PcacLedGppPin[] = { 68, 69, 70, 71};

MV_BOARD_TDM_SPI_INFO db88f6560PcacTdmSpiInfo[] = { {0} };

MV_BOARD_SPEC_INIT db88f6560PcacBoardSpecInit[] = {
	{
		.reg = TBL_TERM,
		.val = TBL_TERM
	}
};

MV_BOARD_INFO db88f6560PCACPInfo = {
	.boardName = "DB-88F6560-PCAC",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(db88f6560PcacInfoBoardMppTypeInfo),
	.pBoardMppTypeValue = db88f6560PcacInfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(db88f6560PcacInfoBoardMppConfigValue),
	.pBoardMppConfigValue = db88f6560PcacInfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(db88f6560PcacInfoBoardDeCsInfo),
	.pDevCsInfo = db88f6560PcacInfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(db88f6560PcacInfoBoardTwsiDev),
	.pBoardTwsiDev = db88f6560PcacInfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(db88f6560PcacInfoBoardMacInfo),
	.pBoardMacInfo = db88f6560PcacInfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(db88f6560PcacInfoBoardGppInfo),
	.pBoardGppInfo = db88f6560PcacInfoBoardGppInfo,
	.activeLedsNumber = MV_ARRAY_SIZE(db88f6560PcacLedGppPin),
	.pLedGppPin = db88f6560PcacLedGppPin,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = DB_88F6560_PCAC_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = DB_88F6560_PCAC_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = DB_88F6560_PCAC_GPP_OUT_ENA_HIGH,
	.gppOutValLow = DB_88F6560_PCAC_GPP_OUT_VAL_LOW,
	.gppOutValMid = DB_88F6560_PCAC_GPP_OUT_VAL_MID,
	.gppOutValHigh = DB_88F6560_PCAC_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = DB_88F6560_PCAC_GPP_POL_LOW,
	.gppPolarityValMid = DB_88F6560_PCAC_GPP_POL_MID,
	.gppPolarityValHigh = DB_88F6560_PCAC_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = NULL,
	.switchInfoNum = 0,

	/* PON configuration. */
	.ponConfigValue = BOARD_PON_NONE,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {0},
	.pBoardTdmInt2CsInfo = {NULL},
	.boardTdmInfoIndex = -1,

	.pBoardSpecInit = db88f6560PcacBoardSpecInit,

	.deepIdlePwrUpDelay = 2400, /* 12 uS */

	/* NAND init params */
	.nandFlashParamsValid = MV_TRUE,
	.nandFlashReadParams = DB_88F6560_PCAC_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = DB_88F6560_PCAC_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = DB_88F6560_PCAC_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = db88f6560PcacTdmSpiInfo,

	/* Disable modules auto-detection. */
	.moduleAutoDetect = MV_FALSE
};

/***************************************************************************
** DB-88F6560-BP
****************************************************************************/
#define DB_88F6535_BOARD_NAND_READ_PARAMS	0x000C0282
#define DB_88F6535_BOARD_NAND_WRITE_PARAMS	0x00010305
/* NAND care support for small page chips */
#define DB_88F6535_BOARD_NAND_CONTROL		0x01c00543

MV_BOARD_TWSI_INFO db88f6535InfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{BOARD_DEV_TWSI_SATR, 0x4C, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4D, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4E, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4F, ADDR7_BIT},
	{BOARD_TWSI_MUX, 0x70, ADDR7_BIT}
};

MV_BOARD_MAC_INFO db88f6535InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_AUTO, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9}
};

MV_BOARD_MPP_TYPE_INFO db88f6535InfoBoardMppTypeInfo[] = {
	{
		.boardMppTdm = MV_BOARD_AUTO,
		.ethSataComplexOpt = ESC_OPT_AUTO,
		.ethPortsMode = 0x0
	}
};

MV_BOARD_GPP_INFO db88f6535InfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
	{BOARD_GPP_PON_XVR_TX, 17},
	{BOARD_GPP_USB_VBUS, 31},
	{BOARD_GPP_PEX_RESET, 67},
	{BOARD_GPP_SDIO_DETECT, 68},
	{BOARD_GPP_SDIO_WP, 69},
};

MV_DEV_CS_INFO db88f6535InfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#ifdef MTD_NAND_LNC
	{NAND_NOR_CS, N_A, BOARD_DEV_NAND_FLASH, 8},	/* NAND DEV */
#endif
#ifdef MV_SPI
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8},		/* SPI DEV */
#endif
#ifdef MV_NOR
	{NAND_NOR_CS, N_A, BOARD_DEV_NOR_FLASH, 16},	/* NOR DEV */
#endif
#if !defined(MTD_NAND_LNC) && !defined(MV_SPI) && !defined(MV_NOR)
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO db88f6535InfoBoardMppConfigValue[] = {
	{{
	  DB_88F6535_MPP0_7,
	  DB_88F6535_MPP8_15,
	  DB_88F6535_MPP16_23,
	  DB_88F6535_MPP24_31,
	  DB_88F6535_MPP32_39,
	  DB_88F6535_MPP40_47,
	  DB_88F6535_MPP48_55,
	  DB_88F6535_MPP56_63,
	  DB_88F6535_MPP64_71,
	  DB_88F6535_MPP72_79,
	  DB_88F6535_MPP80_87,
	  DB_88F6535_MPP88_88
	  }
	 }
};

MV_BOARD_SWITCH_INFO db88f6535InfoBoardSwitchValue[] = {
	{
	 .switchIrq = 29,	/* set to -1 for timer operation */
	 .switchPort = {1, 2, 3, 0, -1, -1, -1, -1},
	 .cpuPort = 4,
	 .connectedPort = {4, 5, -1},
	 .smiScanMode = 2,
	 .quadPhyAddr = 0,
	 .forceLinkMask = 0x0
	 }
};

MV_BOARD_TDM_INFO db88f6535Tdm880[] = {
	{1},
	{2}
};

MV_BOARD_TDM_INFO db88f6535Tdm792[] = {
	{1},
	{2},
	{3},
	{4},
	{6},
	{7}
};

MV_BOARD_TDM_INFO db88f6535Tdm3215[] = { {1} };

MV_BOARD_TDM_SPI_INFO db88f6535TdmSpiInfo[] = { {0} };

MV_BOARD_SPEC_INIT db88f6535BoardSpecInit[] = {
	{
		.reg = PMU_POWER_IF_POLARITY_REG,
		.mask = (BIT1),
		.val = 0
	},
	{
		.reg = TBL_TERM,
		.val = TBL_TERM
	}
};

MV_BOARD_INFO db88f6535Info = {
	.boardName = "DB-88F6560-BP",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(db88f6535InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = db88f6535InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(db88f6535InfoBoardMppConfigValue),
	.pBoardMppConfigValue = db88f6535InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(db88f6535InfoBoardDeCsInfo),
	.pDevCsInfo = db88f6535InfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(db88f6535InfoBoardTwsiDev),
	.pBoardTwsiDev = db88f6535InfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(db88f6535InfoBoardMacInfo),
	.pBoardMacInfo = db88f6535InfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(db88f6535InfoBoardGppInfo),
	.pBoardGppInfo = db88f6535InfoBoardGppInfo,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = DB_88F6535_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = DB_88F6535_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = DB_88F6535_GPP_OUT_ENA_HIGH,
	.gppOutValLow = DB_88F6535_GPP_OUT_VAL_LOW,
	.gppOutValMid = DB_88F6535_GPP_OUT_VAL_MID,
	.gppOutValHigh = DB_88F6535_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = DB_88F6535_GPP_POL_LOW,
	.gppPolarityValMid = DB_88F6535_GPP_POL_MID,
	.gppPolarityValHigh = DB_88F6535_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = db88f6535InfoBoardSwitchValue,
	.switchInfoNum = MV_ARRAY_SIZE(db88f6535InfoBoardSwitchValue),

	/* PON configuration. */
	.ponConfigValue = BOARD_PON_AUTO,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {2, 6, 1},
	.pBoardTdmInt2CsInfo = {db88f6535Tdm880,
				db88f6535Tdm792,
				db88f6535Tdm3215},
	.boardTdmInfoIndex = -1,

	.pBoardSpecInit = db88f6535BoardSpecInit,

	.deepIdlePwrUpDelay = 2400,	/* 12uS */

	/* NAND init params */
	.nandFlashParamsValid = MV_TRUE,
	.nandFlashReadParams = DB_88F6535_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = DB_88F6535_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = DB_88F6535_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = db88f6535TdmSpiInfo,

	/* Enable modules auto-detection. */
	.moduleAutoDetect = MV_TRUE
};


/* MI424WR Common */

#define MI424WR_BOARD_NAND_READ_PARAMS	0x000C0282
#define MI424WR_BOARD_NAND_WRITE_PARAMS	0x00010305
#define MI424WR_BOARD_NAND_CONTROL	0x01c00543

MV_BOARD_TDM_INFO mi424wr_Tdm880[] = { };

MV_BOARD_TDM_SPI_INFO mi424wr_TdmSpiInfo[] = { };

MV_DEV_CS_INFO mi424wr_InfoBoardDeCsInfo[] = {
	{NAND_NOR_CS, N_A, BOARD_DEV_NAND_FLASH, 8}	/* NAND DEV */
};

/* MI424WR-I */
MV_BOARD_MAC_INFO mi424wr_i_InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_100M, 0x13},
	{BOARD_MAC_SPEED_1000M, 0x8},
};

MV_BOARD_MPP_TYPE_INFO mi424wr_i_InfoBoardMppTypeInfo[] = {
	{
	 .boardMppTdm = MV_BOARD_TDM_2CH, /* XXX */
	 .ethSataComplexOpt = ESC_OPT_QSGMII | ESC_OPT_RGMIIB_MAC0 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_RGMIIA_SW_P6,
	 .ethPortsMode = EPM_MAC0_MII | EPM_SW_PORT_6_MII}
};

MV_BOARD_MPP_INFO mi424wr_i_InfoBoardMppConfigValue[] = {
	{{
	  MI424WR_I_MPP0_7,
	  MI424WR_I_MPP8_15,
	  MI424WR_I_MPP16_23,
	  MI424WR_I_MPP24_31,
	  MI424WR_I_MPP32_39,
	  MI424WR_I_MPP40_47,
	  MI424WR_I_MPP48_55,
	  MI424WR_I_MPP56_63,
	  MI424WR_I_MPP64_71,
	  MI424WR_I_MPP72_79,
	  MI424WR_I_MPP80_87,
	  MI424WR_I_MPP88_88
	  }
	 }
};

MV_BOARD_SWITCH_INFO mi424wr_i_InfoBoardSwitchValue[] = {
	{
	 .switchIrq = 29,
	 .switchPort = {1, 2, 3, 0, 6, -1, -1, -1},
	 .cpuPort = 5,
	 .connectedPort = {-1, 5, -1},
	 .smiScanMode = 2,
	 .quadPhyAddr = 0
	 }
};

MV_BOARD_INFO mi424wr_i_Info = {
	.boardName = "MI424WR-I",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(mi424wr_i_InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = mi424wr_i_InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(mi424wr_i_InfoBoardMppConfigValue),
	.pBoardMppConfigValue = mi424wr_i_InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(mi424wr_InfoBoardDeCsInfo),
	.pDevCsInfo = mi424wr_InfoBoardDeCsInfo,
	.numBoardTwsiDev = 0,
	.pBoardTwsiDev = NULL,
	.numBoardMacInfo = MV_ARRAY_SIZE(mi424wr_i_InfoBoardMacInfo),
	.pBoardMacInfo = mi424wr_i_InfoBoardMacInfo,
	.numBoardGppInfo = 0,
	.pBoardGppInfo = NULL,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = MI424WR_I_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = MI424WR_I_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = MI424WR_I_GPP_OUT_ENA_HIGH,
	.gppOutValLow = MI424WR_I_GPP_OUT_VAL_LOW,
	.gppOutValMid = MI424WR_I_GPP_OUT_VAL_MID,
	.gppOutValHigh = MI424WR_I_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = MI424WR_I_GPP_POL_LOW,
	.gppPolarityValMid = MI424WR_I_GPP_POL_MID,
	.gppPolarityValHigh = MI424WR_I_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = mi424wr_i_InfoBoardSwitchValue,
	.switchInfoNum = MV_ARRAY_SIZE(mi424wr_i_InfoBoardSwitchValue),

	/* PON configuration. */
	.ponConfigValue = BOARD_PON_NONE,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {},
	.pBoardTdmInt2CsInfo = { mi424wr_Tdm880 },
	.boardTdmInfoIndex = -1,

	/* NAND init params */
	.nandFlashReadParams = MI424WR_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = MI424WR_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = MI424WR_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = mi424wr_TdmSpiInfo
};

/* MI424WR-J */
MV_BOARD_MAC_INFO mi424wr_j_InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_1000M, 0x13},
	{BOARD_MAC_SPEED_1000M, 0x8},
};

MV_BOARD_MPP_TYPE_INFO mi424wr_j_InfoBoardMppTypeInfo[] = {
	{
	 .boardMppTdm = MV_BOARD_TDM_2CH, /* XXX */
	 .ethSataComplexOpt = ESC_OPT_QSGMII | ESC_OPT_RGMIIB_MAC0 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_RGMIIA_SW_P6,
	 .ethPortsMode = 0 }
};

MV_BOARD_MPP_INFO mi424wr_j_InfoBoardMppConfigValue[] = {
	{{
	  MI424WR_J_MPP0_7,
	  MI424WR_J_MPP8_15,
	  MI424WR_J_MPP16_23,
	  MI424WR_J_MPP24_31,
	  MI424WR_J_MPP32_39,
	  MI424WR_J_MPP40_47,
	  MI424WR_J_MPP48_55,
	  MI424WR_J_MPP56_63,
	  MI424WR_J_MPP64_71,
	  MI424WR_J_MPP72_79,
	  MI424WR_J_MPP80_87,
	  MI424WR_J_MPP88_88
	  }
	 }
};

MV_BOARD_SWITCH_INFO mi424wr_j_InfoBoardSwitchValue[] = {
	{
	 .switchIrq = 29,
	 .switchPort = {1, 2, 3, 0, 6, -1, -1, -1},
	 .cpuPort = 5,
	 .connectedPort = {-1, 5, -1},
	 .smiScanMode = 2,
	 .quadPhyAddr = 0
	 }
};

MV_BOARD_INFO mi424wr_j_Info = {
	.boardName = "MI424WR-J",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(mi424wr_j_InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = mi424wr_j_InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(mi424wr_j_InfoBoardMppConfigValue),
	.pBoardMppConfigValue = mi424wr_j_InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(mi424wr_InfoBoardDeCsInfo),
	.pDevCsInfo = mi424wr_InfoBoardDeCsInfo,
	.numBoardTwsiDev = 0,
	.pBoardTwsiDev = NULL,
	.numBoardMacInfo = MV_ARRAY_SIZE(mi424wr_j_InfoBoardMacInfo),
	.pBoardMacInfo = mi424wr_j_InfoBoardMacInfo,
	.numBoardGppInfo = 0,
	.pBoardGppInfo = NULL,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = MI424WR_J_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = MI424WR_J_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = MI424WR_J_GPP_OUT_ENA_HIGH,
	.gppOutValLow = MI424WR_J_GPP_OUT_VAL_LOW,
	.gppOutValMid = MI424WR_J_GPP_OUT_VAL_MID,
	.gppOutValHigh = MI424WR_J_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = MI424WR_J_GPP_POL_LOW,
	.gppPolarityValMid = MI424WR_J_GPP_POL_MID,
	.gppPolarityValHigh = MI424WR_J_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = mi424wr_j_InfoBoardSwitchValue,
	.switchInfoNum = MV_ARRAY_SIZE(mi424wr_j_InfoBoardSwitchValue),

	/* PON configuration. */
	.ponConfigValue = BOARD_PON_NONE,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {},
	.pBoardTdmInt2CsInfo = { mi424wr_Tdm880 },
	.boardTdmInfoIndex = -1,

	/* NAND init params */
	.nandFlashReadParams = MI424WR_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = MI424WR_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = MI424WR_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = mi424wr_TdmSpiInfo
};

/* MI424WR-K0 */
MV_BOARD_MAC_INFO mi424wr_k0_InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_1000M, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9},
};

MV_BOARD_MPP_TYPE_INFO mi424wr_k0_InfoBoardMppTypeInfo[] = {
	{
	 .boardMppTdm = MV_BOARD_TDM_2CH, /* XXX */
	 .ethSataComplexOpt = ESC_OPT_QSGMII | ESC_OPT_GEPHY_MAC1 | ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_RGMIIA_SW_P6,
	 .ethPortsMode = EPM_SW_PORT_6_MII,
	}
};

MV_BOARD_MPP_INFO mi424wr_k0_InfoBoardMppConfigValue[] = {
	{{
	  MI424WR_K0_MPP0_7,
	  MI424WR_K0_MPP8_15,
	  MI424WR_K0_MPP16_23,
	  MI424WR_K0_MPP24_31,
	  MI424WR_K0_MPP32_39,
	  MI424WR_K0_MPP40_47,
	  MI424WR_K0_MPP48_55,
	  MI424WR_K0_MPP56_63,
	  MI424WR_K0_MPP64_71,
	  MI424WR_K0_MPP72_79,
	  MI424WR_K0_MPP80_87,
	  MI424WR_K0_MPP88_88
	  }
	 }
};

MV_BOARD_SWITCH_INFO mi424wr_k0_InfoBoardSwitchValue[] = {
	{
	 .switchIrq = 29,
	 .switchPort = {1, 2, 3, 0, -1, -1, -1, -1},
	 .cpuPort = 4,
	 .connectedPort = {4, -1, -1},
	 .smiScanMode = 2,
	 .quadPhyAddr = 0
	 }
};

MV_BOARD_INFO mi424wr_k0_Info = {
	.boardName = "MI424WR-K0",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(mi424wr_k0_InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = mi424wr_k0_InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(mi424wr_k0_InfoBoardMppConfigValue),
	.pBoardMppConfigValue = mi424wr_k0_InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(mi424wr_InfoBoardDeCsInfo),
	.pDevCsInfo = mi424wr_InfoBoardDeCsInfo,
	.numBoardTwsiDev = 0,
	.pBoardTwsiDev = NULL,
	.numBoardMacInfo = MV_ARRAY_SIZE(mi424wr_k0_InfoBoardMacInfo),
	.pBoardMacInfo = mi424wr_k0_InfoBoardMacInfo,
	.numBoardGppInfo = 0,
	.pBoardGppInfo = NULL,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = MI424WR_K0_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = MI424WR_K0_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = MI424WR_K0_GPP_OUT_ENA_HIGH,
	.gppOutValLow = MI424WR_K0_GPP_OUT_VAL_LOW,
	.gppOutValMid = MI424WR_K0_GPP_OUT_VAL_MID,
	.gppOutValHigh = MI424WR_K0_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = MI424WR_K0_GPP_POL_LOW,
	.gppPolarityValMid = MI424WR_K0_GPP_POL_MID,
	.gppPolarityValHigh = MI424WR_K0_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = mi424wr_k0_InfoBoardSwitchValue,
	.switchInfoNum = MV_ARRAY_SIZE(mi424wr_k0_InfoBoardSwitchValue),

	/* PON configuration. */
	.ponConfigValue = BOARD_PON_NONE,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {},
	.pBoardTdmInt2CsInfo = { mi424wr_Tdm880 },
	.boardTdmInfoIndex = -1,

	/* NAND init params */
	.nandFlashReadParams = MI424WR_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = MI424WR_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = MI424WR_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = mi424wr_TdmSpiInfo
};

/* MI424WR-K1 */
MV_BOARD_MAC_INFO mi424wr_k1_InfoBoardMacInfo[] = {
	{BOARD_MAC_SPEED_1000M, 0x4},
	{BOARD_MAC_SPEED_AUTO, 0x9},
};

MV_BOARD_MPP_TYPE_INFO mi424wr_k1_InfoBoardMppTypeInfo[] = {
	{
	 .boardMppTdm = MV_BOARD_TDM_2CH, /* XXX */
	 .ethSataComplexOpt = ESC_OPT_RGMIIA_MAC0 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_SATA,
	 .ethPortsMode = 0x0
	}
};

MV_BOARD_MPP_INFO mi424wr_k1_InfoBoardMppConfigValue[] = {
	{{
	  MI424WR_K1_MPP0_7,
	  MI424WR_K1_MPP8_15,
	  MI424WR_K1_MPP16_23,
	  MI424WR_K1_MPP24_31,
	  MI424WR_K1_MPP32_39,
	  MI424WR_K1_MPP40_47,
	  MI424WR_K1_MPP48_55,
	  MI424WR_K1_MPP56_63,
	  MI424WR_K1_MPP64_71,
	  MI424WR_K1_MPP72_79,
	  MI424WR_K1_MPP80_87,
	  MI424WR_K1_MPP88_88
	  }
	 }
};

MV_BOARD_SWITCH_INFO mi424wr_k1_InfoBoardSwitchValue[] = {
	{
	 .switchIrq = 29,
	 .switchPort = {1, 2, 3, 0, -1, -1, -1, -1},
	 .cpuPort = 4,
	 .connectedPort = {4, -1, -1},
	 .smiScanMode = 2,
	 .quadPhyAddr = 0
	 }
};

MV_BOARD_INFO mi424wr_k1_Info = {
	.boardName = "MI424WR-K1",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(mi424wr_k1_InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = mi424wr_k1_InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(mi424wr_k1_InfoBoardMppConfigValue),
	.pBoardMppConfigValue = mi424wr_k1_InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(mi424wr_InfoBoardDeCsInfo),
	.pDevCsInfo = mi424wr_InfoBoardDeCsInfo,
	.numBoardTwsiDev = 0,
	.pBoardTwsiDev = NULL,
	.numBoardMacInfo = MV_ARRAY_SIZE(mi424wr_k1_InfoBoardMacInfo),
	.pBoardMacInfo = mi424wr_k1_InfoBoardMacInfo,
	.numBoardGppInfo = 0,
	.pBoardGppInfo = NULL,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = MI424WR_K1_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = MI424WR_K1_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = MI424WR_K1_GPP_OUT_ENA_HIGH,
	.gppOutValLow = MI424WR_K1_GPP_OUT_VAL_LOW,
	.gppOutValMid = MI424WR_K1_GPP_OUT_VAL_MID,
	.gppOutValHigh = MI424WR_K1_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = MI424WR_K1_GPP_POL_LOW,
	.gppPolarityValMid = MI424WR_K1_GPP_POL_MID,
	.gppPolarityValHigh = MI424WR_K1_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = mi424wr_k1_InfoBoardSwitchValue,
	.switchInfoNum = MV_ARRAY_SIZE(mi424wr_k1_InfoBoardSwitchValue),

	/* PON configuration. */
	.ponConfigValue = BOARD_GPON_CONFIG,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {},
	.pBoardTdmInt2CsInfo = { mi424wr_Tdm880 },
	.boardTdmInfoIndex = -1,

	/* NAND init params */
	.nandFlashReadParams = MI424WR_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = MI424WR_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = MI424WR_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = mi424wr_TdmSpiInfo
};

/* MI424WR-K2 */
MV_BOARD_GPP_INFO mi424wr_k2_InfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
	{BOARD_GPP_PEX_RESET, 20},
};

MV_BOARD_MAC_INFO mi424wr_k2_InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_1000M, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9},
};

MV_BOARD_MPP_TYPE_INFO mi424wr_k2_InfoBoardMppTypeInfo[] = {
	{
	 .boardMppTdm = MV_BOARD_TDM_2CH, /* XXX */
	 .ethSataComplexOpt = ESC_OPT_QSGMII | ESC_OPT_GEPHY_MAC1 | ESC_OPT_MAC0_2_SW_P4,
	 .ethPortsMode = 0x0,
	}
};

MV_BOARD_MPP_INFO mi424wr_k2_InfoBoardMppConfigValue[] = {
	{{
	  MI424WR_K2_MPP0_7,
	  MI424WR_K2_MPP8_15,
	  MI424WR_K2_MPP16_23,
	  MI424WR_K2_MPP24_31,
	  MI424WR_K2_MPP32_39,
	  MI424WR_K2_MPP40_47,
	  MI424WR_K2_MPP48_55,
	  MI424WR_K2_MPP56_63,
	  MI424WR_K2_MPP64_71,
	  MI424WR_K2_MPP72_79,
	  MI424WR_K2_MPP80_87,
	  MI424WR_K2_MPP88_88
	  }
	 }
};

MV_BOARD_SWITCH_INFO mi424wr_k2_InfoBoardSwitchValue[] = {
	{
	 .switchIrq = 29,
	 .switchPort = {1, 2, 3, 0, -1, -1, -1, -1},
	 .cpuPort = 4,
	 .connectedPort = {4, -1, -1},
	 .smiScanMode = 2,
	 .quadPhyAddr = 0
	 }
};

MV_BOARD_INFO mi424wr_k2_Info = {
	.boardName = "MI424WR-K2",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(mi424wr_k2_InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = mi424wr_k2_InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(mi424wr_k2_InfoBoardMppConfigValue),
	.pBoardMppConfigValue = mi424wr_k2_InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(mi424wr_InfoBoardDeCsInfo),
	.pDevCsInfo = mi424wr_InfoBoardDeCsInfo,
	.numBoardTwsiDev = 0,
	.pBoardTwsiDev = NULL,
	.numBoardMacInfo = MV_ARRAY_SIZE(mi424wr_k2_InfoBoardMacInfo),
	.pBoardMacInfo = mi424wr_k2_InfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(mi424wr_k2_InfoBoardGppInfo),
	.pBoardGppInfo = mi424wr_k2_InfoBoardGppInfo,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = MI424WR_K2_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = MI424WR_K2_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = MI424WR_K2_GPP_OUT_ENA_HIGH,
	.gppOutValLow = MI424WR_K2_GPP_OUT_VAL_LOW,
	.gppOutValMid = MI424WR_K2_GPP_OUT_VAL_MID,
	.gppOutValHigh = MI424WR_K2_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = MI424WR_K2_GPP_POL_LOW,
	.gppPolarityValMid = MI424WR_K2_GPP_POL_MID,
	.gppPolarityValHigh = MI424WR_K2_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = mi424wr_k2_InfoBoardSwitchValue,
	.switchInfoNum = MV_ARRAY_SIZE(mi424wr_k2_InfoBoardSwitchValue),

	/* PON configuration. */
	.ponConfigValue = BOARD_GPON_CONFIG,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {},
	.pBoardTdmInt2CsInfo = { mi424wr_Tdm880 },
	.boardTdmInfoIndex = -1,

	/* NAND init params */
	.nandFlashReadParams = MI424WR_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = MI424WR_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = MI424WR_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = mi424wr_TdmSpiInfo
};

/***************************************************************************
** Customer board place holder
****************************************************************************/

#define DB_CUSTOMER_BOARD_PCI_IF_NUM		        0x0
#define DB_CUSTOMER_BOARD_TWSI_DEF_NUM		        0x0
#define DB_CUSTOMER_BOARD_MAC_INFO_NUM		        0x0
#define DB_CUSTOMER_BOARD_GPP_INFO_NUM		        0x0
#define DB_CUSTOMER_BOARD_MPP_GROUP_TYPE_NUN        0x0
#define DB_CUSTOMER_BOARD_MPP_CONFIG_NUM		    0x0
#define DB_CUSTOMER_BOARD_DEVICE_CONFIG_NUM	    	0x0
#define DB_CUSTOMER_BOARD_DEBUG_LED_NUM				0x0
#define DB_CUSTOMER_BOARD_NAND_READ_PARAMS		    0x000E02C2
#define DB_CUSTOMER_BOARD_NAND_WRITE_PARAMS		    0x00010305
#define DB_CUSTOMER_BOARD_NAND_CONTROL		        0x01c00543

MV_U8 dbCustomerInfoBoardDebugLedIf[] = { 0 };

MV_BOARD_MAC_INFO dbCustomerInfoBoardMacInfo[] =
    /* {{MV_BOARD_MAC_SPEED     boardMacSpeed,  MV_U8   boardEthSmiAddr}} */
{ {BOARD_MAC_SPEED_AUTO, 0x0}
};

MV_BOARD_TWSI_INFO dbCustomerInfoBoardTwsiDev[] =
    /* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
{ {BOARD_TWSI_OTHER, 0x0, ADDR7_BIT}
};

MV_BOARD_MPP_TYPE_INFO dbCustomerInfoBoardMppTypeInfo[] = { {MV_BOARD_OTHER, MV_BOARD_OTHER, 0x0}
};

MV_DEV_CS_INFO dbCustomerInfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#if defined(MTD_NAND_LNC)
	{NAND_NOR_CS, N_A, BOARD_DEV_NAND_FLASH, 8}	/* NAND DEV */
#elif defined(MV_SPI)
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8}	/* SPI DEV */
#else
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO dbCustomerInfoBoardMppConfigValue[] = {
	{
		{
			DB_CUSTOMER_MPP0_7,
			DB_CUSTOMER_MPP8_15,
			DB_CUSTOMER_MPP16_23,
			DB_CUSTOMER_MPP24_31,
			DB_CUSTOMER_MPP32_39,
			DB_CUSTOMER_MPP40_47,
			DB_CUSTOMER_MPP48_55
		}
	}
};

MV_BOARD_INFO dbCustomerInfo = { };

MV_BOARD_INFO *boardInfoTbl[] = {
	&db88f6535Info,
	&rd88f6510Info,
	&rd88f6560Info,
	&rd88f6530Info,
	&db88f6560PCACPInfo,
	&mi424wr_i_Info,
	&mi424wr_j_Info,
	&mi424wr_k0_Info,
	&mi424wr_k1_Info,
	&mi424wr_k2_Info,
	&dbCustomerInfo
};
