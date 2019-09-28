#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>
#include "usbdrv.h"
#include "usb_hid_keys.h"
#include "keyboard_device.h"
#include "loader_device.h"

#define NO_RETURN __attribute__((noreturn))
#define ALWAYS_INLINE __attribute__((always_inline))
#define PAYLOAD_BUFFER_LEN 512
#define END_OF_STREAM 0xff

typedef enum
{
    LOADER = 0,
    KEYBOARD = 0xff
} mode_t;

static mode_t mode;
static uint16_t eeprom_write_pos;
static uint8_t payload_buffer[PAYLOAD_BUFFER_LEN];

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
        if (eeprom_write_pos == PAYLOAD_BUFFER_LEN)
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
static inline uint8_t check_mode()
{
    return bit_is_set(PINC, PC1) ? KEYBOARD : LOADER;
}

NO_RETURN
int main()
{
    PORTC |= 1u << PC1;
    eeprom_busy_wait();
    wdt_enable(WDTO_1S);

    mode = check_mode();
    if (mode)
    {
        eeprom_read_block(payload_buffer, (uint8_t*) 0, PAYLOAD_BUFFER_LEN);
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
        const uint8_t actual_mode_flag = check_mode();
        if (actual_mode_flag != mode)
        {
            usb_device_restart();
            mode = actual_mode_flag;
            if (mode)
            {
                eeprom_read_block(payload_buffer, (uint8_t*) 0, PAYLOAD_BUFFER_LEN);
            }
        }
        if (TIFR & (1 << OCF1A))
        {
            can_interrupt = true;
            TIFR |= (1 << OCF1A);
            TCCR1B = 0;
        }
        if (mode && can_interrupt && usbInterruptIsReady())
        {
            if (send_zero_report)
            {
                memset(&keyboard_report, 0, sizeof(keyboard_report_t));
                usbSetInterrupt((void*) &keyboard_report, sizeof(keyboard_report_t));
                send_zero_report = false;
            }
            else if (payload_buffer[eeprom_read_pos] == 0x02)
            {
                TCCR1B = (1 << CS10) | (1 << CS12);
                OCR1A = payload_buffer[eeprom_read_pos + 1] * 15625;
                TCNT1 = 0;
                can_interrupt = false;
                eeprom_read_pos += 2;
            }
            else if (payload_buffer[eeprom_read_pos] == 0xff)
            {
                can_interrupt = false;
            }
            else
            {
                if (payload_buffer[eeprom_read_pos] == 0x01)
                {
                    keyboard_report.modifier = payload_buffer[eeprom_read_pos + 1];
                    eeprom_read_pos += 2;
                }
                keyboard_report.key_code[0] = payload_buffer[eeprom_read_pos];
                usbSetInterrupt((void*) &keyboard_report, sizeof(keyboard_report_t));
                ++eeprom_read_pos;
                send_zero_report = true;
            }
        }
    }
}
