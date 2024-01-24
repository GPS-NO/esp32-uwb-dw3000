## NOTES:

1. The sum of the values is the TX to RX antenna delay, experimentally determined by a calibration process. Here we use a hard coded typical value
   but, in a real application, each device should have its own antenna delay properly calibrated to get the best possible precision when performing
   range measurements.
2. The messages here are similar to those used in the DecaRanging ARM application (shipped with EVK1000 kit). They comply with the IEEE
   802.15.4 standard MAC data frame encoding and they are following the ISO/IEC:24730-62:2013 standard. The messages used are:
    - a poll message sent by the initiator to trigger the ranging exchange.
    - a response message sent by the responder allowing the initiator to go on with the process
    - a final message sent by the initiator to complete the exchange and provide all information needed by the responder to compute the
      time-of-flight (distance) estimate.
   The first 10 bytes of those frame are common and are composed of the following fields:
    - byte 0/1: frame control (0x8841 to indicate a data frame using 16-bit addressing).
    - byte 2: sequence number, incremented for each new frame.
    - byte 3/4: PAN ID (0xDECA).
    - byte 5/6: destination address, see NOTE 3 below.
    - byte 7/8: source address, see NOTE 3 below.
    - byte 9: function code (specific values to indicate which message it is in the ranging process).
   The remaining bytes are specific to each message as follows:
   Poll message:
    - no more data
   Response message:
    - byte 10: activity code (0x02 to tell the initiator to go on with the ranging exchange).
    - byte 11/12: activity parameter, not used for activity code 0x02.
   Final message:
    - byte 10 -> 13: poll message transmission timestamp.
    - byte 14 -> 17: response message reception timestamp.
    - byte 18 -> 21: final message transmission timestamp.
   All messages end with a 2-byte checksum automatically set by DW IC.
3. Source and destination addresses are hard coded constants in this example to keep it simple but for a real product every device should have a
   unique ID. Here, 16-bit addressing is used to keep the messages as short as possible but, in an actual application, this should be done only
   after an exchange of specific messages used to define those short addresses for each device participating to the ranging exchange.
4. Delays between frames have been chosen here to ensure proper synchronisation of transmission and reception of the frames between the initiator
   and the responder and to ensure a correct accuracy of the computed distance. The user is referred to DecaRanging ARM Source Code Guide for more
   details about the timings involved in the ranging process.
   Initiator: |Poll TX| ..... |Resp RX| ........ |Final TX|
   Responder: |Poll RX| ..... |Resp TX| ........ |Final RX|
                  ^|P RMARKER|                                    - time of Poll TX/RX
                                  ^|R RMARKER|                    - time of Resp TX/RX
                                                     ^|R RMARKER| - time of Final TX/RX

                      <--TDLY->                                   - POLL_TX_TO_RESP_RX_DLY_UUS (RDLY-RLEN)
                              <-RLEN->                            - RESP_RX_TIMEOUT_UUS   (length of poll frame)
                   <----RDLY------>                               - POLL_RX_TO_RESP_TX_DLY_UUS (depends on how quickly responder
                                                                                                                     can turn around and reply)


                                       <--T2DLY->                 - RESP_TX_TO_FINAL_RX_DLY_UUS (R2DLY-FLEN)
                                                 <-FLEN--->       - FINAL_RX_TIMEOUT_UUS   (length of response frame)
                                   <----RDLY--------->            - RESP_RX_TO_FINAL_TX_DLY_UUS (depends on how quickly initiator
                                                                                                                     can turn around and reply)

EXAMPLE 1: with SPI rate set to 18 MHz (default on this platform), and frame lengths of ~190 us, the delays can be set to:
           POLL_RX_TO_RESP_TX_DLY_UUS of 400uus, and RESP_RX_TO_FINAL_TX_DLY_UUS of 400uus (TXtoRX delays are set to 210uus)
           reducing the delays further can be achieved by using interrupt to handle the TX/RX events, or other code optimisations/faster SPI

EXAMPLE 2: with SPI rate set to 4.5 MHz, and frame lengths of ~190 us, the delays can be set to:
           POLL_RX_TO_RESP_TX_DLY_UUS of 550uus, and RESP_RX_TO_FINAL_TX_DLY_UUS of 600uus (TXtoRX delays are set to 360 and 410 uus respectively)

5. This timeout is for complete reception of a frame, i.e. timeout duration must take into account the length of the expected frame. Here the value
   is arbitrary but chosen large enough to make sure that there is enough time to receive the complete final frame sent by the responder at the
   6.81 Mbps data rate used (around 200 us).
6. The preamble timeout allows the receiver to stop listening in situations where preamble is not starting (which might be because the responder is
   out of range or did not receive the message to respond to). This saves the power waste of listening for a message that is not coming. We
   recommend a minimum preamble timeout of 5 PACs for short range applications and a larger value (e.g. in the range of 50% to 80% of the preamble
   length) for more challenging longer range, NLOS or noisy environments.
7. In a real application, for optimum performance within regulatory limits, it may be necessary to set TX pulse bandwidth and TX power, (using
   the dwt_configuretxrf API call) to per device calibrated values saved in the target system or the DW IC OTP memory.
8. We use polled mode of operation here to keep the example as simple as possible but all status events can be used to generate interrupts. Please
   refer to DW IC User Manual for more details on "interrupts". It is also to be noted that STATUS register is 5 bytes long but, as the event we
   use are all in the first bytes of the register, we can use the simple dwt_read32bitreg() API call to access it instead of reading the whole 5
   bytes.
9. Timestamps and delayed transmission time are both expressed in device time units so we just have to add the desired response delay to poll RX
   timestamp to get response transmission time. The delayed transmission time resolution is 512 device time units which means that the lower 9 bits
   of the obtained value must be zeroed. This also allows to encode the 40-bit value in a 32-bit words by shifting the all-zero lower 8 bits.
10. dwt_writetxdata() takes the full size of the message as a parameter but only copies (size - 2) bytes as the check-sum at the end of the frame is
    automatically appended by the DW IC. This means that our variable could be two bytes shorter without losing any data (but the sizeof would not
    work anymore then as we would still have to indicate the full length of the frame to dwt_writetxdata()).
11. When running this example on the DWK3000 platform with the POLL_RX_TO_RESP_TX_DLY response delay provided, the dwt_starttx() is always
    successful. However, in cases where the delay is too short (or something else interrupts the code flow), then the dwt_starttx() might be issued
    too late for the configured start time. The code below provides an example of how to handle this condition: In this case it abandons the
    ranging exchange and simply goes back to awaiting another poll message. If this error handling code was not here, a late dwt_starttx() would
    result in the code flow getting stuck waiting subsequent RX event that will will never come. The companion "initiator" example (ex_05a) should
    timeout from awaiting the "response" and proceed to send another poll in due course to initiate another ranging exchange.
12. The high order byte of each 40-bit time-stamps is discarded here. This is acceptable as, on each device, those time-stamps are not separated by
    more than 2**32 device time units (which is around 67 ms) which means that the calculation of the round-trip delays can be handled by a 32-bit
    subtraction.
13. The user is referred to DecaRanging ARM application (distributed with EVK1000 product) for additional practical example of usage, and to the
    DW IC API Guide for more details on the DW IC driver functions.
14. In this example, the DW IC is put into IDLE state after calling dwt_initialise(). This means that a fast SPI rate of up to 38 MHz can be used
    thereafter.
15. Desired configuration by user may be different to the current programmed configuration. dwt_configure is called to set desired
    configuration.