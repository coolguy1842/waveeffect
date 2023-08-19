#ifndef __RGBLIB_DEVICE_HPP__
#define __RGBLIB_DEVICE_HPP__

#include "../util/rgb.hpp"

#include <vector>
#include <stdint.h>


class Device {
protected:
    virtual void initDevice() = 0;

public:
    const std::vector<std::vector<uint8_t>> leds;

    const unsigned int VENDOR_ID;
    const unsigned int PRODUCT_ID;
    
    const unsigned int USAGE_PAGE;
    const unsigned int USAGE;


    Device(unsigned int VENDOR_ID, unsigned int PRODUCT_ID, unsigned int usage_page, unsigned int usage, std::vector<std::vector<uint8_t>> leds) : 
        VENDOR_ID(VENDOR_ID), PRODUCT_ID(PRODUCT_ID), USAGE_PAGE(usage_page), USAGE(usage), leds(leds) {
        
    }

    virtual void set_led(unsigned char led, RGB rgb) = 0;  
};

#endif