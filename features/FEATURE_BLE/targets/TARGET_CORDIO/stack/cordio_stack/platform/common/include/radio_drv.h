/*************************************************************************************************/
/*!
 *  \file
 *
 *  \brief      Radio driver interface file.
 *
 *  Copyright (c) 2016-2018 Arm Ltd. All Rights Reserved.
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

#ifndef RADIO_DRV_H
#define RADIO_DRV_H

#include "wsf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief      Operation types. */
enum
{
  RADIO_DRV_BLE_OP_TEST_TX,                     /*!< Continuous Tx test mode. */
  RADIO_DRV_BLE_OP_TEST_RX,                     /*!< Continuous Rx test mode. */
  RADIO_DRV_BLE_OP_MST_ADV_EVENT,               /*!< Master advertising event. */
  RADIO_DRV_BLE_OP_SLV_ADV_EVENT,               /*!< Slave advertising event. */
  RADIO_DRV_BLE_OP_MST_CONN_EVENT,              /*!< Master connection event. */
  RADIO_DRV_BLE_OP_SLV_CONN_EVENT,              /*!< Slave connection event. */
  RADIO_DRV_15P4_EVENT                          /*!< 15P4 event. */
};

/*! \brief      Radio timing. */
typedef struct
{
  int16_t txOnLatency;                         /*!< Latency between radio on signal and transmit. */
  int16_t rxOnLatency;                         /*!< Latency between radio on signal and receive. */
  int16_t txDataPathLatency;                   /*!< Transmit data path latency. */
  int16_t rxDataPathLatency;                   /*!< Receive data path latency. */
} RadioDrvTiming_t;

/*! \brief    Radio timing. */
typedef struct
{
  /*
   * Correction from protocol time, derived by observing difference between reported packet time
   * (end timestamp - start timestamp) and actual packet time.
   */
  int16_t endTxTimestampCorrection;      /*!< Correction to end Tx timestamp. */

  /* Latency through the demodulator for receives */
  int16_t rxDataPathLatency;                /*!< Receive datapath latency. */

  /* Latency through the modulator for transmits */
  int16_t txDataPathLatency;             /*!< Transmit datapath latency. */

  /* Latency through the modulator PHY for transmits */
  int16_t txPhyLatency;             /*!< Transmit PHY latency. */

  /*
   * Correction from protocol time, derived by observing difference between reported packet time
   * (end timestamp) and actual packet time.
   */
  int16_t endRxTimestampCorrection;      /*!< Correction to end Rx timestamp. */

  /* Latency for different phy to RX */
  int16_t rxCodedPhyLatency;             /*!< RX coded path latency. */

  /* Latency for coded phy to TX */
  int16_t txLatencyFromRx;          /*!< TX latency from RX. */
} RadioDrvPhyTiming_t;

/*! \brief      Abort callback. */
typedef void (*RadioDrvAbortCback_t)(void);

/*! \brief      Crystal failure callback. */
typedef void (*RadioDrvXtalFailCback_t)(int16_t);

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief      Handle radio configuration.
 *
 *  \param      len       Length of configuration data, in octets.
 *  \param      pCfg      Configuration data.
 *
 *  \return     TRUE if radio configuration was handled.
 *
 *  The data block pCfg is only valid during the execution of this function, so configuration
 *  data must be stored or copied.
 */
/*************************************************************************************************/
bool_t RadioDrvCfgHandler(uint16_t len, const uint8_t *pCfg);

/*************************************************************************************************/
/*!
 *  \brief      Initialize the BB radio.
 *
 *  \return     None.
 *
 *  Initialization occurs once upon startup of MAC-layer software to load trim, calibrate clocks,
 *  or perform any other one-time operations.
 */
/*************************************************************************************************/
void RadioDrvInit(void);

/*************************************************************************************************/
/*!
 *  \brief      Get timing parameters for radio.
 *
 *  \param      pTiming         Storage for timing parameters.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void RadioDrvGetTiming(RadioDrvTiming_t *pTiming);

/*************************************************************************************************/
/*!
 *  \brief      Get radio timing parameters.
 *
 *  \return     Radio timing parameters.
 */
/*************************************************************************************************/
const RadioDrvPhyTiming_t *RadioDrvGetRadioTiming(void);

/*************************************************************************************************/
/*!
 *  \brief      Get supported transmit power levels.
 *
 *  \param      pMinTxPwr       Storage for minimum transmit power (expressed in 1dBm units).
 *  \param      pMaxTxPwr       Storage for maximum transmit power (expressed in 1dBm units).
 *
 *  \return     None.
 */
/*************************************************************************************************/
void RadioDrvGetSupTxPower(int8_t *pMinTxPwr, int8_t *pMaxTxPwr);

/*************************************************************************************************/
/*!
 *  \brief      Get the actual Tx power at the antenna (expressed in 1dBm units).
 *
 *  \param      txPwr     Tx power provided by the host (expressed in 1dBm units).
 *
 *  \return     Actual Tx power at the antenna (expressed in 1dBm units).
 */
/*************************************************************************************************/
int8_t RadioDrvGetActualTxPower(int8_t txPwr);

/*************************************************************************************************/
/*!
 *  \brief      Get the radio version.
 *
 *  \param      pVerCode        Version code return value.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void RadioDrvGetVersion(uint32_t *pVerCode);

/*************************************************************************************************/
/*!
 *  \brief      Set abort callback.
 *
 *  \param      cback           Abort callback.
 *
 *  \return     None.
 *
 *  If the abort callback is called after RadioDrvStartTx() or RadioDrvStartRx() but before
 *  RadioDrvStop(), the BB will abort the current operation.  Otherwise, the function is ignored.
 *
 *  The BB will set the callback to NULL to clear the callback.
 */
/*************************************************************************************************/
void RadioDrvSetAbortCback(RadioDrvAbortCback_t cback);

/*************************************************************************************************/
/*!
 *  \brief      Set Crystal failure callback.
 *
 *  \param      cback           Crystal failure callback.
 *
 *  \return     None.
 *
 *  The platform_rtc.c will set this callback.
 */
/*************************************************************************************************/
void RadioDrvSetXtalFailCback(RadioDrvXtalFailCback_t cback);

/*************************************************************************************************/
/*!
 *  \brief      Enable the BB radio.
 *
 *  \return     None.
 *
 *  The radio should be enabled, possibly after leaving sleep.  The XTAL warmup must be started, but
 *  no radio operation will be attempted for xtalWarmup time, when the XTAL must be ready.
 */
/*************************************************************************************************/
void RadioDrvEnable(void);

/*************************************************************************************************/
/*!
 *  \brief      Disable the BB radio.
 *
 *  \return     None.
 *
 *  The radio should be disabled, possibly before entering sleep.  Any ongoing transmit or receive
 *  should be stopped.  The XTAL may be disabled.
 */
/*************************************************************************************************/
void RadioDrvDisable(void);

/*************************************************************************************************/
/*!
 *  \brief      Set RF Debug Mode.
 *
 *  \param      ddmSetting             DDM mode for different modules.
 *  \param      ddmDir                 DDM mode direction.
 *  \param      peripheralSetting      RF setting for debug mode.
 *  \param      peripheralDir          RX or TX.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void RadioDrvSetDdm(uint32_t ddmSetting, uint32_t ddmDir, uint32_t peripheralSetting, uint32_t peripheralDir);

/*************************************************************************************************/
/*!
 *  \brief      Wait until radio is in idle state.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void RadioDrvWaitForIdle(void);

/*************************************************************************************************/
/*!
 *  \brief      Set radio channel parameters.
 *
 *  \param      opType          Operation type.
 *  \param      rfFreq          RF frequency in MHz.
 *  \param      txPhy           Transmitter PHY.
 *  \param      rxPhy           Receiver PHY.
 *  \param      phyOptions      PHY options.
 *  \param      txPower         Transmit power in dBm.
 *
 *  \return     None.
 *
 *  The channel parameters remain active until new parameters are set, the radio is disabled, or a
 *  radio operation is stopped.
 */
/*************************************************************************************************/
void RadioDrvSetChannelParam(uint8_t opType, uint16_t rfFreq, uint8_t txPhy, uint8_t rxPhy, uint8_t phyOptions, int8_t txPower);

/*************************************************************************************************/
/*!
 *  \brief      Start transmitter.
 *
 *  \return     None.
 *
 *  Prepare the transmitter, so that the warmup will begin at the radio request, with the
 *  modulator producing the first bit after txOnLatency.  The transmitter should automatically stop
 *  when the transmit ends so that another transmit or a receive can be started.
 */
/*************************************************************************************************/
void RadioDrvStartTx(void);

/*************************************************************************************************/
/*!
 *  \brief      Start receiver.
 *
 *  \return     None.
 *
 *  Prepare the receiver, so that warmup will begin at the radio request, with the demodulator
 *  expecting the first bit after rxOnLatency.  The receiver should automatically stop when the
 *  receive ends so that another recieve or a transmit can be started.
 */
/*************************************************************************************************/
void RadioDrvStartRx(void);

/*************************************************************************************************/
/*!
 *  \brief      Start transmitter in continuous mode.
 *
 *  \return     None.
 *
 *  Start the transmitter immediately and stay on indefinitely.
 */
/*************************************************************************************************/
void RadioDrvStartContinuousTx(void);

/*************************************************************************************************/
/*!
 *  \brief      Start receiver in continuous mode.
 *
 *  \return     None.
 *
 *  Start the receiver immediately and stay on indefinitely.
 */
/*************************************************************************************************/
void RadioDrvStartContinuousRx(void);

/*************************************************************************************************/
/*!
 *  \brief      Stop transmitter or receiver.
 *
 *  \return     None.
 */
/*************************************************************************************************/
void RadioDrvStop(void);

/*************************************************************************************************/
/*!
 *  \brief      Radio driver set PMU and clk.
 *
 *  \return     True if success.
 */
/*************************************************************************************************/
bool_t RadioDrvSetPMUClk(void);

/*************************************************************************************************/
/*!
 *  \brief      Fill the buffer with random bytes
 *
 *  \param      pBufferRandom   Pointer to buffer
 *  \param      numRandomBytes  Number of bytes to write
 *
 *  \return     None.
 *
 *  \note       Because this function takes manual control of the radio it cannot be used when
 *              the radio is, or might become active. Typically this function will only be used
 *              during boot time to provide random numbers that are used for initialising other
 *              parts of the system.
 */
/*************************************************************************************************/
void RadioDrvGetRandomBytes(uint8_t *pBufferRandom, uint8_t numRandomBytes);

#ifdef __cplusplus
};
#endif

#endif /* RADIO_DRV_H */
