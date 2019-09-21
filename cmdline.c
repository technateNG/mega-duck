#include <stdio.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

#define HID_REPORT_TYPE_FEATURE 3
#define HID_SET_REPORT 0x09

char buffer[512];

typedef unsigned char uchar;

size_t find(uchar buffer[static 1], size_t size, uchar val)
{
    for (size_t i = 0; i < size; ++i)
    {
        if (buffer[i] == val)
        {
            return i;   
        }
    }
}

int main(void)
{
    libusb_device** list;
    libusb_device* device;
    libusb_device_handle* handle;
    puts("[*] USB initialization.");
    int err = libusb_init(NULL);
    if (err)
    {
        fprintf(stderr, "[!] Error with initialization: %s\n", libusb_strerror(err));
        return 1;
    }
    puts("[*] Enumerating devices.");
    size_t num_of_devices = (size_t) libusb_get_device_list(NULL, &list);
    struct libusb_device_descriptor descriptor;
    for (size_t i = 0; i < num_of_devices; ++i) 
    {
        device = list[i];
        libusb_get_device_descriptor(device, &descriptor);
        if (descriptor.idVendor == 0x16c0 && descriptor.idProduct == 0x05df)
        {
            err = libusb_open(device, &handle);
            if (err)
            {
                fprintf(stderr, "[!] Error with opening device: %s\n", libusb_strerror(err));
                return 1;
            }
            libusb_get_string_descriptor_ascii(handle, 1, (uchar*) buffer, 9);
            if (!strcmp(buffer, "technate"))
            {
                libusb_get_string_descriptor_ascii(handle, 2, (uchar*) buffer, 12);
                if (!strcmp(buffer, "duck-loader"))
                {
                    break;
                }
            }
        }
        device = NULL;
        handle = NULL;
    }
    if (!device)
    {
        fprintf(stderr, "[!] Duck-loader not found.\n");
        return 1;
    }
    puts("[*] Duck loader found. Waiting for payload.");
    memset(buffer, 0xff, sizeof(buffer));
    fgets(buffer, 512, stdin);
    size_t msg_len = find(buffer, 512, 0xff);
    puts("[*] Payload received. Parsing...");
    puts("[*] Payload parsed. Sending transfer.");
    libusb_control_transfer(
            handle, 
            LIBUSB_REQUEST_TYPE_CLASS | 
            LIBUSB_RECIPIENT_DEVICE | 
            LIBUSB_ENDPOINT_OUT,
            HID_SET_REPORT, 
	    HID_REPORT_TYPE_FEATURE << 8,
	    0,
	    buffer,
	    msg_len + 1,
	    5000);
    puts("[*] Payload succesfully written to device.");
    libusb_free_device_list(list, 1);
    libusb_exit(NULL);
    return 0;
}
