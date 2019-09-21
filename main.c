#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>
#include "usbdrv.h"
#include "usb_hid_keys.h"
#include "keyboard_descriptors.h"
#include "loader_descriptors.h"

#define NO_RETURN __attribute__((noreturn))
#define ALWAYS_INLINE __attribute__((always_inline))
#define MODE_LOADER 0
#define MODE_KEYBOARD 0xff
#define PAYLOAD_BUFFER_LEN 512

static keyboard_report_t keyboard_report;
static uint8_t idle_rate;
static uint8_t mode;
static uint16_t eeprom_write_pos;
static uint8_t payload_buffer[PAYLOAD_BUFFER_LEN];

usbMsgLen_t keyboard_usb_function_descriptor(usbRequest_t* rq)
{
    switch (rq->wValue.bytes[1])
    {
        case USBDESCR_HID:
            {
                usbMsgPtr = (usbMsgPtr_t) keyboard_config_descriptor + 25;
                return HID_DESCRIPTOR_LEN;
            }
        case USBDESCR_CONFIG:
            {
                usbMsgPtr = (usbMsgPtr_t) keyboard_config_descriptor;
                return KEYBOARD_CONFIG_TOTAL_LEN;
            }
        case USBDESCR_HID_REPORT:
            {
                usbMsgPtr = (usbMsgPtr_t) keyboard_hid_report_descriptor;
                return KEYBOARD_HID_REPORT_DESCRIPTOR_LEN;
            }
        case USBDESCR_DEVICE:
            {
                usbMsgPtr = (usbMsgPtr_t) keyboard_device_descriptor;
                return KEYBOARD_DEVICE_DESCRIPTOR_LEN;
            }
        default:
            {
                switch (rq->wValue.bytes[0])
                {
                    case 1:
                        {
                            usbMsgPtr = (usbMsgPtr_t) keyboard_string_vendor_descriptor;
                            return KEYBOARD_STRING_VENDOR_DESCRIPTOR_LEN;
                        }
                    case 2:
                        {
                            usbMsgPtr = (usbMsgPtr_t) keyboard_string_product_descriptor;
                            return KEYBOARD_STRING_PRODUCT_DESCRIPTOR_LEN;
                        }
                }
            }
    }
    return 0;
}

usbMsgLen_t keyboard_usb_function_setup(uchar data[8])
{
    usbRequest_t* rq = (void*) data;
    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS)
    {
        switch (rq->bRequest)
        {
            case USBRQ_HID_GET_REPORT:
                {
                    usbMsgPtr = (void*) &keyboard_report;
                    return sizeof(keyboard_report);
                }
            case USBRQ_HID_SET_REPORT:
                {
                    return (rq->wLength.word == 1) ? USB_NO_MSG : 0;
                }
            case USBRQ_HID_GET_IDLE:
                {
                    usbMsgPtr = &idle_rate;
                    return 1;
                }
            case USBRQ_HID_SET_IDLE:
                {
                    idle_rate = rq->wValue.bytes[1];
                }
        }
    }
    return 0;
}

usbMsgLen_t loader_usb_function_descriptor(usbRequest_t* rq)
{
    switch (rq->wValue.bytes[1])
    {
        case USBDESCR_HID:
            {
                usbMsgPtr = (usbMsgPtr_t) loader_config_descriptor + 25;
                return HID_DESCRIPTOR_LEN;
            }
        case USBDESCR_CONFIG:
            {
                usbMsgPtr = (usbMsgPtr_t) loader_config_descriptor;
                return LOADER_CONFIG_TOTAL_LEN;
            }
        case USBDESCR_HID_REPORT:
            {
                usbMsgPtr = (usbMsgPtr_t) loader_hid_report_descriptor;
                return LOADER_HID_REPORT_DESCRIPTOR_LEN;
            }
        case USBDESCR_DEVICE:
            {
                usbMsgPtr = (usbMsgPtr_t) loader_device_descriptor;
                return LOADER_DEVICE_DESCRIPTOR_LEN;
            }
        default:
            {
                switch (rq->wValue.bytes[0])
                {
                    case 1:
                        {
                            usbMsgPtr = (usbMsgPtr_t) loader_string_vendor_descriptor;
                            return LOADER_STRING_VENDOR_DESCRIPTOR_LEN;
                        }
                    case 2:
                        {
                            usbMsgPtr = (usbMsgPtr_t) loader_string_product_descriptor;
                            return LOADER_STRING_PRODUCT_DESCRIPTOR_LEN;
                        }
                }
            }
    }
    return 0;
}

usbMsgLen_t loader_usb_function_setup(uchar data[8])
{
    usbRequest_t* rq = (void*) data;

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS)
    {
        if (rq->bRequest == USBRQ_HID_GET_REPORT)
        {
            return USB_NO_MSG;
        }
        else if (rq->bRequest == USBRQ_HID_SET_REPORT)
        {
            return USB_NO_MSG;
        }
    }
    return 0;
}

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    if (mode)
    {
        return keyboard_usb_function_setup(data);
    }
    else
    {
        return loader_usb_function_setup(data);
    }
}

usbMsgLen_t usbFunctionDescriptor(usbRequest_t* rq)
{
    if (mode)
    {
        return keyboard_usb_function_descriptor(rq);
    }
    else
    {
        return loader_usb_function_descriptor(rq);
    }
}

usbMsgLen_t usbFunctionWrite(uint8_t data[], uchar len)
{
    if (mode == MODE_LOADER)
    {
        for (uint8_t i = 0; i < len; ++i)
        {
            if (eeprom_write_pos == 512)
            {
                eeprom_write_pos = 0;
                return 1;
            }
            eeprom_update_byte((uint8_t*) 0 + eeprom_write_pos, data[i]);
            if (data[i] == 0xff)
            {
                eeprom_write_pos = 0;
                return 1;
            }
            ++eeprom_write_pos;

        }
        return 0;
    }
    return 1;
}

usbMsgLen_t usbFunctionRead(uchar *data, uchar len)
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

static inline uint8_t check_mode()
{
    return bit_is_set(PINC, PC1) ? MODE_KEYBOARD : MODE_LOADER;
}

int NO_RETURN main()
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
