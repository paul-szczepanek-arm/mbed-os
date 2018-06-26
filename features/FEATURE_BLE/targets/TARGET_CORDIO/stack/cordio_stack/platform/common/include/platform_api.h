/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief      Platform API file.
 *
 *  Copyright (c) 2013-2018 Arm Ltd. All Rights Reserved.
 *  Arm Ltd. confidential and proprietary.
 *
 *  IMPORTANT.  Your use of this file is governed by a Software License Agreement
 *  ("Agreement") that must be accepted in order to download or otherwise receive a
 *  copy of this file.  You may not use or copy this file for any purpose other than
 *  as described in the Agreement.  If you do not agree to all of the terms of the
 *  Agreement do not use this file and delete all copies in your possession or control;
 *  if you do not have a copy of the Agreement, you must contact Arm Ltd. prior
 *  to any use, copying or further distribution of this software.
 */
/*************************************************************************************************/

#ifndef PLATFORM_API_H
#define PLATFORM_API_H

#include "wsf_types.h"

/**************************************************************************************************
  Type Definitions
**************************************************************************************************/

/*! \brief      Status callback. */
typedef bool_t (*PlatformStatus_t)(uint32_t *);

/*! \brief      Sleep enter callback. */
typedef void (*PlatformSleepEnter_t)(void);

/*! \brief      Sleep exit callback. */
typedef void (*PlatformSleepExit_t)(void);

/*! \brief  BB configuration. */
typedef struct
{
  uint16_t clkPpm;                  /*!< Clock accuracy in PPM. */
  uint8_t  rfSetupDelayUsec;        /*!< RF setup delay in microseconds. */
  uint16_t maxScanPeriodMsec;       /*!< Maximum scan period in milliseconds. */
  uint16_t schSetupDelayUsec;       /*!< Schedule setup delay in microseconds. */
} PlatformBbCfg_t;

/*! \brief  Version component IDs. */
typedef enum
{
  PLATFORM_VER_ID_RADIO     = 0,    /*!< Radio version. */
  PLATFORM_VER_ID_HW_BLE    = 1,    /*!< BLE HW version. */
  PLATFORM_VER_ID_PHY       = 2,    /*!< PHY HW version. */
  PLATFORM_VER_ID_HW_SYS    = 3,    /*!< System HW version. */
  PLATFORM_VER_ID_SW_DRV    = 4,    /*!< Software version. */
  PLATFORM_VER_ID_15P4_MAC  = 5,    /*!< 802.15.4 HW version. */
  PLATFORM_VER_INFO_NUM     = 6     /*!< Total number of version IDs. */
} PlatformVersionId_t;

/*! \brief  Sleep modes. */
typedef enum
{
  PLATFORM_SLEEP_MODE_NONE,
  PLATFORM_SLEEP_MODE_SHALLOW,
  PLATFORM_SLEEP_MODE_DEEP
} PlatformSleepMode_t;

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! \brief      Number of assertions. */
extern uint32_t PlatformAssertCount;

/*! \brief      Trap enabled flag. */
extern volatile bool_t PlatformAssertTrapEnable;

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/* --- Initialization --- */

/*************************************************************************************************/
/*!
 *  \brief      Common platform initialization.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void PlatformInitCommon(void);

/*************************************************************************************************/
/*!
 *  \brief      Hardware initialization for controller operation.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void PlatformInitControllerHardware(void);

/*************************************************************************************************/
/*!
 *  \brief      Hardware initialization for host operation.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void PlatformInitHostHardware(void);

/*************************************************************************************************/
/*!
 *  \brief      Hardware initialization for controller operation.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void PlatformInitBootloaderHardware(void);

/*************************************************************************************************/
/*!
 *  \brief  Initialize Dual Chip LL
 *
 *  \return None
 *
 *  \note   Initilization that takes place after WsfBufInit is performed here
 */
/*************************************************************************************************/
void PlatformInitDualChip(void);

/* --- Bootloader --- */

/*************************************************************************************************/
/*!
 *  \brief      Execute application from RAM.
 *
 *  \return     None.
 *
 *  \note       This routine does not return.
 */
/*************************************************************************************************/
void PlatformExecApplication(void);

/* --- Memory --- */

/*************************************************************************************************/
/*!
 *  \brief  Get memory regions.
 *
 *  \param pCodeMemAddr     If non-NULL, return starting address of code memory.
 *  \param pCodeMemSize     If non-NULL, return size of code memory.
 *  \param pDataMemAddr     If non-NULL, return starting address of data memory.
 *  \param pDataMemSize     If non-NULL, return size of data memory.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformGetMemoryRegions(uint32_t *pCodeMemAddr, uint32_t *pCodeMemSize,
                              uint32_t *pDataMemAddr, uint32_t *pDataMemSize);

/*************************************************************************************************/
/*!
 *  \brief  Get code memory region used by ROM bootloader.
 *
 *  \param pCodeMemAddr     If non-NULL, return starting address of code memory.
 *  \param pCodeMemSize     If non-NULL, return size of code memory.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformGetBootRegions(uint32_t *pCodeMemAddr, uint32_t *pCodeMemSize);

/*************************************************************************************************/
/*!
 *  \brief  Count stack usage.
 *
 *  \return Stack high watermark in bytes.
 */
/*************************************************************************************************/
uint32_t PlatformCountStackUsage(void);

/*************************************************************************************************/
/*!
 *  \brief      Get heap available.
 *
 *  \return     Number of bytes of heap memory available.
 */
/*************************************************************************************************/
uint32_t PlatformGetHeapAvailable(void);

/*************************************************************************************************/
/*!
 *  \brief      Get heap used.
 *
 *  \return     Number of bytes of heap memory used.
 */
/*************************************************************************************************/
uint32_t PlatformGetHeapUsed(void);

/*************************************************************************************************/
/*!
 *  \brief      Reserve heap memory.
 *
 *  \param      size    Number of bytes of heap memory used.
 *
 *  \return     None
 */
/*************************************************************************************************/
void PlatformReserveHeap(uint32_t size);

/*************************************************************************************************/
/*!
 *  \brief      Get next available heap memory.
 *
 *  \return     Address of the start of heap memory.
 */
/*************************************************************************************************/
void *PlatformGetHeapStart(void);

/* --- Configuration --- */

/*************************************************************************************************/
/*!
 *  \brief      Load BB timing configuration.
 *
 *  \param      pCfg                Return configuration values.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void PlatformLoadBbConfig(PlatformBbCfg_t *pCfg);

/* --- Power Management --- */

/*************************************************************************************************/
/*!
 *  \brief      Register sleep callback functions.
 *
 *  \param      protId                   Protocol ID.
 *  \param      statusCback              Callback function for checking status
 *  \param      enterCback               Callback function before entering sleep.
 *  \param      exitCback                Callback function after exiting sleep.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void PlatformRegisterSleep(uint8_t protId, PlatformStatus_t statusCback, PlatformSleepEnter_t enterCback, PlatformSleepExit_t exitCback);

/*************************************************************************************************/
/*!
 *  \brief      Set Sleep mode.
 *
 *  \param      sleepMode                Deep sleep or shallow sleep.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void PlatformSetSleepMode(PlatformSleepMode_t sleepMode);

/*************************************************************************************************/
/*!
 *  \brief      Check if there is an active timer and if there is enough time to
 *              go to sleep.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void PlatformTimeSleep(void);

/*************************************************************************************************/
/*!
 *  \brief      Update WSF timer based on elapsed RTC ticks.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void PlatformTimeUpdate(void);

/* --- Trace --- */

/*************************************************************************************************/
/*!
 *  \brief  Send a trace event.
 *
 *  \param  pBuf      Message.
 *  \param  len       Message length.
 *
 *  \return \ref TRUE is write successful, \ref FALSE otherwise.
 */
/*************************************************************************************************/
bool_t PlatformTraceSendMsg(uint8_t *pBuf, uint8_t len);

/* --- Version --- */

/*************************************************************************************************/
/*!
 *  \brief      Get the hardware version code.
 *
 *  \param      id      Version ID.
 *  \param      pCode   Hardware version code.
 *
 *  \return     TRUE if version code valid, FALSE otherwise.
 *
 *  Interrogate the hardware for its version code.
 */
/*************************************************************************************************/
bool_t PlatformGetVersionCode(PlatformVersionId_t id, uint32_t *pCode);

/*************************************************************************************************/
/*!
 *  \brief  Initialize the terminal.
 *
 *  \return None.
 */
/*************************************************************************************************/
void PlatformTerminalInit(void);

#endif /* PLATFORM_API_H */
