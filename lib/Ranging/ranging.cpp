#include "ranging.h"

/* Values for the PG_DELAY and TX_POWER registers reflect the bandwidth and power
 * of the spectrum at the current temperature.
 * These values can be calibrated prior to taking reference measurements.
 * See NOTE 8 below. */
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

  stationMode = STATION_MODE_UNSET;
  reset(initiatorId, sizeof(initiatorId));
  reset(responderId, sizeof(responderId));

  uint8_t init_msg[MSG_LEN] = { 0x41, 0x88, 0, 0xCA, 0xDE, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0 };

  std::copy(std::begin(init_msg), std::end(init_msg), std::begin(tx_msg));
  std::copy(std::begin(init_msg), std::end(init_msg), std::begin(rx_msg));

  this->reset();
}


RangingSystem::~RangingSystem() {
  destroy();
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

void RangingSystem::setMessageId(uint8_t *msg, uint8_t *stationId) {
  msg[5] = stationId[3];
  msg[6] = stationId[2];
  msg[7] = stationId[1];
  msg[8] = stationId[0];
}

bool RangingSystem::isValidMsg(uint8_t *msg, uint8_t *id) {
  uint8_t msgIndex = 5;

  for (int i = 3; i >= 0; i--) {
    if (msg[msgIndex] != id[i]) return false;
    msgIndex++;
  }

  return true;
}

uint8_t RangingSystem::init(StationMode mode, uint8_t stationId[4]) {
  if (mode != STATION_MODE_UNSET && mode != STATION_MODE_INITIATOR && mode != STATION_MODE_RESPONDER) {
    return DWT_INIT_STATION_MODE_ERROR;
  }

  std::copy(stationId, stationId + 4, myID);

  // fixme: ist die Reihenfolge richtig?
  setMessageId(tx_msg, stationId);
  setMessageId(rx_msg, stationId);

  tx_msg[5] = stationId[3];
  tx_msg[6] = stationId[2];
  tx_msg[7] = stationId[1];
  tx_msg[8] = stationId[0];

  rx_msg[5] = stationId[3];
  rx_msg[6] = stationId[2];
  rx_msg[7] = stationId[1];
  rx_msg[8] = stationId[0];

  stationMode = mode;

  dwt_reset();

  _fastSPI = SPISettings(16000000L, MSBFIRST, SPI_MODE0);
  spiBegin(PIN_IRQ, PIN_RST);
  spiSelect(PIN_SS);

  delay(2);  // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

  while (!dwt_checkidlerc()) {  // Need to make sure DW IC is in IDLE_RC before proceeding
    Serial.println("dwt_checkidlerc() failed");
    return 1;
  }

  if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR) {
    Serial.println("dwt_initialise() failed");
    return 2;
  }

  // Enabling LEDs here for debug so that for each TX the D1 LED will flash on DW3000 red eval-shield boards.
  dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

  if (dwt_configure(&config)) {  // if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device
    Serial.println("dwt_configure() failed");
    return 3;
  }


  dwt_configuretxrf(&txconfig_options);

  // FIXME: maybe change timeout dynamic on MQTT message?
  if (stationMode == STATION_MODE_INITIATOR)
    dwt_setrxtimeout(0);
  else if (stationMode == STATION_MODE_RESPONDER)
    dwt_setrxtimeout(0);

  /* Apply default antenna delay value. See NOTE 2 below. */
  dwt_setrxantennadelay(RX_ANT_DLY);
  dwt_settxantennadelay(TX_ANT_DLY);
  dwt_setrxaftertxdelay(TX_TO_RX_DLY_UUS);

  /* Next can enable TX/RX states output on GPIOs 5 and 6 to help debug, and also TX/RX LEDs
   * Note, in real low power applications the LEDs should not be used. */
  dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);

  Serial.printf("(RangingSystem): started with address %c%c%c%c", (char)stationId[0], (char)stationId[1], (char)stationId[2], (char)stationId[3]);
  Serial.println();

  return DWT_INIT_SUCCESS;
}

float RangingSystem::initiateRanging(uint32_t timeout) {
  this->reset();

  Serial.printf("(RangingSystem): initiate raning to %c%c%c%c with timeout %u", (char)responderId[0], (char)responderId[1], (char)responderId[2], (char)responderId[3], timeout);
  Serial.println();

  this->timeout_started = millis();
  while (1) {
    if (timeout > 0 && millis() - this->timeout_started > timeout) return -2.0f;
    if (!wait_ack && !wait_final) {
      // Broadcast zum Start einer Messung
      Serial.println("(RangingSystem): DBG Broadcast start ranging");
      wait_ack = true;
      tx_msg[MSG_SN_IDX] = frame_seq_nb;
      tx_msg[MSG_FUNC_IDX] = FUNC_CODE_POLL;
      for (size_t i = 0; i < 5; i++) {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
        dwt_writetxdata((uint16_t)(MSG_LEN), tx_msg, 0);
        dwt_writetxfctrl((uint16_t)(MSG_LEN), 0, 1);
        dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
      }
    } else {
      dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
      dwt_rxenable(DWT_START_RX_IMMEDIATE);
    }

    this->subtimeout_started = millis();
    int remainingTimeout = timeout - (this->subtimeout_started - this->timeout_started);
    // --
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR))) {
      if (remainingTimeout <= 0 || millis() - this->subtimeout_started > remainingTimeout) return -3.0f;
    }

    /* receive ack msg or final msg */
    if (status_reg & SYS_STATUS_RXFCG_BIT_MASK) {
      dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
      dwt_readrxdata(rx_buffer, BUF_LEN, 0);
      // Wenn die Antwort des Broadcast nicht die ResponderID ist, versuche es erneut.
      if (!isValidMsg(rx_buffer, responderId)) {
        Serial.println("(RangingSystem): DBG !isValidMsg");
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        wait_ack = false;
        wait_final = false;
        continue;
      }

      //Wird ausgelöst, wenn die Nachricht aus dem rx_buffer vom richtig Responder ist.
      if (wait_ack) {
        poll_tx_ts = get_tx_timestamp_u64();
        t_round_1 = get_rx_timestamp_u64() - poll_tx_ts;
        resp_msg_get_ts(&rx_buffer[MSG_T_REPLY_IDX], &t_reply_1);
      } else {
        resp_msg_get_ts(&rx_buffer[MSG_T_REPLY_IDX], &t_round_2);
      }
    } else { /* timeout or error, reset, send ack*/
      Serial.println("(RangingSystem): DBG timeout or error, reset, send ack to responders");
      tx_msg[MSG_SN_IDX] = frame_seq_nb;
      tx_msg[MSG_FUNC_IDX] = FUNC_CODE_RESET;
      for (size_t i = 0; i < 5; i++) {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
        dwt_writetxdata((uint16_t)(MSG_LEN), tx_msg, 0);
        dwt_writetxfctrl((uint16_t)(MSG_LEN), 0, 1);
        dwt_starttx(DWT_START_TX_IMMEDIATE);
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
      }

      wait_ack = false;
      wait_final = false;
      delay(1);
      return -1.0f;
    }
    if (wait_ack) {
      /* received all ack msg send range msg*/
      Serial.println("(RangingSystem): DBG send range msg");
      tx_time = (get_rx_timestamp_u64() + (RX_TO_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
      tx_ts = (((uint64_t)(tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;
      dwt_setdelayedtrxtime(tx_time);
      tx_msg[MSG_SN_IDX] = frame_seq_nb;
      tx_msg[MSG_FUNC_IDX] = FUNC_CODE_RANGE;

      for (size_t i = 0; i < 5; i++) {
        dwt_writetxdata((uint16_t)(BUF_LEN), tx_msg, 0);
        dwt_writetxfctrl((uint16_t)(BUF_LEN), 0, 1);
        dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
      }

      wait_ack = false;
      wait_final = true;
      continue;
    }
    if (wait_final) { /* received all final msg */
      Serial.println("(RangingSystem): DBG received all final msg");
      range_tx_ts = get_tx_timestamp_u64();

      t_reply_2 = range_tx_ts - (t_round_1 + poll_tx_ts);
      tof = (t_round_1 * t_round_2 - t_reply_1 * t_reply_2) / (t_round_1 + t_round_2 + t_reply_1 + t_reply_2) * DWT_TIME_UNITS;
      distance = tof * SPEED_OF_LIGHT;
      snprintf(dist_str, sizeof(dist_str), "%3.3f m\t", distance);
      Serial.print("\t");
      Serial.print(dist_str);

      Serial.println();
      wait_ack = false;
      wait_final = false;
      frame_seq_nb++;
      return distance;
    }
  }
  return 0.0;
}

int16_t RangingSystem::respondToRanging(uint32_t timeout) {
  this->reset();

  Serial.printf("(RangingSystem): start responding to %c%c%c%c with timeout %u", (char)initiatorId[0], (char)initiatorId[1], (char)initiatorId[2], (char)initiatorId[3], timeout);
  Serial.println();

  this->timeout_started = millis();
  while (1) {
    if (timeout > 0 && millis() - this->timeout_started > timeout) return -2;
    dwt_rxenable(DWT_START_RX_IMMEDIATE);

    this->subtimeout_started = millis();
    int remainingTimeout = timeout - (this->subtimeout_started - this->timeout_started);
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR))) {
      if (remainingTimeout <= 0 || millis() - this->subtimeout_started > remainingTimeout) return -3;
    }

    if (status_reg & SYS_STATUS_RXFCG_BIT_MASK) { /* receive msg */
      dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);
      dwt_readrxdata(rx_buffer, BUF_LEN, 0);
      // Wenn timeout vom Initiator erkannt wurde brodcasted er ein timeout zum resetten
      if (rx_buffer[MSG_FUNC_IDX] == FUNC_CODE_RESET) {
        Serial.println("(RangingSystem): received RESET function from initiator");
        wait_poll = true;
        wait_range = false;
        return -2;
      }
      if (!isValidMsg(rx_buffer, initiatorId)) {
        Serial.println("(RangingSystem): DBG !isValidMsg");
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        wait_poll = true;
        wait_range = false;
        continue;
      }
      if (wait_poll)  // received poll from U1
        poll_rx_ts = get_rx_timestamp_u64();
      else if (wait_range)
        range_rx_ts = get_rx_timestamp_u64();
    } else {
      wait_poll = true;
      wait_range = false;
      dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR | SYS_STATUS_ALL_RX_TO);
      continue;
    }
    if (wait_poll) {
      Serial.println("(RangingSystem): DBG send wait_poll");
      tx_time = (get_rx_timestamp_u64() + (RX_TO_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
      tx_ts = (((uint64_t)(tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;
      dwt_setdelayedtrxtime(tx_time);
      tx_msg[MSG_SN_IDX] = frame_seq_nb;
      tx_msg[MSG_FUNC_IDX] = FUNC_CODE_ACK;
      resp_msg_set_ts(&tx_msg[MSG_T_REPLY_IDX], tx_ts - poll_rx_ts);

      for (size_t i = 0; i < 5; i++) {
        dwt_writetxdata((uint16_t)(BUF_LEN), tx_msg, 0);
        dwt_writetxfctrl((uint16_t)(BUF_LEN), 0, 1);
        dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
      }

      /* all anchors sent the ack msgs */
      wait_poll = false;
      wait_range = true;
      continue;
    }
    if (wait_range) {
      /* send final msg */
      Serial.println("(RangingSystem): DBG send final msg");
      ack_tx_ts = get_tx_timestamp_u64(); /* ack tx */
      tx_time = (get_rx_timestamp_u64() + (RX_TO_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
      tx_ts = (((uint64_t)(tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;
      dwt_setdelayedtrxtime(tx_time);
      resp_msg_set_ts(&tx_msg[MSG_T_REPLY_IDX], range_rx_ts - ack_tx_ts);
      tx_msg[MSG_SN_IDX] = frame_seq_nb;
      tx_msg[MSG_FUNC_IDX] = FUNC_CODE_FINAL;

      for (size_t i = 0; i < 5; i++) {
        dwt_writetxdata((uint16_t)(BUF_LEN), tx_msg, 0);
        dwt_writetxfctrl((uint16_t)(BUF_LEN), 0, 1);
        dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
      }

      /* all anchors sent the final msgs */
      wait_poll = true;
      wait_range = false;
      frame_seq_nb++;
      return 0;
    }
  }

  return -1;
}

void RangingSystem::reset() {

  wait_poll = true;
  wait_ack = false;
  wait_range = false;
  wait_final = false;

  /* Frame sequence number, incremented after each transmission. */
  frame_seq_nb = 0;

  /* Buffer to store received response message.
     * Its size is adjusted to longest frame that this example code
     * is supposed to handle. */
  memset(rx_buffer, 0, sizeof(rx_buffer));

  /* Hold copy of status register state here for reference so that it can be
     * examined at a debug breakpoint. */
  status_reg = 0;

  /* Hold copies of computed time of flight and distance here for reference
     * so that it can be examined at a debug breakpoint. */
  tof = 0.0;
  distance = 0.0;
}

void RangingSystem::reset(uint8_t *array, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    array[i] = 0;
  }
}

const char *RangingSystem::getStationModeChar() const {
  switch (stationMode) {
    case STATION_MODE_UNSET:
      return "UNSET";
    case STATION_MODE_INITIATOR:
      return "INITIATOR";
    case STATION_MODE_RESPONDER:
      return "RESPONDER";
    default:
      return "UNKNOWN";
  }
}