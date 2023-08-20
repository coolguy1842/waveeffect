#ifndef __MOUSE_HPP__
#define __MOUSE_HPP__

#include "../util/rgb.hpp"

#include "./device.hpp"

class Mouse : public Device {
public:
    Mouse(unsigned int vendor_id, unsigned int product_id, unsigned int usage_page, unsigned int usage, std::vector<std::vector<uint8_t>> leds, std::function<void()> onDeviceConnect) :
        Device(vendor_id, product_id, usage_page, usage, leds, onDeviceConnect) {
        
    }

    virtual void set_led(uint8_t led, RGB rgb) = 0;
};

#endif