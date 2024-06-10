#include "ranging.h"

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power
 * of the spectrum at the current temperature.
 * These values can be calibrated prior to taking reference measurements.
 * See NOTE 8 below. */
extern dwt_txconfig_t txconfig_options;

RangingSystem *RangingSystem::instance = nullptr;

RangingSystem *RangingSystem::getInstance() {
  if (instance == NULL) {
    instance = new RangingSystem();
  }

  return instance;
}

RangingSystem::RangingSystem() {
  /* Default communication configuration. We use default non-STS DW mode. */
  config = {
    5,                /* Channel number. */
    DWT_PLEN_128,     /* Preamble length. Used in TX only. */
    DWT_PAC8,         /* Preamble acquisition chunk size. Used in RX only. */
    9,                /* TX preamble code. Used in TX only. */
    9,                /* RX preamble code. Used in RX only. */
    1,                /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_6M8,       /* Data rate. */
    DWT_PHRMODE_STD,  /* PHY header mode. */
    DWT_PHRRATE_STD,  /* PHY header rate. */
    (129 + 8 - 8),    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF, /* STS disabled */
    DWT_STS_LEN_64,   /* STS length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0       /* PDOA mode off */
  };

  this->reset();
}

RangingSystem::~RangingSystem() {
  destroy();
}

void RangingSystem::setMessageId(uint8_t *msg, uint8_t *stationId) {
  msg[5] = stationId[0];
  msg[6] = stationId[1];
  msg[7] = stationId[2];
  msg[8] = stationId[3];
}

void RangingSystem::destroy() {
  if (instance != nullptr) {
    delete instance;
    instance = nullptr;
  }
}

void RangingSystem::printHex(uint8_t num) {
  char hexCar[2];
  sprintf(hexCar, "%02X", num);
  Serial.print(hexCar);
}

int8_t RangingSystem::init(int irq, int rst, int ss) {
  spiBegin(irq, rst);
  spiSelect(ss);

  dwt_reset();

  /* Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC */
  delay(5);

  /* Need to make sure DW IC is in IDLE_RC before proceeding */
  while (!dwt_checkidlerc()) {
    Serial.println("ranging_init: IDLE FAILED");
    return 2;
  }

  if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR) {
    Serial.println("ranging_init: INIT FAILED");
    return 3;
  }

  /* Configure DW IC. See NOTE 2 below. */
  /* if the dwt_configure returns DWT_ERROR either the PLL or RX calibration
     * has failed the host should reset the device */
  if (dwt_configure(&config)) {
    Serial.println("ranging_init: CONFIG FAILED");
    return 4;
  }

  /* Configure the TX spectrum parameters (power, PG delay and PG count) */
  dwt_configuretxrf(&txconfig_options);

  /* Apply default antenna delay value. See NOTE 1 below. */
  dwt_setrxantennadelay(RX_ANT_DLY);
  dwt_settxantennadelay(TX_ANT_DLY);

  /* Next can enable TX/RX states output on GPIOs 5 and 6 to help debug,
     * and also TX/RX LEDs.
     * Note, in real low power applications the LEDs should not be used. */
  dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);

  dwt_setleds(DWT_LEDS_DISABLE);

  Serial.printf("(RangingSystem): started");
  Serial.println();
  return 0;
}

int16_t RangingSystem::initiateRanging(uint32_t timeout) {
  this->reset();

  setMessageId(this->initator_poll_msg, this->initiatorId);
  setMessageId(this->initator_final_msg, this->initiatorId);
  setMessageId(this->responder_msg, this->responderId);

  Serial.printf("(RangingSystem): initiate ranging to %c%c%c%c with timeout %u\r\n", (char)this->responderId[0], (char)this->responderId[1], (char)this->responderId[2], (char)this->responderId[3], timeout);

  /* Set expected response's delay and timeout. See NOTE 4, 5 and 7 below.
     * As this example only handles one incoming frame with always the same
     * delay and timeout, those values can be set here once for all. */
  dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
  dwt_setrxtimeout(0);
  dwt_setpreambledetecttimeout(PRE_TIMEOUT);

  /* Write frame data to DW IC and prepare transmission. See NOTE 9 below. */
  initator_poll_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
  dwt_writetxdata(sizeof(initator_poll_msg) - 2, initator_poll_msg, 0); /* Zero offset in TX buffer. */
  dwt_writetxfctrl(sizeof(initator_poll_msg) - 2 + FCS_LEN, 0, 1);      /* Zero offset in TX buffer, ranging. */

  /* Start transmission, indicating that a response is expected so that
     * reception is enabled automatically after the frame is sent and the
     * delay set by dwt_setrxaftertxdelay() has elapsed. */
  dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

  /* We assume that the transmission is achieved correctly, poll for reception of a frame or error/timeout. See NOTE 10 below. */
  this->timeout_started = millis();
  while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR))) {
    if (timeout > 0 && millis() - this->timeout_started > timeout) return -3;
  }

  /* Increment frame sequence number after transmission of the poll message (modulo 256). */
  frame_seq_nb++;

  if (status_reg & SYS_STATUS_RXFCG_BIT_MASK) {
    /* Clear good RX frame event and TX frame sent in the DW IC status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_TXFRS_BIT_MASK);

    /* A frame has been received, read it into the local buffer. */
    uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & FRAME_LEN_MAX_EX;
    if (frame_len <= RX_BUF_LEN) dwt_readrxdata(rx_buffer, frame_len, 0);

    /* Check that the frame is the expected response from the
         * companion "DS TWR responder" example.
         * As the sequence number field of the frame is not relevant,
         * it is cleared to simplify the validation of the frame. */
    rx_buffer[ALL_MSG_SN_IDX] = 0;
    if (memcmp(rx_buffer, responder_msg, ALL_MSG_COMMON_LEN) == 0) {
      uint32_t final_tx_time;

      /* Retrieve poll transmission and response reception timestamp. */
      poll_ts = get_tx_timestamp_u64();
      resp_ts = get_rx_timestamp_u64();

      /* Compute final message transmission time. See NOTE 11 below. */
      final_tx_time = (resp_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
      dwt_setdelayedtrxtime(final_tx_time);

      /* Final TX timestamp is the transmission time we programmed plus the TX antenna delay. */
      final_ts = (((uint64_t)(final_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

      /* Write all timestamps in the final message. See NOTE 12 below. */
      final_msg_set_ts(&initator_final_msg[FINAL_MSG_POLL_TX_TS_IDX], poll_ts);
      final_msg_set_ts(&initator_final_msg[FINAL_MSG_RESP_RX_TS_IDX], resp_ts);
      final_msg_set_ts(&initator_final_msg[FINAL_MSG_FINAL_TX_TS_IDX], final_ts);

      /* Write and send final message. See NOTE 9 below. */
      initator_final_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
      for (size_t i = 0; i < 5; i++) {
        dwt_writetxdata(sizeof(initator_final_msg) - 2, initator_final_msg, 0); /* Zero offset in TX buffer. */
        dwt_writetxfctrl(sizeof(initator_final_msg) - 2 + FCS_LEN, 0, 1);       /* Zero offset in TX buffer, ranging bit set. */

        /* If dwt_starttx() returns an error, abandon this ranging exchange and proceed to the next one. See NOTE 13 below. */
        if (dwt_starttx(DWT_START_TX_DELAYED) == DWT_SUCCESS) {
          /* Poll DW IC until TX frame sent event set. See NOTE 10 below. */
          this->timeout_started = millis();
          while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS_BIT_MASK)) {
            if (timeout > 0 && millis() - this->timeout_started > timeout) return -3;
          }

          /* Clear TXFRS event. */
          dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
        }
      }

      /* Increment frame sequence number after transmission of the final message (modulo 256). */
      frame_seq_nb++;
      return 1;
    }
  } else
    /* Clear RX error/timeout events in the DW IC status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR | SYS_STATUS_TXFRS_BIT_MASK);

  /* Execute a delay between ranging exchanges. */
  delay(RNG_DELAY_MS);
  return 0;
}

float RangingSystem::respondToRanging(uint32_t timeout) {
  this->reset();

  Serial.printf("(RangingSystem): start responding to %c%c%c%c with timeout %u\r\n", (char)this->initiatorId[0], (char)this->initiatorId[1], (char)this->initiatorId[2], (char)this->initiatorId[3], timeout);

  setMessageId(this->initator_poll_msg, this->initiatorId);
  setMessageId(this->initator_final_msg, this->initiatorId);
  setMessageId(this->responder_msg, this->responderId);

  dwt_setpreambledetecttimeout(0);
  /* Clear reception timeout to start next ranging process. */
  dwt_setrxtimeout(0);

  /* Activate reception immediately. */
  dwt_rxenable(DWT_START_RX_IMMEDIATE);

  /* Poll for reception of a frame or error/timeout. See NOTE 8 below. */
  this->timeout_started = millis();
  while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR))) {
    if (timeout > 0 && millis() - this->timeout_started > timeout) return -2;
  }

  if (status_reg & SYS_STATUS_RXFCG_BIT_MASK) {
    /* Clear good RX frame event in the DW IC status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

    /* A frame has been received, read it into the local buffer. */
    uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & FRAME_LEN_MAX_EX;
    if (frame_len <= RX_BUF_LEN) dwt_readrxdata(rx_buffer, frame_len, 0);

    /* Check that the frame is a poll sent by "DS TWR initiator" example.
         * As the sequence number field of the frame is not relevant, it
         * is cleared to simplify the validation of the frame.
         */
    rx_buffer[ALL_MSG_SN_IDX] = 0;
    if (memcmp(rx_buffer, initator_poll_msg, ALL_MSG_COMMON_LEN) == 0) {
      uint32_t resp_tx_time;

      /* Retrieve poll reception timestamp. */
      poll_ts = get_rx_timestamp_u64();

      /* Set send time for response. See NOTE 9 below. */
      resp_tx_time = (poll_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
      dwt_setdelayedtrxtime(resp_tx_time);

      /* Set expected delay and timeout for final message reception. See NOTE 4 and 5 below. */
      dwt_setrxaftertxdelay(RESP_TX_TO_FINAL_RX_DLY_UUS);
      dwt_setrxtimeout(0);

      /* Set preamble timeout for expected frames. See NOTE 6 below. */
      dwt_setpreambledetecttimeout(PRE_TIMEOUT);

      /* Write and send the response message. See NOTE 10 below.*/
      responder_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
      dwt_writetxdata(sizeof(responder_msg), responder_msg, 0); /* Zero offset in TX buffer. */
      dwt_writetxfctrl(sizeof(responder_msg), 0, 1);            /* Zero offset in TX buffer, ranging. */
      int ret = dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);

      /* If dwt_starttx() returns an error, abandon this ranging
             * exchange and proceed to the next one. See NOTE 11 below. */
      if (ret == DWT_ERROR) return -1;

      /* Poll for reception of expected "final" frame or error/timeout.
             * See NOTE 8 below.
             */
      this->timeout_started = millis();
      while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR))) {
        if (timeout > 0 && millis() - this->timeout_started > timeout) return -3;
      }

      /* Increment frame sequence number after transmission of the
             * response message (modulo 256).
             */
      frame_seq_nb++;

      if (status_reg & SYS_STATUS_RXFCG_BIT_MASK) {
        /* Clear good RX frame event and TX frame sent in the DW IC status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_TXFRS_BIT_MASK);

        /* A frame has been received, read it into the local buffer. */
        frame_len = dwt_read32bitreg(RX_FINFO_ID) & FRAME_LEN_MAX_EX;
        if (frame_len <= RX_BUF_LEN) dwt_readrxdata(rx_buffer, frame_len, 0);

        /* Check that the frame is a final message sent by
                 * "DS TWR initiator" example.
                 * As the sequence number field of the frame is not used in
                 * this example, it can be zeroed to ease the validation of
                 * the frame.
                 */
        rx_buffer[ALL_MSG_SN_IDX] = 0;
        if (memcmp(rx_buffer, initator_final_msg, ALL_MSG_COMMON_LEN) == 0) {
          uint32_t poll_tx_ts, resp_rx_ts, final_tx_ts;
          uint32_t poll_rx_ts_32, resp_tx_ts_32, final_rx_ts_32;
          double Ra, Rb, Da, Db;
          int64_t tof_dtu;

          /* Retrieve response transmission and final
                     * reception timestamps. */
          resp_ts = get_tx_timestamp_u64();
          final_ts = get_rx_timestamp_u64();

          /* Get timestamps embedded in the final message. */
          final_msg_get_ts(&rx_buffer[FINAL_MSG_POLL_TX_TS_IDX], &poll_tx_ts);
          final_msg_get_ts(&rx_buffer[FINAL_MSG_RESP_RX_TS_IDX], &resp_rx_ts);
          final_msg_get_ts(&rx_buffer[FINAL_MSG_FINAL_TX_TS_IDX], &final_tx_ts);

          /* Compute time of flight. 32-bit subtractions give
                     * correct answers even if clock has wrapped.
                     * See NOTE 12 below. */
          poll_rx_ts_32 = (uint32_t)poll_ts;
          resp_tx_ts_32 = (uint32_t)resp_ts;
          final_rx_ts_32 = (uint32_t)final_ts;
          Ra = (double)(resp_rx_ts - poll_tx_ts);
          Rb = (double)(final_rx_ts_32 - resp_tx_ts_32);
          Da = (double)(final_tx_ts - resp_rx_ts);
          Db = (double)(resp_tx_ts_32 - poll_rx_ts_32);
          tof_dtu = (int64_t)((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db));

          tof = tof_dtu * DWT_TIME_UNITS;
          distance = tof * SPEED_OF_LIGHT;

          /* Display computed distance. */
          Serial.println(String(distance * 100) + "cm");

          /* As DS-TWR initiator is waiting for RNG_DELAY_MS
                     * before next poll transmission we can add a delay
                     * here before RX is re-enabled again.
                     */
          // delay(RNG_DELAY_MS - 10);  // start couple of ms earlier
          return distance * 100;
        }
      } else
        /* Clear RX error/timeout events in the DW IC
                 * status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
    }
  } else
    /* Clear RX error/timeout events in the DW IC status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);

  return 0;
}

void RangingSystem::reset() {
  /* Frames used in the ranging process. See NOTE 2 below. */
  uint8_t val_initator_poll_msg[] = { 0x41, 0x88, 0, 0xCA, 0xDE, '-', '-', '-', '-', 0x21, 0, 0 };
  uint8_t val_responder_msg[] = { 0x41, 0x88, 0, 0xCA, 0xDE, '-', '-', '-', '-', 0x10, 0x02, 0, 0, 0, 0 };
  uint8_t val_initator_final_msg[] = { 0x41, 0x88, 0, 0xCA, 0xDE, '-', '-', '-', '-', 0x23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  std::copy(val_initator_poll_msg, val_initator_poll_msg + sizeof(val_initator_poll_msg) / sizeof(val_initator_poll_msg[0]), initator_poll_msg);
  std::copy(val_responder_msg, val_responder_msg + sizeof(val_responder_msg) / sizeof(val_responder_msg[0]), responder_msg);
  std::copy(val_initator_final_msg, val_initator_final_msg + sizeof(val_initator_final_msg) / sizeof(val_initator_final_msg[0]), initator_final_msg);

  /* Frame sequence number, incremented after each transmission. */
  frame_seq_nb = 0;

  /* Buffer to store received response message.
     * Its size is adjusted to longest frame that this example code
     * is supposed to handle. */
  memset(rx_buffer, 0, sizeof(rx_buffer));

  /* Hold copy of status register state here for reference so that it can be
     * examined at a debug breakpoint. */
  status_reg = 0;

  /* Time-stamps of frames transmission/reception, expressed in device time units. */
  poll_ts = 0;
  resp_ts = 0;
  final_ts = 0;

  /* Hold copies of computed time of flight and distance here for reference
     * so that it can be examined at a debug breakpoint. */
  tof = 0.0;
  distance = 0.0;
}
