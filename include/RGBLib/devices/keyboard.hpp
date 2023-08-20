#ifndef __KEYBOARD_HPP__
#define __KEYBOARD_HPP__

#include "../util/rgb.hpp"
#include "./device.hpp"

#include <vector>

class Keyboard : public Device {
public:
    Keyboard(unsigned int VENDOR_ID, unsigned int PRODUCT_ID, unsigned int usage_page, unsigned int usage, std::vector<std::vector<uint8_t>> leds, std::function<void()> onDeviceConnect) : 
        Device(VENDOR_ID, PRODUCT_ID, usage_page, usage, leds, onDeviceConnect) {}

    virtual void draw_frame() = 0;
};

#endif