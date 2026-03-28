/*
 *  Copyright (C) 2021 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Auto generated file
 */
#include "ti_board_config.h"

/*
 * Auto generated file
 */




/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <enet.h>
#include <dp83869.h>
#include <enet_apputils.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/AddrTranslateP.h>
#include <networking/enet/core/src/phy/enetphy_priv.h>
#include <networking/enet/core/src/phy/generic_phy.h>
#include <drivers/gpio.h>
#include <board/eeprom.h>
#include "ti_board_open_close.h"
#include "ti_drivers_open_close.h"
#include "ti_drivers_config.h"

#include <drivers/gpio.h>
#include <kernel/dpl/AddrTranslateP.h>
#include "ti_drivers_config.h"

#define ENET_BOARD_NUM_MACADDR_MAX (3U)
#define I2C_EEPROM_MAC_DATA_OFFSET (0x3D)
#define I2C_EEPROM_MAC_CTRL_OFFSET (0x3B)
#define I2C_EEPROM_PCB_REV_OFFSET  (0x23)

#define ENET_GET_NUM_MAC_ADDR(num) ((num>>3)+1)
#define ENET_MAC_ADDR_VALIDATE_MASK (0x01U)

#define MSS_CPSW_CONTROL_PORT_MODE_RMII                                   (0x1U)
#define MSS_CPSW_CONTROL_PORT_MODE_RGMII                                  (0x2U)


/* PHY drivers */
extern Phy_DrvObj_t gEnetPhyDrvDp83869;
extern Phy_DrvObj_t gEnetPhyDrvGeneric;

/*! \brief All the registered PHY specific drivers. */
static const EthPhyDrv_If gEnetPhyDrvs[] =
{
    &gEnetPhyDrvDp83869,    /* DP83869 */
    &gEnetPhyDrvGeneric,    /* Generic PHY - must be last */
};

const EnetPhy_DrvInfoTbl gEnetPhyDrvTbl =
{
    .numHandles = ENET_ARRAYSIZE(gEnetPhyDrvs),
    .hPhyDrvList = gEnetPhyDrvs,
};


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static const EnetBoard_PortCfg *EnetBoard_getPortCfg(const EnetBoard_EthPort *ethPort);

static const EnetBoard_PortCfg *EnetBoard_findPortCfg(const EnetBoard_EthPort *ethPort,
                                                      const EnetBoard_PortCfg *ethPortCfgs,
                                                      uint32_t numEthPorts);

static void EnetBoard_setEnetControl(Enet_Type enetType,
                                     Enet_MacPort macPort,
                                     EnetMacPort_Interface *mii);

static void EnetBoard_enableExternalMux();

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
/*!
 * \brief Common Processor Board (CPB) board's DP83869 PHY configuration.
 */
static const Dp83869_Cfg gEnetCpbBoard_ConfigEnetEthphy0PhyCfg =
{
/* The delay values are set based on trial and error and not tuned per port of the evm */
	/* Extended PHY configuration for DP83869*/
.txClkShiftEn         = false,
.rxClkShiftEn         = true,
.txDelayInPs          = 500U, /* Value in pecosec. Refer to DLL_RX_DELAY_CTRL_SL field in ANA_RGMII_DLL_CTRL register of DP83869 PHY datasheet */
.rxDelayInPs          = 500U, /* Value in pecosec. Refer to DLL_TX_DELAY_CTRL_SL field in ANA_RGMII_DLL_CTRL register of DP83869 PHY datasheet */
.txFifoDepth          = 4U,
.impedanceInMilliOhms = 35000,  /* 35 ohms */
.idleCntThresh        = 4U,     /* Improves short cable performance */
.gpio0Mode            = DP83869_GPIO0_LED3,
.gpio1Mode            = DP83869_GPIO1_COL, /* Unused */
.ledMode              =
{
    DP83869_LED_LINKED,         /* Unused */
    DP83869_LED_LINKED_100BTX,
    DP83869_LED_RXTXACT,
    DP83869_LED_LINKED_1000BT,
},
};
/*!
 * \brief Common Processor Board (CPB) board's DP83869 PHY configuration.
 */
static const Dp83869_Cfg gEnetCpbBoard_ConfigEnetEthphy1PhyCfg =
{
/* The delay values are set based on trial and error and not tuned per port of the evm */
	/* Extended PHY configuration for DP83869*/
.txClkShiftEn         = false,
.rxClkShiftEn         = true,
.txDelayInPs          = 500U, /* Value in pecosec. Refer to DLL_RX_DELAY_CTRL_SL field in ANA_RGMII_DLL_CTRL register of DP83869 PHY datasheet */
.rxDelayInPs          = 500U, /* Value in pecosec. Refer to DLL_TX_DELAY_CTRL_SL field in ANA_RGMII_DLL_CTRL register of DP83869 PHY datasheet */
.txFifoDepth          = 4U,
.impedanceInMilliOhms = 35000,  /* 35 ohms */
.idleCntThresh        = 4U,     /* Improves short cable performance */
.gpio0Mode            = DP83869_GPIO0_LED3,
.gpio1Mode            = DP83869_GPIO1_COL, /* Unused */
.ledMode              =
{
    DP83869_LED_LINKED,         /* Unused */
    DP83869_LED_LINKED_100BTX,
    DP83869_LED_RXTXACT,
    DP83869_LED_LINKED_1000BT,
},
};
/*
 * am243x-lp board configuration.
 *
 * RMII/RGMII PHY connected to am243x-lp CPSW_3G MAC port.
 */
static const EnetBoard_PortCfg gEnetCpbBoard_am243x_lp_EthPort[] =
{
    {    /* "CPSW3G" */
        .enetType = ENET_CPSW_3G,
        .instId   = 0U,
        .macPort  = ENET_MAC_PORT_1,
        .mii      = {ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED},
        .phyCfg   =
        {
            .phyAddr         = 3,
            .isStrapped      = false,
            .skipExtendedCfg = false,
			.extendedCfg     = &gEnetCpbBoard_ConfigEnetEthphy0PhyCfg,
			.extendedCfgSize = sizeof(gEnetCpbBoard_ConfigEnetEthphy0PhyCfg)
        },
        .flags    = 0U,
    },
    {    /* "CPSW3G" */
        .enetType = ENET_CPSW_3G,
        .instId   = 0U,
        .macPort  = ENET_MAC_PORT_2,
        .mii      = {ENET_MAC_LAYER_GMII, ENET_MAC_SUBLAYER_REDUCED},
        .phyCfg   =
        {
            .phyAddr         = 15,
            .isStrapped      = false,
            .skipExtendedCfg = false,
			.extendedCfg     = &gEnetCpbBoard_ConfigEnetEthphy1PhyCfg,
			.extendedCfgSize = sizeof(gEnetCpbBoard_ConfigEnetEthphy1PhyCfg)
        },
        .flags    = 0U,
    },
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

const EnetBoard_PhyCfg *EnetBoard_getPhyCfg(const EnetBoard_EthPort *ethPort)
{
    const EnetBoard_PortCfg *portCfg;

    portCfg = EnetBoard_getPortCfg(ethPort);

    return (portCfg != NULL) ? &portCfg->phyCfg : NULL;
}

static void EnetBoard_enableExternalMux()
{

    /* For AM243x-LP E3 board:
    *  - GPIO_27 - RGMII1 MUX select PIN: (Y20)
    *        HIGH to select CPSW
    *        LOW  to select ICSS
    *  - GPIO_26 - RGMII1 PHY RST PIN: (W20)
    *        LOW to RESET PHY
    */
    /* For AM243x-LP E2 board:
    *  - GPIO_70 - RGMII1 MUX select PIN: (AA11)
    *        HIGH to select CPSW
    *        LOW  to select ICSS
    */
    /* Address translate and Setup GPIO for CPSW Mux selection*/
#ifdef CONFIG_ENET_RGMII_MUX_SEL_PIN
    uint32_t prgCpswRgmii1MuxSel = (uint32_t) AddrTranslateP_getLocalAddr(CONFIG_ENET_RGMII_MUX_SEL_BASE_ADDR);
    GPIO_setDirMode(prgCpswRgmii1MuxSel, CONFIG_ENET_RGMII_MUX_SEL_PIN, CONFIG_ENET_RGMII_MUX_SEL_DIR);
    GPIO_pinWriteHigh(prgCpswRgmii1MuxSel, CONFIG_ENET_RGMII_MUX_SEL_PIN);
#endif

#ifdef CONFIG_ENET_RGMII_PHY_RST_PIN
    uint32_t rgmii1PhyRst        = (uint32_t) AddrTranslateP_getLocalAddr(CONFIG_ENET_RGMII_PHY_RST_BASE_ADDR);
    GPIO_setDirMode(rgmii1PhyRst, CONFIG_ENET_RGMII_PHY_RST_PIN, CONFIG_ENET_RGMII_PHY_RST_DIR);
    GPIO_pinWriteHigh(rgmii1PhyRst, CONFIG_ENET_RGMII_PHY_RST_PIN);
#endif
}

static const EnetBoard_PortCfg *EnetBoard_getPortCfg(const EnetBoard_EthPort *ethPort)
{
    const EnetBoard_PortCfg *portCfg = NULL;

    if (ENET_NOT_ZERO(ethPort->boardId & ENETBOARD_CPB_ID) || 
        ((portCfg == NULL) && ENET_NOT_ZERO(ethPort->boardId & ENETBOARD_LOOPBACK_ID)))
    {
        portCfg = EnetBoard_findPortCfg(ethPort,
                                        gEnetCpbBoard_am243x_lp_EthPort,
                                        ENETPHY_ARRAYSIZE(gEnetCpbBoard_am243x_lp_EthPort));
    }
    return portCfg;
}

static const EnetBoard_PortCfg *EnetBoard_findPortCfg(const EnetBoard_EthPort *ethPort,
                                                      const EnetBoard_PortCfg *ethPortCfgs,
                                                      uint32_t numEthPorts)
{
    const EnetBoard_PortCfg *ethPortCfg = NULL;
    bool found = false;
    uint32_t i;

    for (i = 0U; i < numEthPorts; i++)
    {
        ethPortCfg = &ethPortCfgs[i];

        if ((ethPortCfg->enetType == ethPort->enetType) &&
            (ethPortCfg->instId == ethPort->instId) &&
            (ethPortCfg->macPort == ethPort->macPort) &&
            (ethPortCfg->mii.layerType == ethPort->mii.layerType) &&
            (ethPortCfg->mii.sublayerType == ethPort->mii.sublayerType))
        {
            found = true;
            break;
        }
    }

    return found ? ethPortCfg : NULL;
}

void EnetBoard_getMiiConfig(EnetMacPort_Interface *mii)
{
    mii->layerType      = ENET_MAC_LAYER_GMII;
    mii->variantType    = ENET_MAC_VARIANT_FORCED;
    mii->sublayerType   = ENET_MAC_SUBLAYER_REDUCED;
}

int32_t EnetBoard_setupPorts(EnetBoard_EthPort *ethPorts,
                             uint32_t numEthPorts)
{
    CSL_main_ctrl_mmr_cfg0Regs *regs = (CSL_main_ctrl_mmr_cfg0Regs *)(uintptr_t)CSL_CTRL_MMR0_CFG0_BASE;
    EnetAppUtils_MmrLockState prevLockState;

    DebugP_assert(numEthPorts == 1);
    DebugP_assert(ethPorts->mii.sublayerType == ENET_MAC_SUBLAYER_REDUCED);

    EnetBoard_enableExternalMux();
    /* Override the ENET control set by board lib */
    EnetBoard_setEnetControl(ethPorts->enetType, ethPorts->macPort, &ethPorts->mii);

    prevLockState = EnetAppUtils_mainMmrCtrl(ENETAPPUTILS_MMR_LOCK1, ENETAPPUTILS_UNLOCK_MMR);

    switch(ethPorts->macPort)
    {
        case ENET_MAC_PORT_1:
            CSL_FINS (regs->ENET1_CTRL, MAIN_CTRL_MMR_CFG0_ENET1_CTRL_PORT_MODE_SEL, MSS_CPSW_CONTROL_PORT_MODE_RGMII);
            break;
        case ENET_MAC_PORT_2:
            CSL_FINS (regs->ENET2_CTRL, MAIN_CTRL_MMR_CFG0_ENET2_CTRL_PORT_MODE_SEL, MSS_CPSW_CONTROL_PORT_MODE_RGMII);
            break;
        default:
            DebugP_assert(false);
    }

    if (prevLockState == ENETAPPUTILS_LOCK_MMR)
    {
        EnetAppUtils_mainMmrCtrl(ENETAPPUTILS_MMR_LOCK1, ENETAPPUTILS_LOCK_MMR);
    }

    /* Nothing else to do */
    return ENET_SOK;
}

static void EnetBoard_setEnetControl(Enet_Type enetType,
                                     Enet_MacPort macPort,
                                     EnetMacPort_Interface *mii)
{

}

void EnetBoard_getMacAddrList(uint8_t macAddr[][ENET_MAC_ADDR_LEN],
                              uint32_t maxMacEntries,
                              uint32_t *pAvailMacEntries)
{
    int32_t status = ENET_SOK;
    uint32_t macAddrCnt;
    uint32_t i;
    uint8_t macAddrBuf[ENET_BOARD_NUM_MACADDR_MAX * ENET_MAC_ADDR_LEN];
    uint8_t numMacMax;
    uint8_t validNumMac = 0;

    status = EEPROM_read(gEepromHandle[CONFIG_EEPROM0],  I2C_EEPROM_MAC_CTRL_OFFSET, &numMacMax, sizeof(uint8_t));
    EnetAppUtils_assert(status == ENET_SOK);
    EnetAppUtils_assert(ENET_GET_NUM_MAC_ADDR(numMacMax) <= ENET_BOARD_NUM_MACADDR_MAX);
    macAddrCnt = EnetUtils_min(ENET_GET_NUM_MAC_ADDR(numMacMax), maxMacEntries);

    EnetAppUtils_assert(pAvailMacEntries != NULL);

    status = EEPROM_read(gEepromHandle[CONFIG_EEPROM0], I2C_EEPROM_MAC_DATA_OFFSET, macAddrBuf, (macAddrCnt * ENET_MAC_ADDR_LEN));
    EnetAppUtils_assert(status == ENET_SOK);

    /* Save only those required to meet the max number of MAC entries */
    /* Validating that the MAC addresses from the EEPROM are not MULTICAST addresses */
    for (i = 0U; i < macAddrCnt; i++)
    {
        if(!(macAddrBuf[i * ENET_MAC_ADDR_LEN] & ENET_MAC_ADDR_VALIDATE_MASK)){
            memcpy(macAddr[validNumMac], &macAddrBuf[i * ENET_MAC_ADDR_LEN], ENET_MAC_ADDR_LEN);
            validNumMac++;
        }
    }

    *pAvailMacEntries = validNumMac;

    if (macAddrCnt == 0U)
    {
        EnetAppUtils_print("EnetBoard_getMacAddrList Failed - IDK not present\n");
        EnetAppUtils_assert(false);
    }
}


/*
 * Get ethernet board id
 */
uint32_t EnetBoard_getId(void)
{
    return ENETBOARD_AM64X_AM243X_EVM;
}



void Board_init(void)
{

}

void Board_deinit(void)
{


}
