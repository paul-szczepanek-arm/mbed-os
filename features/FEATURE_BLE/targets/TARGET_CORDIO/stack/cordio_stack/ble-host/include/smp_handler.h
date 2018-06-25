/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief  Interface to SMP event handler.
 *
 *  Copyright (c) 2010-2018 Arm Ltd. All Rights Reserved.
 *  ARM Ltd. confidential and proprietary.
 *
 *  IMPORTANT.  Your use of this file is governed by a Software License Agreement
 *  ("Agreement") that must be accepted in order to download or otherwise receive a
 *  copy of this file.  You may not use or copy this file for any purpose other than
 *  as described in the Agreement.  If you do not agree to all of the terms of the
 *  Agreement do not use this file and delete all copies in your possession or control;
 *  if you do not have a copy of the Agreement, you must contact ARM Ltd. prior
 *  to any use, copying or further distribution of this software.
 */
/*************************************************************************************************/
#ifndef SMP_HANDLER_H
#define SMP_HANDLER_H

#include "wsf_os.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \addtogroup STACK_EVENT
 *  \{ */

/** \name SMP Event Handling
 * Message passing interface to SMP from other tasks through WSF.
 */
/**@{*/

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  SMP handler init function called during system initialization.
 *
 *  \param  handlerId  WSF handler ID for SMP.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SmpHandlerInit(wsfHandlerId_t handlerId);


/*************************************************************************************************/
/*!
 *  \brief  WSF event handler for SMP.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SmpHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);

/**@}*/

/*! \} */    /* STACK_SMP_API */

#ifdef __cplusplus
};
#endif

#endif /* SMP_HANDLER_H */
