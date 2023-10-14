#ifndef __KEYCHRON_V6_HPP__
#define __KEYCHRON_V6_HPP__

#include "../keyboard.hpp"
#include "../../util/bytes.hpp"

#include <hidapi/hidapi.h>
#include <cmath>

#include <map>

const size_t KeychronV6PayloadLength = 32;
const size_t KeychronV6TotalLEDs = 108;

const std::vector<std::vector<uint8_t>> KeychronV6LEDS = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

const uint8_t KeychronV6Cols = 22;
const uint8_t KeychronV6Rows = 6;

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
    id_custom_single_led_channel = 7,
    id_custom_single_col_channel = 8,
    id_custom_array_col_channel = 9,
};

class KeychronV6 : public Keyboard {
private:
    std::map<uint8_t, RGB> framebuffer;
    std::map<uint8_t, RGB> emptyFramebuffer;

public:
    KeychronV6() : Keyboard(0x3434, 0x0361, 0xFF60, 0x61, KeychronV6LEDS, [this]() -> void {
        this->set_effect();
    }) {
        emptyFramebuffer = {};
        for(uint8_t col = 0; col < KeychronV6Cols; col++) {
            emptyFramebuffer[col] = { 0x00, 0x00, 0x00 };
        }

        framebuffer = emptyFramebuffer;
    }

    virtual ~KeychronV6() {}


    uint8_t getCols() { return KeychronV6Cols; }
    uint8_t getRows() { return KeychronV6Rows; }

    void set_col(uint8_t col, RGB rgb) {
        if(col >= KeychronV6Cols) return;

        framebuffer[col] = rgb;
    }

    void set_led(uint8_t col, RGB rgb) {
        if(col >= KeychronV6Cols) return;

        framebuffer[col] = rgb;
    }

    void set_effect() {
        if(!device) return;

        deviceMutex.lock();

        uint8_t payload[KeychronV6PayloadLength + 1];
        std::memset(payload, 0x00, KeychronV6PayloadLength + 1);

        payload[1] = KeychronV6PacketCommands::id_custom_set_value;
        payload[2] = KeychronV6PacketChannels::id_custom_set_effect_channel;

        hid_write(device, payload, (KeychronV6PayloadLength + 1) * sizeof(uint8_t));

        deviceMutex.unlock();
    }

    void draw_frame() {
        if(!device) return;
        if(!deviceMutex.try_lock()) return;

        const size_t cols = framebuffer.size();

        size_t unformattedPayloadLength = (cols * (sizeof(RGB) + 1));
        uint8_t unformattedPayload[unformattedPayloadLength];

        size_t i = 0;

        for(std::pair<uint8_t, RGB> pair : framebuffer) {
            unformattedPayload[i++] = pair.first;
            unformattedPayload[i++] = pair.second.red;
            unformattedPayload[i++] = pair.second.green;
            unformattedPayload[i++] = pair.second.blue;
        }

        size_t totalPayloads = std::ceil((double)(unformattedPayloadLength) / (double)(KeychronV6PayloadLength - 4));
        long long bytesLeft = (long long)unformattedPayloadLength;

        for(size_t i = 0; i < totalPayloads; i++) {
            uint8_t payload[KeychronV6PayloadLength + 1];
            std::memset(payload, 0x00, KeychronV6PayloadLength + 1);

            payload[1] = KeychronV6PacketCommands::id_custom_set_value;
            payload[2] = KeychronV6PacketChannels::id_custom_array_col_channel;

            if((size_t)bytesLeft < KeychronV6PayloadLength - 3) {
                payload[bytesLeft + 3] = 0xFF;
            }
            else {
                payload[KeychronV6PayloadLength - 1] = 0xFF;
            }

            for(size_t j = 3, k = unformattedPayloadLength - bytesLeft;
                    j < KeychronV6PayloadLength - 1 && bytesLeft > 0;
                    bytesLeft -= 4, k = unformattedPayloadLength - bytesLeft) {

                uint8_t index = unformattedPayload[k];
                uint8_t* rgb = &(unformattedPayload[k + 1]);

                payload[j++] = index;
                payload[j++] = rgb[0];
                payload[j++] = rgb[1];
                payload[j++] = rgb[2];
            }

            if(hid_write(device, payload, (KeychronV6PayloadLength + 1) * sizeof(uint8_t)) == -1) {
                return;
            }
        }

        framebuffer = emptyFramebuffer;

        deviceMutex.unlock();
    }
};

#endif
