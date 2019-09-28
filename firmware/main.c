#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>
#include "usbdrv.h"
#include "keyboard_device.h"
#include "loader_device.h"

#define NO_RETURN __attribute__((noreturn))
#define ALWAYS_INLINE __attribute__((always_inline))
#define PAYLOAD_LEN 512
#define ONE_SECOND 15625

typedef enum Mode
{
    LOADER = 0,
    KEYBOARD = 0xff
} mode_t;

typedef enum Token
{
    MODIFIER = 0x01,
    TIME = 0x02,
    END_OF_STREAM = 0xff,
} token_t;

static mode_t mode;
static uint16_t eeprom_write_pos;
static uint8_t payload[PAYLOAD_LEN];

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    if (mode == KEYBOARD)
    {
        return keyboard_usb_function_setup(data);
    }
    return loader_usb_function_setup(data);
}

usbMsgLen_t usbFunctionDescriptor(usbRequest_t* rq)
{
    if (mode == KEYBOARD)
    {
        return keyboard_usb_function_descriptor(rq);
    }
    return loader_usb_function_descriptor(rq);
}

usbMsgLen_t usbFunctionWrite(uint8_t data[], uchar len)
{
    if (mode == KEYBOARD)
    {
        return 1;
    }
    for (uint8_t i = 0; i < len; ++i)
    {
        if (eeprom_write_pos == PAYLOAD_LEN)
        {
            eeprom_write_pos = 0;
            return 1;
        }
        eeprom_update_byte((uint8_t*) 0 + eeprom_write_pos, data[i]);
        if (data[i] == END_OF_STREAM)
        {
            eeprom_write_pos = 0;
            return 1;
        }
        ++eeprom_write_pos;
    }
    return 0;
}

usbMsgLen_t usbFunctionRead(uchar* data, uchar len)
{
    return 1;
}

static inline void usb_device_restart()
{
    cli();
    usbDeviceDisconnect();
    for (uint8_t i = 255; i; --i)
    {
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();
}

ALWAYS_INLINE
static inline void fill_report_with_zeroes()
{
    memset(&keyboard_report, 0, sizeof(keyboard_report_t));
}

ALWAYS_INLINE
static inline void stop_clock()
{
    TIFR |= (1 << OCF1A);
    TCCR1B = 0;
}

ALWAYS_INLINE
static inline void start_clock()
{
    TCCR1B = (1 << CS10) | (1 << CS12);
    TCNT1 = 0;
}

ALWAYS_INLINE
static inline mode_t check_mode()
{
    return bit_is_set(PINC, PC1) ? KEYBOARD : LOADER;
}

NO_RETURN
int main()
{
    PORTC |= 1 << PC1;
    eeprom_busy_wait();
    wdt_enable(WDTO_1S);

    mode = check_mode();
    if (mode == LOADER)
    {
        eeprom_read_block(payload, (uint8_t*) 0, PAYLOAD_LEN);
    }

    usbInit();
    usb_device_restart();

    bool can_interrupt = true;
    bool send_zero_report = false;
    uint16_t eeprom_read_pos = 0;
    for (;;)
    {
        wdt_reset();
        usbPoll();
        const mode_t actual_mode_flag = check_mode();
        if (actual_mode_flag != mode)
        {
            usb_device_restart();
            mode = actual_mode_flag;
            if (mode == LOADER)
            {
                eeprom_read_block(payload, (uint8_t*) 0, PAYLOAD_LEN);
            }
        }
        if (TIFR & (1 << OCF1A))
        {
            can_interrupt = true;
            stop_clock();
        }
        if (mode && can_interrupt && usbInterruptIsReady())
        {
            if (send_zero_report)
            {
                fill_report_with_zeroes();
                usbSetInterrupt((void*) &keyboard_report, sizeof(keyboard_report_t));
                send_zero_report = false;
            }
            else if (payload[eeprom_read_pos] == TIME)
            {
                OCR1A = payload[eeprom_read_pos + 1] * ONE_SECOND;
                start_clock();
                can_interrupt = false;
                eeprom_read_pos += 2;
            }
            else if (payload[eeprom_read_pos] == END_OF_STREAM)
            {
                can_interrupt = false;
            }
            else
            {
                if (payload[eeprom_read_pos] == MODIFIER)
                {
                    keyboard_report.modifier = payload[eeprom_read_pos + 1];
                    eeprom_read_pos += 2;
                }
                keyboard_report.key_code[0] = payload[eeprom_read_pos];
                usbSetInterrupt((void*) &keyboard_report, sizeof(keyboard_report_t));
                ++eeprom_read_pos;
                send_zero_report = true;
            }
        }
    }
}
