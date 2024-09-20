#ifndef __GC9A01A_H_
#define __GC9A01A_H_

#include <avr/io.h>
#include <util/delay.h>
#include "spi.h"

#define RST_PIN PB0
#define DC_PIN PB1
#define CS_PIN PB2

#define RST_HIGH() PORTB |= (1 << RST_PIN)
#define RST_LOW() PORTB &= ~(1 << RST_PIN)
#define DC_HIGH() PORTB |= (1 << DC_PIN)
#define DC_LOW() PORTB &= ~(1 << DC_PIN)
#define CS_HIGH() PORTB |= (1 << CS_PIN)
#define CS_LOW() PORTB &= ~(1 << CS_PIN)

#define LCD_WIDTH 240
#define LCD_HEIGHT 240

void lcd_init();

void lcd_set_cursor(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend);

void lcd_fill(uint16_t color);
void lcd_fill_partial(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
void lcd_circle_filled(uint8_t x, uint8_t y, uint8_t r, uint16_t color);

void lcd_digit_sized(uint8_t x, uint8_t y, uint8_t digit, uint8_t scale, uint16_t color);
void lcd_digit(uint8_t x, uint8_t y, uint8_t digit, uint16_t color);

void lcd_data_u16(uint16_t word);

#endif