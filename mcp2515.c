#include "mcp2515.h"

static uint8_t irq_request = 0;

static void (*mcp2515_rx_event_callback)(uint16_t id, uint8_t *buf, uint8_t len);

void register_mcp2515_rx_event_callback(void (*callback)(uint16_t id, uint8_t *buf, uint8_t len))
{
    mcp2515_rx_event_callback = callback;
}

static void mcp2515_write_reg(uint8_t address, uint8_t value)
{
    MCP2515_CS_LOW;
    spi_transmit(0x02);
    spi_transmit(address);
    spi_transmit(value);
    MCP2515_CS_HIGH;
}

static uint8_t mcp2515_read_reg(uint8_t address)
{
    MCP2515_CS_LOW;
    spi_transmit(0x03);
    spi_transmit(address);
    uint8_t value = spi_transmit(0x00);
    MCP2515_CS_HIGH;

    return value;
}

static void mcp2515_modify_reg(uint8_t address, uint8_t mask, uint8_t data)
{
    MCP2515_CS_LOW;
    spi_transmit(0x05);
    spi_transmit(address);
    spi_transmit(mask);
    spi_transmit(data);
    MCP2515_CS_HIGH;
}

static void mcp2515_soft_reset()
{
    MCP2515_CS_LOW;
    spi_transmit(0xC0);
    MCP2515_CS_HIGH;
}

static MCP2515_result_t mcp2515_enter_config()
{
    mcp2515_write_reg(REG_CANCTRL, 0x80);
    if (mcp2515_read_reg(REG_CANCTRL) != 0x80)
        return MCP2515_RESULT_ERROR;

    return MCP2515_RESULT_SUCCESS;
}

MCP2515_result_t mcp2515_init()
{
    MCP2515_CS_DDR |= (1 << MCP2515_CS_PIN);

    mcp2515_soft_reset();

    if (mcp2515_enter_config() != MCP2515_RESULT_SUCCESS)
        return MCP2515_RESULT_ERROR;

#ifdef USE_IRQ
    MCP2515_INT_DDR &= ~(1 << MCP2515_INT_PIN);

    EICRA |= (1 << ISC01);
    EIMSK |= (1 << MCP2515_INT);
#endif

    // for 8MHz crystal and 500E3 baud rate:
    // (long)8E6,  (long)500E3, { 0x00, 0x90, 0x02 }

    mcp2515_write_reg(REG_CNF1, 0x00);
    mcp2515_write_reg(REG_CNF2, 0x90);
    mcp2515_write_reg(REG_CNF3, 0x02);

    mcp2515_write_reg(REG_CANINTE, FLAG_RXnIE(1) | FLAG_RXnIE(0));
    mcp2515_write_reg(REG_BFPCTRL, 0x00);
    mcp2515_write_reg(REG_TXRTSCTRL, 0x00);
    mcp2515_write_reg(REG_RXBnCTRL(0), FLAG_RXM1 | FLAG_RXM0);
    mcp2515_write_reg(REG_RXBnCTRL(1), FLAG_RXM1 | FLAG_RXM0);

    mcp2515_write_reg(REG_CANCTRL, 0x00);
    if (mcp2515_read_reg(REG_CANCTRL) != 0x00)
        return MCP2515_RESULT_ERROR;

    return MCP2515_RESULT_SUCCESS;
}

MCP2515_result_t mcp2515_filer(uint16_t id, uint16_t mask)
{
    mask &= 0x7FF;
    id &= 0x7FF;

    // if (id > 0x7FF || mask > 0x7FF) return MCP2515_RESULT_INVALID_PARAMETER;

    if (mcp2515_enter_config() != MCP2515_RESULT_SUCCESS)
        return MCP2515_RESULT_ERROR;

    for (uint8_t n = 0; n < 2; n++)
    {
        mcp2515_write_reg(REG_RXBnCTRL(n), FLAG_RXM0);
        mcp2515_write_reg(REG_RXBnCTRL(n), FLAG_RXM0);

        mcp2515_write_reg(REG_RXMnSIDH(n), mask >> 3);
        mcp2515_write_reg(REG_RXMnSIDL(n), mask << 5);

        mcp2515_write_reg(REG_RXMnEID8(n), 0x00);
        mcp2515_write_reg(REG_RXMnEID0(n), 0x00);
    }

    for (uint8_t n = 0; n < 6; n++)
    {
        mcp2515_write_reg(REG_RXFnSIDH(n), id >> 3);
        mcp2515_write_reg(REG_RXFnSIDL(n), id << 5);
        mcp2515_write_reg(REG_RXFnEID8(n), 0x00);
        mcp2515_write_reg(REG_RXFnEID0(n), 0x00);
    }

    mcp2515_write_reg(REG_CANCTRL, 0x00);
    if (mcp2515_read_reg(REG_CANCTRL) != 0x00)
        return MCP2515_RESULT_ERROR;

    return MCP2515_RESULT_SUCCESS;
}

MCP2515_result_t mcp2515_put(uint16_t id, int8_t dlc, int8_t rtr, uint8_t *data, uint8_t len)
{
    if (id > 0x7FF || dlc > 8)
        return MCP2515_RESULT_INVALID_PARAMETER;

    mcp2515_write_reg(REG_TXBnSIDH(0), id >> 3);
    mcp2515_write_reg(REG_TXBnSIDL(0), id << 5);
    mcp2515_write_reg(REG_TXBnEID8(0), 0x00);
    mcp2515_write_reg(REG_TXBnEID0(0), 0x00);

    if (rtr)
        mcp2515_write_reg(REG_TXBnDLC(0), 0x40 | dlc);
    else
    {
        mcp2515_write_reg(REG_TXBnDLC(0), dlc);

        for (uint8_t i = 0; i < len; i++)
            mcp2515_write_reg(REG_TXBnD0(0) + i, data[i]);
    }

    mcp2515_write_reg(REG_TXBnCTRL(0), 0x08);

    uint8_t status = 0;
    while (mcp2515_read_reg(REG_TXBnCTRL(0)) & 0x08)
    {
        if ((status = (mcp2515_read_reg(REG_RXBnCTRL(0) & 0x10))))
            mcp2515_modify_reg(REG_CANCTRL, 0x10, 0x10);
    }

    if (status)
        mcp2515_modify_reg(REG_CANCTRL, 0x10, 0x00);

    mcp2515_modify_reg(REG_CANINTF, FLAG_TXnIF(0), 0x00);

    return (mcp2515_read_reg(REG_TXBnCTRL(0)) & 0x70) ? MCP2515_RESULT_ERROR : MCP2515_RESULT_SUCCESS;
}

MCP2515_result_t mcp2515_get(uint8_t *rx_data)
{
#ifdef USE_IRQ
    if (!irq_request)
        return MCP2515_RESULT_ERROR;
#endif

    uint8_t status = mcp2515_read_reg(REG_CANINTF);

    if (!(status & FLAG_RXnIF(0)))
        return MCP2515_RESULT_ERROR;

    uint16_t id = ((mcp2515_read_reg(REG_RXBnSIDH(0)) << 3) & 0x07F8) | ((mcp2515_read_reg(REG_RXBnSIDL(0)) >> 5) & 0x07);
    uint8_t rx_rtr = (mcp2515_read_reg(REG_RXBnSIDL(0)) & FLAG_SRR) ? 1 : 0;
    uint8_t rx_dlc = mcp2515_read_reg(REG_RXBnDLC(0)) & 0x0F;

    uint8_t rx_len = 0;

    if (rx_rtr)
        rx_len = 0;
    else
    {
        rx_len = rx_dlc;
        for (uint8_t i = 0; i < rx_len; i++)
        {
            rx_data[i] = mcp2515_read_reg(REG_RXBnD0(0) + i);
        }
    }

    mcp2515_modify_reg(REG_CANINTF, FLAG_RXnIF(0), 0x00);

    if (mcp2515_rx_event_callback)
        mcp2515_rx_event_callback(id, rx_data, rx_len);

#ifdef USE_IRQ
    irq_request = 0;
#endif

    return MCP2515_RESULT_SUCCESS;
}

ISR(MCP2515_INT_vect)
{
    irq_request = 1;
}