#include "loader_device.h"

PROGMEM
const uint8_t loader_hid_report_descriptor[LOADER_HID_REPORT_DESCRIPTOR_LEN] = {
        0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
        0x09, 0x01,                    // USAGE (Vendor Usage 1)
        0xa1, 0x01,                    // COLLECTION (Application)
        0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
        0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
        0x75, 0x08,                    //   REPORT_SIZE (8)
        0x95, 0x80,                    //   REPORT_COUNT (128)
        0x09, 0x00,                    //   USAGE (Undefined)
        0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
        0xc0                           // END_COLLECTION
};

PROGMEM
static const uint8_t loader_device_descriptor[LOADER_DEVICE_DESCRIPTOR_LEN] = {
        LOADER_DEVICE_DESCRIPTOR_LEN,             /* Length */
        USBDESCR_DEVICE,                          /* Descriptor Type: Device */
        0x10, 0x01,                               /* USB specification version */
        0x00,                                     /* Class */
        0x00,                                     /* Subclass */
        0x00,                                     /* Protocol */
        0x08,                                     /* Max packet size */
        0xc0, 0x16,                               /* Vendor id */
        0xdf, 0x05,                               /* Product id */
        0x00, 0x01,                               /* Device release number */
        0x01,                                     /* Manufacturer string descriptor index */
        0x02,                                     /* Product string descriptor index */
        0x00,                                     /* Serial nb. string descriptor index */
        0x01                                      /* Nb. of configurations */
};

PROGMEM
static const int loader_string_vendor_descriptor[] = {
        USB_STRING_DESCRIPTOR_HEADER(8),
        't', 'e', 'c', 'h', 'n', 'a', 't', 'e'
};

PROGMEM
static const int loader_string_product_descriptor[] = {
        USB_STRING_DESCRIPTOR_HEADER(11),
        'd', 'u', 'c', 'k', '-', 'l', 'o', 'a', 'd', 'e', 'r'
};

PROGMEM
static const uint8_t loader_config_descriptor[LOADER_CONFIG_TOTAL_LEN] = {
        LOADER_CONFIG_DESCRIPTOR_LEN,             /* Length */
        USBDESCR_CONFIG,                          /* Descriptor Type: Configuration */
        (LOADER_CONFIG_DESCRIPTOR_LEN +
         HID_DESCRIPTOR_LEN +
         LOADER_INTER_DESCRIPTOR_LEN +
         LOADER_ENDPOINT_DESCRIPTOR_LEN), 0x00,   /* Total length */
        0x01,                                     /* Nb of interfaces */
        0x01,                                     /* Configuration index */
        0x00,                                     /* Index of string descriptor for this configuration */
        0x80,                                     /* Configuration characteristics - bus powered */
        0x0a,                                     /* Max power XmA / 2*/
        LOADER_INTER_DESCRIPTOR_LEN,              /* Length */
        USBDESCR_INTERFACE,                       /* Descriptor type: Interface */
        0x00,                                     /* Index of interface */
        0x00,                                     /* Alternate setting */
        0x01,                                     /* Nb. of endpoints */
        0x03,                                     /* Class code - HID */
        0x00,                                     /* Subclass - no */
        0x00,                                     /* Protocol - no */
        0x00,                                     /* String interface */
        LOADER_ENDPOINT_DESCRIPTOR_LEN,           /* Length */
        USBDESCR_ENDPOINT,                        /* Descriptor type: Endpoint */
        0x81,                                     /* Address of endpoint */
        0x03,                                     /* Type of endpoint: Interrupt */
        0x08, 0x00,                               /* Maximum packet size */
        0x64,                                     /* Interval of poll */
        HID_DESCRIPTOR_LEN,                       /* Length */
        USBDESCR_HID,                             /* Descriptor type: HID */
        0x01, 0x01,                               /* HID class specification */
        0x00,                                     /* Country */
        0x01,                                     /* Nb. of HID's */
        0x22,                                     /* Report descriptor type */
        0x16, 0x00                                /* Total length of report HID */
};



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
        if (rq->bRequest == USBRQ_HID_GET_REPORT || rq->bRequest == USBRQ_HID_SET_REPORT)
        {
            return USB_NO_MSG;
        }
    }
    return 0;
}