#include "GC9A01A.h"

static uint8_t font[10][5] = {
    {0b0111110, 0b1000101, 0b1001001, 0b1010001, 0b0111110}, // 0
    {0b0000000, 0b0100001, 0b1111111, 0b0000001, 0b0000000}, // 1
    {0b0100111, 0b1001001, 0b1001001, 0b1001001, 0b0110001}, // 2
    {0b1000010, 0b1000001, 0b1001001, 0b1011001, 0b1100110}, // 3
    {0b0001100, 0b0010100, 0b0100100, 0b1111111, 0b0000100}, // 4
    {0b1110010, 0b1010001, 0b1010001, 0b1010001, 0b1001110}, // 5
    {0b0011110, 0b0101001, 0b1001001, 0b1001001, 0b1000110}, // 6
    {0b1000001, 0b1000010, 0b1000100, 0b1001000, 0b1110000}, // 7
    {0b0110110, 0b1001001, 0b1001001, 0b1001001, 0b0110110}, // 8
    {0b0110001, 0b1001001, 0b1001001, 0b1001010, 0b0111100}  // 9
};

static void lcd_reset()
{
    RST_HIGH();
    _delay_ms(100);
    RST_LOW();
    _delay_ms(100);
    RST_HIGH();
    _delay_ms(100);
}

static void lcd_command(uint8_t cmd)
{
    DC_LOW();
    CS_LOW();
    spi_transmit(cmd);
    CS_HIGH();
}

static void lcd_data(uint8_t data)
{
    DC_HIGH();
    CS_LOW();
    spi_transmit(data);
    CS_HIGH();
}

void lcd_data_u16(uint16_t word)
{
    DC_HIGH();
    CS_LOW();
    spi_transmit(word >> 8);
    spi_transmit(word);
    CS_HIGH();
}

void lcd_set_cursor(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend)
{
    lcd_command(0x2A);
    lcd_data(0x00);
    lcd_data(xstart);
    lcd_data(0x00);
    lcd_data(xend);

    lcd_command(0x2B);
    lcd_data(0x00);
    lcd_data(ystart);
    lcd_data(0x00);
    lcd_data(yend);

    lcd_command(0x2C);
}

void lcd_fill(uint16_t color)
{
    lcd_set_cursor(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    DC_HIGH();
    CS_LOW();
    for (uint16_t i = 0; i < ((uint16_t)LCD_WIDTH * LCD_HEIGHT); i++)
    {
        lcd_data_u16(color);
    }
    CS_HIGH();
}

void lcd_digit(uint8_t x, uint8_t y, uint8_t digit, uint16_t color)
{
    lcd_set_cursor(x, y, x + 4, y + 6);

    for (uint8_t j = 0; j < 7; j++)
    {
        for (uint8_t i = 0; i < 5; i++)
        {
            if (font[digit][i] & (1 << (6 - j)))
                lcd_data_u16(0xFFFF);
            else
                lcd_data_u16(0x0000);
        }
    }
}

void lcd_digit_sized(uint8_t x, uint8_t y, uint8_t digit, uint8_t scale, uint16_t color)
{
    lcd_set_cursor(x, y, x + 5 * scale - 1, y + 7 * scale - 1);

    for (uint8_t j = 0; j < 7; j++)
    {
        for (uint8_t l = 0; l < scale; l++)
        {
            for (uint8_t i = 0; i < 5; i++)
            {
                for (uint8_t k = 0; k < scale; k++)
                {
                    if (font[digit][i] & (1 << (6 - j)))
                        lcd_data_u16(color);
                    else
                        lcd_data_u16(0x0000);
                }
            }
        }
    }
}

void lcd_fill_partial(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color)
{
    for (uint8_t i = 0; i < w; i++)
    {
        for (uint8_t j = 0; j < h; j++)
        {
            lcd_set_cursor(x + i, y + j, x + i, y + j);
            lcd_data_u16(color);
        }
    }
}

void lcd_circle_filled(uint8_t x, uint8_t y, uint8_t r, uint16_t color)
{
    for (uint8_t cy = 0; cy < LCD_HEIGHT; cy++)
    {
        for (uint8_t cx = 0; cx < LCD_WIDTH; cx++)
        {
            int16_t dx = cx - x;
            int16_t dy = cy - y;

            if (dx * dx + dy * dy <= r * r)
                lcd_data_u16(0x0000);
            else
                lcd_data_u16(color);
        }
    }
}

void lcd_init()
{
    DDRB |= (1 << RST_PIN) | (1 << DC_PIN) | (1 << CS_PIN);

    lcd_reset();

    // set attributes
    lcd_command(0x36);
    lcd_data(0xC8);

    // init registers
    lcd_command(0xEF);
    lcd_command(0xEB);
    lcd_data(0x14);

    lcd_command(0xFE);
    lcd_command(0xEF);
    lcd_command(0xEB);
    lcd_data(0x14);

    lcd_command(0x84);
    lcd_data(0x40);

    lcd_command(0x85);
    lcd_data(0xFF);

    lcd_command(0x86);
    lcd_data(0xFF);

    lcd_command(0x87);
    lcd_data(0xFF);

    lcd_command(0x88);
    lcd_data(0x0A);

    lcd_command(0x89);
    lcd_data(0x21);

    lcd_command(0x8A);
    lcd_data(0x00);

    lcd_command(0x8B);
    lcd_data(0x80);

    lcd_command(0x8C);
    lcd_data(0x01);

    lcd_command(0x8D);
    lcd_data(0x01);

    lcd_command(0x8E);
    lcd_data(0xFF);

    lcd_command(0x8F);
    lcd_data(0xFF);

    lcd_command(0xB6);
    lcd_data(0x00);
    lcd_data(0x20);

    lcd_command(0x36);
    lcd_data(0x08);

    lcd_command(0x3A);
    lcd_data(0x05);

    lcd_command(0x90);
    lcd_data(0x08);
    lcd_data(0x08);
    lcd_data(0x08);
    lcd_data(0x08);

    lcd_command(0xBD);
    lcd_data(0x06);

    lcd_command(0xBC);
    lcd_data(0x00);

    lcd_command(0xFF);
    lcd_data(0x60);
    lcd_data(0x01);
    lcd_data(0x04);

    lcd_command(0xC3);
    lcd_data(0x13);
    lcd_command(0xC4);
    lcd_data(0x13);

    lcd_command(0xC9);
    lcd_data(0x22);

    lcd_command(0xBE);
    lcd_data(0x11);

    lcd_command(0xE1);
    lcd_data(0x10);
    lcd_data(0x0E);

    lcd_command(0xDF);
    lcd_data(0x21);
    lcd_data(0x0c);
    lcd_data(0x02);

    lcd_command(0xF0);
    lcd_data(0x45);
    lcd_data(0x09);
    lcd_data(0x08);
    lcd_data(0x08);
    lcd_data(0x26);
    lcd_data(0x2A);

    lcd_command(0xF1);
    lcd_data(0x43);
    lcd_data(0x70);
    lcd_data(0x72);
    lcd_data(0x36);
    lcd_data(0x37);
    lcd_data(0x6F);

    lcd_command(0xF2);
    lcd_data(0x45);
    lcd_data(0x09);
    lcd_data(0x08);
    lcd_data(0x08);
    lcd_data(0x26);
    lcd_data(0x2A);

    lcd_command(0xF3);
    lcd_data(0x43);
    lcd_data(0x70);
    lcd_data(0x72);
    lcd_data(0x36);
    lcd_data(0x37);
    lcd_data(0x6F);

    lcd_command(0xED);
    lcd_data(0x1B);
    lcd_data(0x0B);

    lcd_command(0xAE);
    lcd_data(0x77);

    lcd_command(0xCD);
    lcd_data(0x63);

    lcd_command(0x70);
    lcd_data(0x07);
    lcd_data(0x07);
    lcd_data(0x04);
    lcd_data(0x0E);
    lcd_data(0x0F);
    lcd_data(0x09);
    lcd_data(0x07);
    lcd_data(0x08);
    lcd_data(0x03);

    lcd_command(0xE8);
    lcd_data(0x34);

    lcd_command(0x62);
    lcd_data(0x18);
    lcd_data(0x0D);
    lcd_data(0x71);
    lcd_data(0xED);
    lcd_data(0x70);
    lcd_data(0x70);
    lcd_data(0x18);
    lcd_data(0x0F);
    lcd_data(0x71);
    lcd_data(0xEF);
    lcd_data(0x70);
    lcd_data(0x70);

    lcd_command(0x63);
    lcd_data(0x18);
    lcd_data(0x11);
    lcd_data(0x71);
    lcd_data(0xF1);
    lcd_data(0x70);
    lcd_data(0x70);
    lcd_data(0x18);
    lcd_data(0x13);
    lcd_data(0x71);
    lcd_data(0xF3);
    lcd_data(0x70);
    lcd_data(0x70);

    lcd_command(0x64);
    lcd_data(0x28);
    lcd_data(0x29);
    lcd_data(0xF1);
    lcd_data(0x01);
    lcd_data(0xF1);
    lcd_data(0x00);
    lcd_data(0x07);

    lcd_command(0x66);
    lcd_data(0x3C);
    lcd_data(0x00);
    lcd_data(0xCD);
    lcd_data(0x67);
    lcd_data(0x45);
    lcd_data(0x45);
    lcd_data(0x10);
    lcd_data(0x00);
    lcd_data(0x00);
    lcd_data(0x00);

    lcd_command(0x67);
    lcd_data(0x00);
    lcd_data(0x3C);
    lcd_data(0x00);
    lcd_data(0x00);
    lcd_data(0x00);
    lcd_data(0x01);
    lcd_data(0x54);
    lcd_data(0x10);
    lcd_data(0x32);
    lcd_data(0x98);

    lcd_command(0x74);
    lcd_data(0x10);
    lcd_data(0x85);
    lcd_data(0x80);
    lcd_data(0x00);
    lcd_data(0x00);
    lcd_data(0x4E);
    lcd_data(0x00);

    lcd_command(0x98);
    lcd_data(0x3e);
    lcd_data(0x07);

    lcd_command(0x35);
    lcd_command(0x21);

    lcd_command(0x11);
    _delay_ms(120);
    lcd_command(0x29);
    _delay_ms(20);
}