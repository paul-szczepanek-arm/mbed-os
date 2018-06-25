/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  Token trace decode header file.
 *
 *  Copyright (c) 2018 Arm Ltd. All Rights Reserved.
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

#ifndef WSF_DETOKEN_H
#define WSF_DETOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 Macros
**************************************************************************************************/

/*! \brief Vendor specific event mask opcode. */
#define WSF_DETOKEN_VS_SET_EVENT_MASK_OPCODE    HCI_OPCODE(HCI_OGF_VENDOR_SPEC, 0x3E1)

/*! \brief Event mask bits. */
#define WSF_DETOKEN_ENABLE_BIT                  (1<<1)

/*! \brief Vendor specific token event. */
#define WSF_DETOKEN_VS_EVT_TOKEN                0xFFF0

/*! \brief Platform identifiers. */
#define WSF_DETOKEN_TRACE_NORDIC                1
#define WSF_DETOKEN_TRACE_BT4                   2
#define WSF_DETOKEN_TRACE_C50                   3

/*! \brief Parameter mask bits. */
#define WSF_DETOKEN_PARAM_VARIABLE              0
#define WSF_DETOKEN_PARAM_STRING                1

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Initialize detoken trace.
 *
 *  \return None.
 */
/*************************************************************************************************/
void WsfDetokenInit(void);

/*************************************************************************************************/
/*!
 *  \brief  Enable/disable detoken trace.
 *
 *  \param  enable    TRUE to enable, FALSE to disable.
 *
 *  \return None.
 */
/*************************************************************************************************/
void WsfDetokenEnable(bool_t enable);

/*************************************************************************************************/
/*!
 *  \brief  Process vendor specific HCI events and decode token trace events from the LL.
 *
 *  \param  len       Length of pBuffer in bytes.
 *  \param  pBuffer   Buffer containing HCI event.
 *
 *  \return TRUE if VS HCI message is a token, else FALSE.
 */
/*************************************************************************************************/
bool_t WsfDetokenProcessHciEvent(uint16_t len, uint8_t *pBuffer);

#ifdef __cplusplus
};
#endif

#endif /* WSF_DETOKEN_H */
