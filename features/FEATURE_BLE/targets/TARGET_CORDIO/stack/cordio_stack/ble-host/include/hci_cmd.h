/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  HCI command module.
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
#ifndef HCI_CMD_H
#define HCI_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Send an HCI command and service the HCI command queue.
 *
 *  \param  pData  Buffer containing HCI command to send or NULL.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciCmdSend(uint8_t *pData);

/*************************************************************************************************/
/*!
 *  \brief  Allocate an HCI command buffer and set the command header fields.
 *
 *  \param  opcode  Command opcode.
 *  \param  len     length of command parameters.
 *
 *  \return Pointer to WSF msg buffer.
 */
/*************************************************************************************************/
uint8_t *hciCmdAlloc(uint16_t opcode, uint16_t len);

/*************************************************************************************************/
/*!
 *  \brief  Initialize the HCI cmd module.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciCmdInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Process an HCI command timeout.
 *
 *  \param  pMsg  Message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciCmdTimeout(wsfMsgHdr_t *pMsg);

/*************************************************************************************************/
/*!
 *  \brief  Process an HCI Command Complete or Command Status event.
 *
 *  \param  numCmdPkts  Number of commands that can be sent to the controller.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciCmdRecvCmpl(uint8_t numCmdPkts);



#ifdef __cplusplus
};
#endif

#endif /* HCI_CMD_H */
