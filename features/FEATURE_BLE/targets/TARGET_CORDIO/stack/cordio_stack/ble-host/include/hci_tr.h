/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  HCI transport interface.
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
#ifndef HCI_TR_H
#define HCI_TR_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Send a complete HCI ACL packet to the transport.
 *
 *  \param  pContext    Connection context.
 *  \param  pAclData    WSF msg buffer containing an ACL packet.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciTrSendAclData(void *pContext, uint8_t *pAclData);

/*************************************************************************************************/
/*!
 *  \brief  Send a complete HCI command to the transport.
 *
 *  \param  pCmdData    WSF msg buffer containing an HCI command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciTrSendCmd(uint8_t *pCmdData);

/*************************************************************************************************/
/*!
 *  \brief  Initialize HCI transport resources.
 *
 *  \param  port        COM port.
 *  \param  baudRate    Baud rate.
 *  \param  flowControl TRUE if flow control is enabled
 *
 *  \return TRUE if initialization succeeds, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t hciTrInit(uint8_t port, uint32_t baudRate, bool_t flowControl);

/*************************************************************************************************/
/*!
 *  \brief  Close HCI transport resources.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciTrShutdown(void);

#ifdef __cplusplus
};
#endif

#endif /* HCI_TR_H */
