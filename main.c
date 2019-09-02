#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "usbdrv.h"
#include "usb_hid_keys.h"
#include "keyboard_descriptors.h"
#include "loader_descriptors.h"

#define NO_RETURN __attribute__((noreturn))
#define MODE_LOADER 0
#define MODE_KEYBOARD 0xff
#define PAYLOAD_BUFFER_LEN 512

static keyboard_report_t keyboard_report;
static uint8_t idle_rate;
static uint8_t mode;
static uint16_t current_eeprom_address;
static uint16_t payload_pos;
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

void keyboard_interrupter()
{
    if (usbInterruptIsReady())
    {
        uint8_t i = 0;
        while (i < 6)
        {
            switch (payload_buffer[payload_pos])
            {
                case '\0':
                {
                    return;
                }
                case 1:
                {
                    ++payload_pos;
                    keyboard_report.modifier = payload_buffer[payload_pos];
                    ++payload_pos;
                    keyboard_report.key_code[i] = payload_buffer[payload_pos];
                }
                case 2:
                {
                    ++payload_pos;
                    const uint8_t delay = payload_buffer[payload_pos];
                    usbSetInterrupt((void*) &keyboard_report, sizeof(keyboard_report));
                    // TODO
                }
                default:
                {
                    keyboard_report.key_code[i]
                }
            }
            ++i;
        }
        usbSetInterrupt((void*) &keyboard_report, sizeof(keyboard_report));
    }
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

usbMsgLen_t usbFunctionWrite(uint8_t* data, uchar len)
{
    if (mode == MODE_LOADER)
    {
        uint8_t was_reset = 0;
        if (len && data[0] == '\0')
        {
            current_eeprom_address = 0;
            was_reset = 1;
        }
        if (current_eeprom_address + len > 511)
        {
            len = 511 - current_eeprom_address;
        }
        eeprom_write_block(data + was_reset,
                           (uint8_t*) 0 + current_eeprom_address,
                           len);
        current_eeprom_address += len;  //BABOL
    }
    return 1;
}

usbMsgLen_t usbFunctionRead(uchar *data, uchar len)
{
    return 1;
}

void usb_device_restart()
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

uint8_t check_mode()
{
     return bit_is_set(PINC, PC1) ? MODE_KEYBOARD : MODE_LOADER;
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
        if (mode)
        {
            keyboard_interrupter();
        }
    }
}