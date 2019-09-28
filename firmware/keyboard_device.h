#ifndef MEGA_DUCK_KEYBOARD_DESCRIPTORS_HPP
#define MEGA_DUCK_KEYBOARD_DESCRIPTORS_HPP

#include <avr/pgmspace.h>
#include "usbdrv.h"

#define KEYBOARD_HID_REPORT_DESCRIPTOR_LEN 63
#define KEYBOARD_INTER_DESCRIPTOR_LEN 9
#define KEYBOARD_ENDPOINT_DESCRIPTOR_LEN 7
#define KEYBOARD_CONFIG_DESCRIPTOR_LEN 9
#define KEYBOARD_CONFIG_TOTAL_LEN 34
#define KEYBOARD_DEVICE_DESCRIPTOR_LEN 18
#define HID_DESCRIPTOR_LEN 9
#define KEYBOARD_STRING_VENDOR_DESCRIPTOR_LEN sizeof(keyboard_string_vendor_descriptor)
#define KEYBOARD_STRING_PRODUCT_DESCRIPTOR_LEN sizeof(keyboard_string_product_descriptor)

PROGMEM
static const uint8_t keyboard_hid_report_descriptor[KEYBOARD_HID_REPORT_DESCRIPTOR_LEN];

PROGMEM
static const uint8_t keyboard_device_descriptor[KEYBOARD_DEVICE_DESCRIPTOR_LEN];

PROGMEM
static const int keyboard_string_vendor_descriptor[9];

PROGMEM
static const int keyboard_string_product_descriptor[14];

PROGMEM
static const uint8_t keyboard_config_descriptor[KEYBOARD_CONFIG_TOTAL_LEN];

typedef struct
{
    uint8_t modifier;
    uint8_t reserved;
    uint8_t key_code[6];
} keyboard_report_t;

usbMsgLen_t keyboard_usb_function_descriptor(usbRequest_t* rq);
usbMsgLen_t keyboard_usb_function_setup(uchar data[8]);

static uint8_t idle_rate;
static keyboard_report_t keyboard_report;
#endif //MEGA_DUCK_KEYBOARD_DESCRIPTORS_HPP
