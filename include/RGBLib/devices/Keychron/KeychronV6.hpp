#ifndef __KEYCHRON_V6_HPP__
#define __KEYCHRON_V6_HPP__

#include "../keyboard.hpp"
#include "../../util/bytes.hpp"

#include <hidapi/hidapi.h>
#include <cmath>

#include "robin_hood.h"

const size_t KeychronV6PayloadLength = 32;

const std::vector<std::vector<uint8_t>> KeychronV6LEDS = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 },
    { 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40 },
    { 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60 },
    { 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77 },
    { 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93 },
    { 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107 },
};

enum KeychronV6PacketCommands {
    id_get_protocol_version                 = 0x01, // always 0x01
    id_get_keyboard_value                   = 0x02,
    id_set_keyboard_value                   = 0x03,
    id_dynamic_keymap_get_keycode           = 0x04,
    id_dynamic_keymap_set_keycode           = 0x05,
    id_dynamic_keymap_reset                 = 0x06,
    id_custom_set_value                     = 0x07,
    id_custom_get_value                     = 0x08,
    id_custom_save                          = 0x09,
    id_eeprom_reset                         = 0x0A,
    id_bootloader_jump                      = 0x0B,
    id_dynamic_keymap_macro_get_count       = 0x0C,
    id_dynamic_keymap_macro_get_buffer_size = 0x0D,
    id_dynamic_keymap_macro_get_buffer      = 0x0E,
    id_dynamic_keymap_macro_set_buffer      = 0x0F,
    id_dynamic_keymap_macro_reset           = 0x10,
    id_dynamic_keymap_get_layer_count       = 0x11,
    id_dynamic_keymap_get_buffer            = 0x12,
    id_dynamic_keymap_set_buffer            = 0x13,
    id_dynamic_keymap_get_encoder           = 0x14,
    id_dynamic_keymap_set_encoder           = 0x15,
    id_unhandled                            = 0xFF,
};

enum KeychronV6PacketChannels {
    id_custom_channel            = 0,
    id_qmk_backlight_channel     = 1,
    id_qmk_rgblight_channel      = 2,
    id_qmk_rgb_matrix_channel    = 3,
    id_qmk_audio_channel         = 4,
    id_custom_set_effect_channel = 5,
    id_custom_array_led_channel  = 6,
    id_custom_single_led_channel = 7
};

class KeychronV6 : public Keyboard {
private:
    robin_hood::unordered_flat_map<uint8_t, RGB> framebuffer;
    robin_hood::unordered_flat_map<uint8_t, RGB> emptyFramebuffer;

public:
    KeychronV6() : Keyboard(0x3434, 0x0361, 0xFF60, 0x61, KeychronV6LEDS, [this]() -> void {
        this->set_effect();
    }) {
        emptyFramebuffer = {};
        for(std::vector<uint8_t> col : leds) {
            for(uint8_t index : col) {
                emptyFramebuffer[index] = { 0x00, 0x00, 0x00 };
            }
        }

        framebuffer = emptyFramebuffer;
    }

    void set_led(uint8_t led, RGB rgb) {
        framebuffer[led] = rgb;
    }

    void set_effect() {
        if(!device) return;

        uint8_t payload[KeychronV6PayloadLength];
        std::memset(payload, 0x00, KeychronV6PayloadLength);

        payload[0] = 0x00;
        payload[1] = KeychronV6PacketCommands::id_custom_set_value;
        payload[2] = KeychronV6PacketChannels::id_custom_set_effect_channel;

        hid_write(device, payload, KeychronV6PayloadLength * sizeof(uint8_t));
    }

    void draw_frame() {
        if(!device) return;
        
        const size_t leds = framebuffer.size();

        size_t unformattedPayloadLength = (leds * (sizeof(RGB) + 1));
        uint8_t unformattedPayload[leds * (sizeof(RGB) + 1)];

        size_t i = 0;

        for(robin_hood::pair<uint8_t, RGB> pair : framebuffer) {
            unformattedPayload[i] = pair.first;
            unformattedPayload[i + 1] = pair.second.red;
            unformattedPayload[i + 2] = pair.second.green;
            unformattedPayload[i + 3] = pair.second.blue;

            i += 4;
        }

        size_t totalPayloads = std::ceil((double)(unformattedPayloadLength + 4) / (double)(KeychronV6PayloadLength));
        long long bytesLeft = (long long)unformattedPayloadLength;
            
        for(size_t i = 0; i < totalPayloads; i++) {
            uint8_t payload[KeychronV6PayloadLength];
            std::memset(payload, 0x00, KeychronV6PayloadLength);

            payload[0] = 0x00;
            payload[1] = KeychronV6PacketCommands::id_custom_set_value;
            payload[2] = KeychronV6PacketChannels::id_custom_array_led_channel;

            if(bytesLeft < KeychronV6PayloadLength - 3) {
                payload[bytesLeft + 3] = 0xFF;
            }
            else {
                payload[KeychronV6PayloadLength - 1] = 0xFF;
            }

            for(size_t j = 3, k = 0; j < KeychronV6PayloadLength - 1 && bytesLeft - 1 > 0; j += 4) {
                uint8_t index = unformattedPayload[unformattedPayloadLength - bytesLeft];
                uint8_t* rgb = &(unformattedPayload[(unformattedPayloadLength - bytesLeft) + 1]);

                payload[j] = index;
                payload[j + 1] = rgb[0];
                payload[j + 2] = rgb[1];
                payload[j + 3] = rgb[2]; 

                bytesLeft -= 4;
                k += 4;
            }

            hid_write(device, payload, KeychronV6PayloadLength * sizeof(uint8_t));
        }

        framebuffer = emptyFramebuffer;
    }
};

#endif