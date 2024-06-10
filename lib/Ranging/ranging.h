#include "SPI.h"
#include "dw3000.h"
#include "utils.h"

#pragma once
#ifndef ranging_h
#define ranging_h

/* Default antenna delay values for 64 MHz PRF. See NOTE 1 below. */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

#define TX_TO_RX_DLY_UUS 100
#define RX_TO_TX_DLY_UUS 800

#define RX_BUF_LEN 32

//Martin
#define PIN_RST 27
#define PIN_IRQ 34
#define PIN_SS 4

#define MSG_LEN 16         /* message length */
#define BUF_LEN MSG_LEN    /* buffer length */
#define MSG_SN_IDX 2       /* sequence number */
#define MSG_FUNC_IDX 9     /* func code*/
#define MSG_T_REPLY_IDX 10 /* byte index of transmitter ts */

#define FUNC_CODE_POLL 0xE2
#define FUNC_CODE_ACK 0xE3
#define FUNC_CODE_RANGE 0xE4
#define FUNC_CODE_FINAL 0xE5
#define FUNC_CODE_RESET 0xE6

/* */
#define DWT_INIT_SUCCESS (0)
#define DWT_INIT_IDLE_ERROR (2)
#define DWT_INIT_ERROR (3)
#define DWT_INIT_CONFIG_ERROR (4)
#define DWT_INIT_STATION_MODE_ERROR (5)

enum StationMode {
  STATION_MODE_UNSET = 0,
  STATION_MODE_INITIATOR = 1,
  STATION_MODE_RESPONDER = 2
};

class RangingSystem {
private:
  static RangingSystem *instance;
  uint8_t myID[4];

  uint8_t frame_seq_nb;
  uint8_t rx_buffer[RX_BUF_LEN];
  uint32_t status_reg;

  double tof;
  double distance;

  unsigned long timeout_started;
  unsigned long subtimeout_started;

  dwt_config_t config;

  RangingSystem();
  ~RangingSystem();

  void printHex(uint8_t num);

  // Martin
  uint8_t stationMode;

  uint8_t initiatorId[4];
  uint8_t responderId[4];
  void setMessageId(uint8_t *msg, uint8_t *stationId);

  // Initiator related
  dwt_txconfig_t txconfig_options;
  SPISettings _fastSPI;
  bool wait_poll, wait_ack, wait_range, wait_final;
  uint64_t range_tx_ts;
  uint64_t poll_tx_ts;
  uint64_t tx_ts;
  uint32_t tx_time;
  uint64_t t_reply_2;
  uint8_t tx_msg[MSG_LEN], rx_msg[MSG_LEN];
  uint32_t t_reply_1;
  uint64_t t_round_1;
  uint32_t t_round_2;
  uint64_t range_rx_ts;
  uint64_t poll_rx_ts;
  uint64_t ack_tx_ts;

  bool isValidMsg(uint8_t *msg, uint8_t *id);

  // Responder related
  dwt_txconfig_t txconfig_options_rx;

  void reset(uint8_t *array, size_t size);

public:
  static RangingSystem *getInstance();
  float initiateRanging(uint32_t timeout = 10000);
  int16_t respondToRanging(uint32_t timeout = 10000);
  void reset();
  static void destroy();

  //Martin
  uint8_t init(StationMode mode, uint8_t mID[4]);
  const char *getStationModeChar() const;

  inline StationMode getStationMode() const {
    return static_cast<StationMode>(stationMode);
  }

  inline void setStationMode(StationMode mode) {
    stationMode = mode;
  }

  inline uint8_t *getInitiatorId() {
    return initiatorId;
  }

  inline void setInitiatorId(uint8_t id[4]) {
    for (int i = 0; i < 4; i++) {
      initiatorId[i] = id[i];
    }
  }

  inline uint8_t *getResponderId() {
    return responderId;
  }

  inline void setResponderId(uint8_t id[4]) {
    for (int i = 0; i < 4; i++) {
      responderId[i] = id[i];
    }
  }

  bool isInitiatorIdSet() const {
    return isArrayZeroed(initiatorId);
  }

  bool isResponderIdSet() const {
    return isArrayZeroed(responderId);
  }
};

#endif