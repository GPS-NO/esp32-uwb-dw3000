#include "SPI.h"
#include "dw3000.h"

#pragma once
#ifndef ranging_h
#define ranging_h

/* Inter-ranging delay period, in milliseconds. */
#define RNG_DELAY_MS 500

/* Default antenna delay values for 64 MHz PRF. See NOTE 1 below. */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

/* Length of the common part of the message (up to and including the function code, see NOTE 2 below). */
#define ALL_MSG_COMMON_LEN 10

/* Indexes to access some of the fields in the frames defined above. */
#define ALL_MSG_SN_IDX 2
#define FINAL_MSG_POLL_TX_TS_IDX 10
#define FINAL_MSG_RESP_RX_TS_IDX 14
#define FINAL_MSG_FINAL_TX_TS_IDX 18

/* Delay between frames, in UWB microseconds. See NOTE 4 below. */
/* This is the delay from the end of the frame transmission to the enable of
 * the receiver, as programmed for the DW IC's wait for response feature. */
#define POLL_TX_TO_RESP_RX_DLY_UUS 700
/* This is the delay from Frame RX timestamp to TX reply timestamp used for
 * calculating/setting the DW IC's delayed TX function. This includes the
 * frame length of approximately 190 us with above configuration. */
#define RESP_RX_TO_FINAL_TX_DLY_UUS 700
/* Receive response timeout. See NOTE 5 below. */
#define RESP_RX_TIMEOUT_UUS 300

/* Delay between frames, in UWB microseconds. See NOTE 4 below. */
/* This is the delay from Frame RX timestamp to TX reply timestamp used for
 * calculating/setting the DW IC's delayed TX function. This includes the
 * frame length of approximately 190 us with above configuration. */
#define POLL_RX_TO_RESP_TX_DLY_UUS 900
/* This is the delay from the end of the frame transmission to the enable of
 * the receiver, as programmed for the DW IC's wait for response feature. */
#define RESP_TX_TO_FINAL_RX_DLY_UUS 500
/* Receive final timeout. See NOTE 5 below. */
#define FINAL_RX_TIMEOUT_UUS 220

/* Preamble timeout, in multiple of PAC size. See NOTE 7 below. */
#define PRE_TIMEOUT 5

/* Inter-ranging delay period, in milliseconds. */
#define RNG_DELAY_MS 500

/* Default antenna delay values for 64 MHz PRF. See NOTE 1 below. */
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

#define RX_BUF_LEN 32

class RangingSystem {
private:
  static RangingSystem *instance;

  RangingSystem();
  ~RangingSystem();

  uint32_t chipId = 0;

  uint8_t initator_poll_msg[12];
  uint8_t responder_msg[15];
  uint8_t initator_final_msg[24];

  uint8_t myID[4];
  uint8_t otherID[4];

  uint8_t frame_seq_nb;
  uint8_t rx_buffer[RX_BUF_LEN];
  uint32_t status_reg;

  uint64_t poll_ts;
  uint64_t resp_ts;
  uint64_t final_ts;

  double tof;
  double distance;

  unsigned long timeout_started;

  dwt_config_t config;

  void printHex(uint8_t num);

public:
  static RangingSystem *getInstance();
  int8_t init(uint8_t mID[4], int irq, int rst, int ss);
  int16_t initiateRanging(uint8_t oID[4], uint32_t timeout = 10000);
  float respondToRanging(uint8_t oID[4], uint32_t timeout = 10000);
  void reset();
  static void destroy();
};
#endif