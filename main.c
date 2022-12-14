/*
 * File Name: main.c
 * Project: MSP430 Programing
 * File Created: 25-08-2022  10:53:56:PM
 * Author: <<KhangKT>>
 * AuthorEmail: khangkieutrong@gmail.com
 * Phone:0964991713
 */

#include <msp430g2553.h>
#include <stdint.h>
#include "Include/mcu.h"
#include "Include/uart.h"
#include "Include/spi.h"
#include "Include/sx1276.h"
#include "Include/sx1276regs-fsk.h"
#include "String.h"

#define RF_FREQUENCY 434000000 // Hz

#define FSK_FDEV 25e3              // Hz
#define FSK_DATARATE 50e3          // bps
#define FSK_BANDWIDTH 50e3         // Hz
#define FSK_AFC_BANDWIDTH 83.333e3 // Hz
#define FSK_PREAMBLE_LENGTH 5      // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON false

#define LORA_BANDWIDTH 1         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 12 // [SF7..SF12]
#define LORA_CODINGRATE 1        // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH 8   // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 3    // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define RX_TIMEOUT_VALUE 1000
#define TX_OUTPUT_POWER 17 // dBm
#define BUFFER_SIZE 32     // Define the payload size here

char buffer[BUFFER_SIZE];
char Rxbuffer[BUFFER_SIZE];
static radio_events_t radio_events;

int state = 0;

void SendPing()
{

  buffer[0] = 'A';
  sx1276_send((uint8_t *)buffer, 1);
}

void OnTxDone()
{
  uart_write("Data Sending");
  uart_writec(':');
  uart_write(buffer);
  uart_writec('\n');
  uart_write("TX_Done\n");
  P1OUT |= BIT0;
  __delay_cycles(100000);
  P1OUT &= ~BIT0;
  __delay_cycles(100000);
  // if(state == 1) sx1276_set_rx(0);
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  P1OUT |= BIT0;
  uart_write("Data Recevied");
  uart_writec(':');
  uart_writec(UCB0RXBUF);
  uart_writec('\n');
  uart_write("RX_Done\n");
  P1OUT &= ~BIT0;

  if (state == 1)
    SendPing();
}

void OnRxError()
{

  uart_write("$RXE\n");
}

void rf_init_lora()
{
  radio_events.TxDone = OnTxDone;
  radio_events.RxDone = OnRxDone;
  radio_events.RxError = OnRxError;

  sx1276_init(&radio_events);
  sx1276_set_channel(RF_FREQUENCY);

  sx1276_set_txconfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

  sx1276_set_rxconfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
}

void main(void)
{
  mcu_init();

  P1DIR |= BIT0;
  P2DIR |= BIT3;
  P2OUT = BIT3;
  P1OUT &= ~BIT0;

  uart_init();
  uart_write("\n\n");
  spi_init();

  rf_init_lora();

  uart_write("$IND,");
  uart_printhex8(sx1276_read(REG_VERSION));
  uart_writec(',');
  uart_printhex32(RF_FREQUENCY);
  uart_writec(',');
  uart_printhex8(TX_OUTPUT_POWER);
  uart_writec(',');
  uart_printhex8(LORA_BANDWIDTH);
  uart_writec(',');
  uart_printhex8(LORA_SPREADING_FACTOR);
  uart_writec(',');
  uart_printhex8(LORA_CODINGRATE);
  uart_writec('\n');

  //_enable_interrupt();
  while (1)
  {
    //    state = 0;
    //
    //      SendPing();
    //      //state = 1;
    //      __delay_cycles(3000000);
    //
    //

    state = 1;
    sx1276_set_rx(0);
    __delay_cycles(300000);

    P2OUT |= BIT3;
  }
}