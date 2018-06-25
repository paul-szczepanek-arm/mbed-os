/*************************************************************************************************/
/*!
 *  \file   wstr.h
 *
 *  \brief  String manipulation functions.
 *
 *  Copyright (c) 2014-2018 Arm Ltd. All Rights Reserved.
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

#ifndef WSTR_H
#define WSTR_H

#ifdef __cplusplus
extern "C" {
#endif

/*! \addtogroup WSF_UTIL_API
 *  \{ */

/*************************************************************************************************/
/*!
 *  \brief  Copies a string up to a given length.
 *
 *  \param  pBuf    Pointer to buffer to copy to.
 *  \param  pData   Pointer to the string to copy.
 *  \param  n       Size of pBuf in bytes.
 *
 *  \return none.
 */
/*************************************************************************************************/
void WstrnCpy(char *pBuf, const char *pData, uint8_t n);

/*************************************************************************************************/
/*!
 *  \brief  Byte by byte reverse and copy a buffer.
 *
 *  \param  pBuf1   Buffer to hold reversed copy.
 *  \param  pBuf2   Buffer to copy.
 *  \param  len     Size of pBuf1 and pBuf2 in bytes.
 *
 *  \return None.
 */
/*************************************************************************************************/
void WStrReverseCpy(uint8_t *pBuf1, const uint8_t *pBuf2, uint16_t len);

/*************************************************************************************************/
/*!
 *  \brief  Byte by byte reverse a buffer.
 *
 *  \param  pBuf   Buffer to reverse.
 *  \param  len    size of pBuf in bytes.
 *
 *  \return None.
 */
/*************************************************************************************************/
void WStrReverse(uint8_t *pBuf, uint8_t len);

/*************************************************************************************************/
/*!
 *  \brief  Format a hex value.
 *
 *  \param  pBuf    Storage for string representation of value.
 *  \param  val     Value.
 *  \param  len     Length of value, in bits.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void WStrFormatHex(char *pBuf, uint32_t val, uint8_t len);

/*! \} */    /* WSF_UTIL_API */

#ifdef __cplusplus
}
#endif

#endif /* WSTR_H */
