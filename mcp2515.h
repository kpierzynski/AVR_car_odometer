#ifndef __MCP2515_H_
#define __MCP2515_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "spi.h"

#define USE_IRQ

typedef enum MCP2515_result
{
    MCP2515_RESULT_SUCCESS = 0,
    MCP2515_RESULT_ERROR = 1,
    MCP2515_RESULT_INVALID_PARAMETER = 2
} MCP2515_result_t;

typedef struct
{
    uint16_t tx_id;
    uint8_t tx_len;
    uint8_t tx_data[8];
    int8_t tx_dlc;
    int8_t tx_rtr;

} packet_t;

#define REG_BFPCTRL 0x0c
#define REG_TXRTSCTRL 0x0d

#define REG_CANCTRL 0x0f

#define REG_CNF3 0x28
#define REG_CNF2 0x29
#define REG_CNF1 0x2a

#define REG_CANINTE 0x2b
#define REG_CANINTF 0x2c

#define FLAG_RXnIE(n) (0x01 << n)
#define FLAG_RXnIF(n) (0x01 << n)
#define FLAG_TXnIF(n) (0x04 << n)

#define REG_RXFnSIDH(n) (0x00 + (n * 4))
#define REG_RXFnSIDL(n) (0x01 + (n * 4))
#define REG_RXFnEID8(n) (0x02 + (n * 4))
#define REG_RXFnEID0(n) (0x03 + (n * 4))

#define REG_RXMnSIDH(n) (0x20 + (n * 0x04))
#define REG_RXMnSIDL(n) (0x21 + (n * 0x04))
#define REG_RXMnEID8(n) (0x22 + (n * 0x04))
#define REG_RXMnEID0(n) (0x23 + (n * 0x04))

#define REG_TXBnCTRL(n) (0x30 + (n * 0x10))
#define REG_TXBnSIDH(n) (0x31 + (n * 0x10))
#define REG_TXBnSIDL(n) (0x32 + (n * 0x10))
#define REG_TXBnEID8(n) (0x33 + (n * 0x10))
#define REG_TXBnEID0(n) (0x34 + (n * 0x10))
#define REG_TXBnDLC(n) (0x35 + (n * 0x10))
#define REG_TXBnD0(n) (0x36 + (n * 0x10))

#define REG_RXBnCTRL(n) (0x60 + (n * 0x10))
#define REG_RXBnSIDH(n) (0x61 + (n * 0x10))
#define REG_RXBnSIDL(n) (0x62 + (n * 0x10))
#define REG_RXBnEID8(n) (0x63 + (n * 0x10))
#define REG_RXBnEID0(n) (0x64 + (n * 0x10))
#define REG_RXBnDLC(n) (0x65 + (n * 0x10))
#define REG_RXBnD0(n) (0x66 + (n * 0x10))

#define FLAG_IDE 0x08
#define FLAG_SRR 0x10
#define FLAG_RTR 0x40
#define FLAG_EXIDE 0x08

#define FLAG_RXM0 0x20
#define FLAG_RXM1 0x40

#define MCP2515_CS_DDR DDRD
#define MCP2515_CS_PORT PORTD
#define MCP2515_CS_PIN PD7
#define MCP2515_CS_LOW MCP2515_CS_PORT &= ~(1 << MCP2515_CS_PIN)
#define MCP2515_CS_HIGH MCP2515_CS_PORT |= (1 << MCP2515_CS_PIN)

#define MCP2515_INT_DDR DDRD
#define MCP2515_INT_PORT PORTD
#define MCP2515_INT_PIN PD2
#define MCP2515_INT INT0
#define MCP2515_INT_vect INT0_vect

#define MCP2515_RESET_DDR DDRD
#define MCP2515_RESET_PORT PORTD
#define MCP2515_RESET_PIN PD3
#define MCP2515_RESET_LOW MCP2515_RESET_PORT &= ~(1 << MCP2515_RESET_PIN)
#define MCP2515_RESET_HIGH MCP2515_RESET_PORT |= (1 << MCP2515_RESET_PIN)

void register_mcp2515_rx_event_callback(void (*callback)(uint16_t id, uint8_t *buf, uint8_t len));

MCP2515_result_t mcp2515_init();
MCP2515_result_t mcp2515_filer(uint16_t id, uint16_t mask);

MCP2515_result_t mcp2515_put(uint16_t id, int8_t dlc, int8_t rtr, uint8_t *data, uint8_t len);
MCP2515_result_t mcp2515_get(uint8_t *rx_data);

#endif