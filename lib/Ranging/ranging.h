#include "dw3000.h"

#define PIN_RST 27
#define PIN_IRQ 34
#define PIN_SS 4

#define RNG_DELAY_MS 1000
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385
#define ALL_MSG_COMMON_LEN 10
#define ALL_MSG_SN_IDX 2
#define RESP_MSG_POLL_RX_TS_IDX 10
#define RESP_MSG_RESP_TX_TS_IDX 14
#define RESP_MSG_TS_LEN 4
#define POLL_TX_TO_RESP_RX_DLY_UUS 240
#define RESP_RX_TIMEOUT_UUS 400

/* Default communication configuration. We use default non-STS DW mode. */
static dwt_config_t config = {
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

static uint8_t tx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0};
static uint8_t rx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t frame_seq_nb = 0;
static uint8_t rx_buffer[20];
static uint32_t status_reg = 0;
static double tof;
static double distance;
extern dwt_txconfig_t txconfig_options;

int8_t ranging_init() {
    spiBegin(PIN_IRQ, PIN_RST);
    spiSelect(PIN_SS);

    // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)
    delay(2);

    // IDLE FAILED
    // Need to make sure DW IC is in IDLE_RC before proceeding
    while (!dwt_checkidlerc()) return 1;

    // INIT FAILED
    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR) return 2;

    // Enabling LEDs here for debug so that for each TX the D1 LED will flash on DW3000 red eval-shield boards.
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

    // CONFIG FAILED
    // Configure DW IC. See NOTE 6 below.
    // if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device
    if (dwt_configure(&config)) return 3;

    // Configure the TX spectrum parameters (power, PG delay and PG count)
    dwt_configuretxrf(&txconfig_options);

    // Apply default antenna delay value. See NOTE 2 below.
    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);

    // Set expected response's delay and timeout. See NOTE 1 and 5 below.
    // As this example only handles one incoming frame with always the same delay and timeout, those values can be set here once for all.
    dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
    dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);

    // Next can enable TX/RX states output on GPIOs 5 and 6 to help debug, and also TX/RX LEDs
    // Note, in real low power applications the LEDs should not be used.
    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);

    return true;
}