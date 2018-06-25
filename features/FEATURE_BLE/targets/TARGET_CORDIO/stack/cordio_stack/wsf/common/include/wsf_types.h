/*************************************************************************************************/
/*!
 *  \file   wsf_types.h
 *
 *  \brief  Platform-independent data types.
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
#ifndef WSF_TYPES_H
#define WSF_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*! \addtogroup WSF_TYPES
 *  \{ */

/**************************************************************************************************
  Macros
**************************************************************************************************/

#ifndef NULL
/*! \brief NULL reference */
#define NULL  0
#endif

#ifndef TRUE
/*! \brief Boolean True */
#define TRUE  1
#endif

#ifndef FALSE
/*! \brief Boolean False */
#define FALSE 0
#endif

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/** \name Integer Data Types
 *
 */
/**@{*/
#if ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) && \
     (!defined(__ICC8051__) || (__ICC8051__ == 0))) || \
     defined(__CC_ARM) || defined(__IAR_SYSTEMS_ICC__)

#include <stdint.h>
#else
/*! \brief Signed 8-bit value. */
typedef signed char int8_t;
/*! \brief Unsigned 8-bit value. */
typedef unsigned char uint8_t;
/*! \brief Signed 16-bit value. */
typedef signed short int16_t;
/*! \brief Unsigned 16-bit value. */
typedef unsigned short uint16_t;
/*! \brief Signed 32-bit value. */
typedef signed long int32_t;
/*! \brief Unsigned 32-bit value. */
typedef unsigned long uint32_t;
/*! \brief Unsigned 64-bit value. */
typedef unsigned long long uint64_t;
#endif
/**@}*/

/*! \brief Boolean data type */
typedef uint8_t bool_t;

/*! \} */    /* WSF_TYPES */

#ifdef __cplusplus
};
#endif

#endif /* WSF_TYPES_H */
