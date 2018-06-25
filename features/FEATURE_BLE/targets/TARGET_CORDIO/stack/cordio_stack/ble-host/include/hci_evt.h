/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  HCI event module.
 *
 *  Copyright (c) 2009-2018 Arm Ltd. All Rights Reserved.
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
#ifndef HCI_EVT_H
#define HCI_EVT_H

#ifdef __cplusplus
extern "C" {
#endif

/*! \addtogroup STACK_HCI_API
 *  \{ */

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief HCI event statistics */
typedef struct
{
  uint16_t numDiscCmplEvt;              /*!< \brief Number discovery complete events. */
  uint16_t numEncChangeEvt;             /*!< \brief Number encryption change events. */
  uint16_t numReadRemoteVerInfoCmpEvt;  /*!< \brief Number read remote version info complete events. */
  uint16_t numCmdCmplEvt;               /*!< \brief Number command complete events. */
  uint16_t numCmdStatusEvt;             /*!< \brief Number command status events. */
  uint16_t numHwErrorEvt;               /*!< \brief Number hardware error events. */
  uint16_t numCmplPktsEvt;              /*!< \brief Number complete packet events. */
  uint16_t numDataBufOverflowEvt;       /*!< \brief Number data buf overflow events. */
  uint16_t numEncKeyRefreshCmplEvt;     /*!< \brief Number encryption key refresh complete events. */
  uint16_t numLeMetaEvt;                /*!< \brief Number LE meta events. */
  uint16_t numVendorSpecEvt;            /*!< \brief Number vendor specific events. */
  uint16_t numAuthToEvt;                /*!< \brief Number authenticated to events. */
} hciEvtStats_t;

/*! \} */    /* STACK_HCI_API */

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/


/*************************************************************************************************/
/*!
 *  \brief  Process received HCI events.
 *
 *  \param  pEvt    Buffer containing HCI event.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciEvtProcessMsg(uint8_t *pEvt);

/*************************************************************************************************/
/*!
 *  \brief  Get event statistics.
 *
 *  \return Event statistics.
 */
/*************************************************************************************************/
hciEvtStats_t *hciEvtGetStats(void);

#ifdef __cplusplus
};
#endif

#endif /* HCI_EVT_H */
