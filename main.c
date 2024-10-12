#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "spi.h"
#include "mcp2515.h"
#include "uart.h"
#include "GC9A01A.h"
#include "obd_pid.h"

#define TIMER_COUNT 2

volatile uint16_t softwareTimers[TIMER_COUNT];

void initTimer0()
{
    TCCR0A = (1 << WGM01);              // Set CTC mode
    TCCR0B = (1 << CS01) | (1 << CS00); // Set prescaler to 64
    OCR0A = 249;                        // Set compare value for 1ms interrupts (assuming 16 MHz clock)
    TIMSK0 |= (1 << OCIE0A);            // Enable compare match interrupt
}

ISR(TIMER0_COMPA_vect)
{
    for (int i = 0; i < TIMER_COUNT; i++)
    {
        if (softwareTimers[i] > 0)
        {
            softwareTimers[i]--;
        }
    }
}

void startTimer(uint8_t timerIndex, uint16_t durationMs)
{
    if (timerIndex < TIMER_COUNT)
    {
        softwareTimers[timerIndex] = durationMs;
    }
}

uint8_t isTimerExpired(uint8_t timerIndex)
{
    if (timerIndex < TIMER_COUNT)
    {
        return (softwareTimers[timerIndex] == 0);
    }
    return 0;
}

uint8_t speed = 0;
int16_t oil_temperature = 0;

void obd_request(uint8_t pid)
{
    uart_puts("Requesting PID: ");
    uart_putd(pid);
    uart_puts("\r\n");

    uint8_t buf[] = {0x02, 0x01, pid};
    MCP2515_result_t status;
    if ((status = mcp2515_put(0x7DF, 8, 0, buf, 3)) != MCP2515_RESULT_SUCCESS)
        mcp2515_get_error(status);
}

void obd_request22(uint16_t pid)
{
    uart_puts("Requesting PID: ");
    uart_putd16(pid);
    uart_puts("\r\n");

    uint8_t buf[] = {0x02, 0x22, (uint8_t)(pid & 0xFF), (uint8_t)(pid >> 8)};
    MCP2515_result_t status;
    if ((status = mcp2515_put(0x7E0, 8, 0, buf, 4)) != MCP2515_RESULT_SUCCESS)
        mcp2515_get_error(status);
}

void rx_obd(uint16_t id, uint8_t *buf, uint8_t len)
{
    uart_puts("Received length: ");
    uart_putd(len);
    uart_puts(", bytes: ");
    for (uint8_t i = 0; i < len; i++)
    {
        uart_putd(buf[i]);
        uart_puts(" ");
    }
    uart_puts("\r\n");

    switch (buf[2])
    {
    case OBD2_PID_VEHICLE_SPEED:
        speed = OBD2_PID_VEHICLE_SPEED_CONV(buf[3]);
        return;

    case OBD2_PID_OIL_TEMP:
        oil_temperature = OBD2_PID_OIL_TEMP_CONV(buf[3]);
        return;

    default:
        uart_puts("Unknown PID: ");
        uart_putd(buf[2]);
        uart_puts(".\r\nTry to convert: ");
        uart_putd16(OBD2_PID_ENGINE_TEMP_CONV(buf[3], buf[4]));
        uart_puts("\r\n");
        return;
    }
}

uint16_t value_to_rgb565(uint8_t value)
{
    uint8_t red = (value * 31) / 100;
    uint8_t blue = ((100 - value) * 31) / 100;

    return (red << 11) | blue;
}

int main()
{
    _delay_ms(3000);

    spi_init();
    uart_init();
    initTimer0();

    lcd_init();
    lcd_fill(0x0000);
    lcd_circle_filled(120, 120, 105, 0b1111100000000000);

    sei();

    uart_puts_P(PSTR("Hello, World!\r\n"));

    register_mcp2515_rx_event_callback(rx_obd);

    if (mcp2515_init() != MCP2515_RESULT_SUCCESS)
    {
        uart_puts_P(PSTR("MCP2515 init failed.\r\n"));
        lcd_digit(120 - 2, 20, 0, 0xFFFF);
        while (1)
            ;
    }

#define SPEED_DIGIT_SIZE 11
#define OIL_DIGIT_SIZE 5
#define DIGIT_SIZE_WIDTH 5
#define DIGIT_SIZE_HEIGHT 7

#define SPEED_Y_POS 120 - SPEED_DIGIT_SIZE *DIGIT_SIZE_HEIGHT / 2
#define SPEED_LEFT_X_POS 120 - SPEED_DIGIT_SIZE *DIGIT_SIZE_WIDTH - DIGIT_SIZE_WIDTH
#define SPEED_RIGHT_X_POS 120 + DIGIT_SIZE_WIDTH

#define OIL_TEMP_Y_POS 180 - OIL_DIGIT_SIZE *DIGIT_SIZE_HEIGHT / 2
#define OIL_LEFT_X_POS 120 - OIL_DIGIT_SIZE *DIGIT_SIZE_WIDTH - DIGIT_SIZE_WIDTH
#define OIL_RIGHT_X_POS 120 + DIGIT_SIZE_WIDTH

#define SPEED_DIGIT_COLOR 0xFFFF
#define OIL_DIGIT_COLOR 0b1101011111100000

    uint8_t recv_buf[8];
    for (uint8_t i = 0; i < 8; i++)
        recv_buf[i] = 0x00;

    startTimer(0, 500);
    startTimer(1, 3250);

    while (1)
    {
        mcp2515_get(recv_buf);

        lcd_digit_sized(SPEED_LEFT_X_POS, SPEED_Y_POS, speed / 10, SPEED_DIGIT_SIZE, SPEED_DIGIT_COLOR);
        lcd_digit_sized(SPEED_RIGHT_X_POS, SPEED_Y_POS, speed % 10, SPEED_DIGIT_SIZE, SPEED_DIGIT_COLOR);

        lcd_digit_sized(OIL_LEFT_X_POS, OIL_TEMP_Y_POS, oil_temperature / 10, OIL_DIGIT_SIZE, value_to_rgb565(oil_temperature));
        lcd_digit_sized(OIL_RIGHT_X_POS, OIL_TEMP_Y_POS, oil_temperature % 10, OIL_DIGIT_SIZE, value_to_rgb565(oil_temperature));

        lcd_set_cursor(OIL_RIGHT_X_POS + OIL_DIGIT_SIZE * DIGIT_SIZE_WIDTH + DIGIT_SIZE_WIDTH,
                       OIL_TEMP_Y_POS,
                       OIL_RIGHT_X_POS + OIL_DIGIT_SIZE * DIGIT_SIZE_WIDTH + DIGIT_SIZE_WIDTH + 5 * 3 - 1,
                       OIL_TEMP_Y_POS + 5 * 3 - 1);

        for (uint8_t i = 0; i < 5 * 3; i++)
        {
            for (uint8_t j = 0; j < 5 * 3; j++)
            {
                if ((i >= 5 && i < 10 && j >= 5 && j < 10) || (i == 0 && j == 0) || (i == 0 && j == 14) || (i == 14 && j == 0) || (i == 14 && j == 14))
                    lcd_data_u16(0x0000);
                else
                    lcd_data_u16(value_to_rgb565(oil_temperature));
            }
        }

        // if (isTimerExpired(0))
        // {
        //     uart_puts_P(PSTR("Requesting speed.\r\n"));
        //     obd_request(OBD2_PID_VEHICLE_SPEED);
        //     startTimer(0, 500);
        // }

        if (isTimerExpired(1))
        {
            uart_puts_P(PSTR("Requesting oil temperature.\r\n"));
            // obd_request(OBD2_PID_OIL_TEMP);
            obd_request22(OBD2_PID_ENGINE_TEMP);
            startTimer(1, 3250);
        }
    }

    return 0;
}
