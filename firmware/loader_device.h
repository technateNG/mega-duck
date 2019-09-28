#ifndef MEGA_DUCK_LOADER_DEVICE_H
#define MEGA_DUCK_LOADER_DEVICE_H

#include <avr/pgmspace.h>
#include "usbdrv.h"

#define LOADER_HID_REPORT_DESCRIPTOR_LEN 22
#define LOADER_INTER_DESCRIPTOR_LEN 9
#define LOADER_ENDPOINT_DESCRIPTOR_LEN 7
#define LOADER_CONFIG_DESCRIPTOR_LEN 9
#define LOADER_CONFIG_TOTAL_LEN 34
#define LOADER_DEVICE_DESCRIPTOR_LEN 18
#define HID_DESCRIPTOR_LEN 9
#define LOADER_STRING_VENDOR_DESCRIPTOR_LEN sizeof(loader_string_vendor_descriptor)
#define LOADER_STRING_PRODUCT_DESCRIPTOR_LEN sizeof(loader_string_product_descriptor)

PROGMEM
const uint8_t loader_hid_report_descriptor[LOADER_HID_REPORT_DESCRIPTOR_LEN];

PROGMEM
static const uint8_t loader_device_descriptor[LOADER_DEVICE_DESCRIPTOR_LEN];

PROGMEM
static const int loader_string_vendor_descriptor[9];

PROGMEM
static const int loader_string_product_descriptor[12];

PROGMEM
static const uint8_t loader_config_descriptor[LOADER_CONFIG_TOTAL_LEN];

usbMsgLen_t loader_usb_function_descriptor(usbRequest_t* rq);
usbMsgLen_t loader_usb_function_setup(uchar data[8]);

#endif //MEGA_DUCK_LOADER_DEVICE_H
