#ifndef MEGA_DUCK_KEYBOARD_DESCRIPTORS_HPP
#define MEGA_DUCK_KEYBOARD_DESCRIPTORS_HPP

#include <avr/pgmspace.h>

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
static const uint8_t keyboard_hid_report_descriptor[KEYBOARD_HID_REPORT_DESCRIPTOR_LEN] = {
        0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
        0x09, 0x06,                    // USAGE (Keyboard)
        0xa1, 0x01,                    // COLLECTION (Application)
        0x75, 0x01,                    //   REPORT_SIZE (1)
        0x95, 0x08,                    //   REPORT_COUNT (8)
        0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
        0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)(224)
        0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
        0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
        0x81, 0x02,                    //   INPUT (Data,Var,Abs) ; Modifier byte
        0x95, 0x01,                    //   REPORT_COUNT (1)
        0x75, 0x08,                    //   REPORT_SIZE (8)
        0x81, 0x03,                    //   INPUT (Cnst,Var,Abs) ; Reserved byte
        0x95, 0x05,                    //   REPORT_COUNT (5)
        0x75, 0x01,                    //   REPORT_SIZE (1)
        0x05, 0x08,                    //   USAGE_PAGE (LEDs)
        0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
        0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
        0x91, 0x02,                    //   OUTPUT (Data,Var,Abs) ; LED report
        0x95, 0x01,                    //   REPORT_COUNT (1)
        0x75, 0x03,                    //   REPORT_SIZE (3)
        0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs) ; LED report padding
        0x95, 0x06,                    //   REPORT_COUNT (6)
        0x75, 0x08,                    //   REPORT_SIZE (8)
        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
        0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
        0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
        0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))(0)
        0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)(101)
        0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
        0xc0                           // END_COLLECTION
};

PROGMEM
static const uint8_t keyboard_device_descriptor[KEYBOARD_DEVICE_DESCRIPTOR_LEN] = {
        KEYBOARD_DEVICE_DESCRIPTOR_LEN,           /* Length */
        USBDESCR_DEVICE,                          /* Descriptor Type: Device */
        0x10, 0x01,                               /* USB specification version */
        0x00,                                     /* Class */
        0x00,                                     /* Subclass */
        0x00,                                     /* Protocol */
        0x08,                                     /* Max packet size */
        0xc0, 0x16,                               /* Vendor id */
        0xdb, 0x27,                               /* Product id */
        0x00, 0x01,                               /* Device release number */
        0x01,                                     /* Manufacturer string descriptor index */
        0x02,                                     /* Product string descriptor index */
        0x00,                                     /* Serial nb. string descriptor index */
        0x01                                      /* Nb. of configurations */
};

PROGMEM
static const int keyboard_string_vendor_descriptor[] = {
        USB_STRING_DESCRIPTOR_HEADER(8),
        't', 'e', 'c', 'h', 'n', 'a', 't', 'e'
};

PROGMEM
static const int keyboard_string_product_descriptor[] = {
        USB_STRING_DESCRIPTOR_HEADER(13),
        'd', 'u', 'c', 'k', '-', 'k', 'e', 'y', 'b', 'o', 'a', 'r', 'd'
};

PROGMEM
static const uint8_t keyboard_config_descriptor[KEYBOARD_CONFIG_TOTAL_LEN] = {
        KEYBOARD_CONFIG_DESCRIPTOR_LEN,           /* Length */
        USBDESCR_CONFIG,                          /* Descriptor Type: Configuration */
        (KEYBOARD_CONFIG_DESCRIPTOR_LEN +
         HID_DESCRIPTOR_LEN +
         KEYBOARD_INTER_DESCRIPTOR_LEN +
         KEYBOARD_ENDPOINT_DESCRIPTOR_LEN), 0x00, /* Total length */
        0x01,                                     /* Nb of interfaces */
        0x01,                                     /* Configuration index */
        0x00,                                     /* Index of string descriptor for this configuration */
        0x80,                                     /* Configuration characteristics - bus powered */
        0x0a,                                     /* Max power XmA / 2*/
        KEYBOARD_INTER_DESCRIPTOR_LEN,            /* Length */
        USBDESCR_INTERFACE,                       /* Descriptor type: Interface */
        0x00,                                     /* Number of interface */
        0x00,                                     /* Alternate setting */
        0x01,                                     /* Nb. of endpoints */
        0x03,                                     /* Class code - HID */
        0x01,                                     /* Subclass - boot */
        0x01,                                     /* Protocol - keyboard */
        0x00,                                     /* String interface */
        KEYBOARD_ENDPOINT_DESCRIPTOR_LEN,         /* Length */
        USBDESCR_ENDPOINT,                        /* Descriptor type: Endpoint */
        0x81,                                     /* Address of endpoint */
        0x03,                                     /* Type of endpoint: Interrupt */
        0x08, 0x00,                               /* Maximum packet size */
        0x0a,                                     /* Interval of poll */
        HID_DESCRIPTOR_LEN,                       /* Length */
        USBDESCR_HID,                             /* Descriptor type: HID */
        0x01, 0x01,                               /* HID class specification */
        0x00,                                     /* Country */
        0x01,                                     /* Nb. of HID's */
        0x22,                                     /* Report descriptor type */
        0x3f, 0x00                                /* Total length of report HID */
};

typedef struct
{
    uint8_t modifier;
    uint8_t reserved;
    uint8_t key_code[6];
} keyboard_report_t;


#endif //MEGA_DUCK_KEYBOARD_DESCRIPTORS_HPP
