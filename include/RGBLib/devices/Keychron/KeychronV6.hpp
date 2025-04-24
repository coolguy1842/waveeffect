#ifndef __KEYCHRON_V6_HPP__
#define __KEYCHRON_V6_HPP__

#include "../keyboard.hpp"
#include "../../util/bytes.hpp"

#include <hidapi/hidapi.h>
#include <cmath>

#include <algorithm>

#include <map>
#include <list>
#include <vector>

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
    id_custom_array_col_channel  = 9,
    id_custom_draw_channel       = 10,
};

class KeychronV6 : public Keyboard {
public:
    std::map<uint8_t, time_t> keypressStartTimes;

private:
    std::list<uint16_t> SCAN_TO_KEY = {
        // 0xFF is ignored
        { KEY_ESC },  { KEY_F1 }, { KEY_F2 }, { KEY_F3 }, { KEY_F4 },  { KEY_F5 }, { KEY_F6 }, { KEY_F7 }, { KEY_F8 },  { KEY_F9 }, { KEY_F10 }, { KEY_F11 }, { KEY_F12 },    { KEY_PRINT }, { KEY_SCROLLLOCK }, { 0xFF },    { 0xFF }, { 0xFF }, { 0xFF }, { 0xFF },
        { KEY_GRAVE }, { KEY_1 }, { KEY_2 }, { KEY_3 }, { KEY_4 }, { KEY_5 }, { KEY_6 }, { KEY_7 }, { KEY_8 }, { KEY_9 }, { KEY_0 }, { KEY_MINUS }, { KEY_EQUAL }, { KEY_BACKSPACE },   { KEY_INSERT }, { KEY_HOME }, { KEY_PAGEUP },   { KEY_NUMLOCK }, { KEY_KPSLASH }, { KEY_KPASTERISK }, { KEY_KPMINUS },
        { KEY_TAB }, { KEY_Q }, { KEY_W }, { KEY_E }, { KEY_R }, { KEY_T }, { KEY_Y }, { KEY_U }, { KEY_I }, { KEY_O }, { KEY_P }, { KEY_LEFTBRACE }, { KEY_RIGHTBRACE }, { KEY_BACKSLASH }, { KEY_DELETE }, { KEY_END }, { KEY_PAGEDOWN }, { KEY_KP7 }, { KEY_KP8 }, { KEY_KP9 },
        { KEY_CAPSLOCK }, { KEY_A }, { KEY_S }, { KEY_D }, { KEY_F }, { KEY_G }, { KEY_H }, { KEY_J }, { KEY_K }, { KEY_L }, { KEY_SEMICOLON }, { KEY_APOSTROPHE }, { KEY_ENTER }, { KEY_KP4 }, { KEY_KP5 }, { KEY_KP6 }, { KEY_KPPLUS },
        { KEY_LEFTSHIFT }, { KEY_Z }, { KEY_X }, { KEY_C }, { KEY_V }, { KEY_B }, { KEY_N }, { KEY_M }, { KEY_COMMA }, { KEY_DOT }, { KEY_SLASH }, { KEY_RIGHTSHIFT }, { KEY_UP }, { KEY_KP1 }, { KEY_KP2 }, { KEY_KP3 },
        { KEY_LEFTCTRL }, { KEY_LEFTMETA }, { KEY_LEFTALT }, { KEY_SPACE }, { KEY_RIGHTALT }, { KEY_RIGHTMETA }, { 0xFF }, { KEY_RIGHTCTRL }, { KEY_LEFT }, { KEY_DOWN }, { KEY_RIGHT }, { KEY_KP0 }, { KEY_KPDOT }, { KEY_KPENTER }
    };

    const uint8_t DRAW_PACKET[KeychronV6PayloadLength + 1] = { 0x00, id_custom_set_value, id_custom_draw_channel };
    std::map<uint8_t, RGB> framebuffer;
    std::map<uint8_t, RGB> emptyFramebuffer;

    std::map<uint8_t, uint8_t> dimmedKeysValues;
    std::map<uint8_t, RGB> dimmedKeysRGB;


    void onDeviceEvent(struct libevdev* device, struct input_event* event) {
        // printf(
        //     "Event: %s %s %d, %u\n",
        //     libevdev_event_type_get_name(event->type),
        //     libevdev_event_code_get_name(event->type, event->code),
        //     event->value,
        //     event->code
        // );

        if(event->type != EV_KEY) return;

        switch(event->type) {
        case EV_KEY: {
            switch(event->value) {
            case 1: {
                keypressStartTimes[event->code] = time(NULL);
            }
            case 2: {
                auto it = std::find(SCAN_TO_KEY.begin(), SCAN_TO_KEY.end(), event->code);
                if(it == SCAN_TO_KEY.end()) break;

                size_t idx = std::distance(SCAN_TO_KEY.begin(), it);

                dimmedKeysValues[idx] = 10;
                break;
            }
            case 0:
                auto it = keypressStartTimes.find(event->code);
                if(it == keypressStartTimes.end()) break;

                keypressStartTimes.erase(it);
                break;
            }

            break;
        }
        default: break;
        }
    }

public:
    KeychronV6() : Keyboard(0x3434, 0x0361, 0xFF60, 0x0061, KeychronV6LEDS, [this]() -> void {
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


    std::vector<uint8_t> getUnformattedPayload(std::map<uint8_t, RGB> buffer) {
        size_t unformattedPayloadLength = (buffer.size() * (sizeof(RGB) + 1));
        std::vector<uint8_t> unformattedPayload(unformattedPayloadLength);

        size_t i = 0;
        for(std::pair<uint8_t, RGB> pair : buffer) {
            unformattedPayload[i++] = pair.first;
            unformattedPayload[i++] = pair.second.red;
            unformattedPayload[i++] = pair.second.green;
            unformattedPayload[i++] = pair.second.blue;
        }

        return unformattedPayload;
    }

    std::vector<std::vector<uint8_t>> getPayloads(std::vector<uint8_t> unformattedPayload, uint8_t command, uint8_t channel) {
        size_t totalPayloads = std::ceil((double)(unformattedPayload.size()) / (double)(KeychronV6PayloadLength - 4));
        std::vector<std::vector<uint8_t>> payloads(totalPayloads);

        long long bytesLeft = (long long)unformattedPayload.size();

        for(size_t i = 0; i < totalPayloads; i++) {
            if((size_t)bytesLeft < KeychronV6PayloadLength - 3) {
                payloads[i] = std::vector<uint8_t>(bytesLeft + 4, 0x00);
            }
            else {
                payloads[i] = std::vector<uint8_t>(KeychronV6PayloadLength, 0x00);
            }

            std::vector<uint8_t>& payload = payloads[i];
            payload[0] = 0x00;
            payload[1] = command;
            payload[2] = channel;

            payload[payload.size() - 1] = 0xFF;

            for(size_t j = 3, k = unformattedPayload.size() - bytesLeft;
                    j < KeychronV6PayloadLength - 1 && bytesLeft > 0;
                    bytesLeft -= 4, k = unformattedPayload.size() - bytesLeft) {

                uint8_t index = unformattedPayload[k];
                uint8_t* rgb = &(unformattedPayload[k + 1]);

                payload[j++] = index;
                payload[j++] = rgb[0];
                payload[j++] = rgb[1];
                payload[j++] = rgb[2];
            }
        }

        return payloads;
    }



    void loadDimmedKeys() {
        dimmedKeysRGB = {};
        for(auto it = dimmedKeysValues.begin(); it != dimmedKeysValues.end();) {
            if(custom_leds.find(it->first) != custom_leds.end()) {
                it++;
                continue;
            }

            RGB rgb = framebuffer[it->first % framebuffer.size()];
            dimmedKeysRGB[it->first] = {
                (uint8_t)std::max(rgb.red / it->second, 0),
                (uint8_t)std::max(rgb.green / it->second, 0),
                (uint8_t)std::max(rgb.blue / it->second, 0)
            };

            it->second = std::max(it->second - 2, 0);
            if(it->second <= 0) {
                it = dimmedKeysValues.erase(it);
                continue;
            }

            it++;
        }
    }

    void draw_frame() {
        if(!device) return;
        if(!deviceMutex.try_lock()) return;

        std::vector<uint8_t> unformattedPayload = getUnformattedPayload(framebuffer);
        for(std::vector<uint8_t> payload : getPayloads(unformattedPayload, id_custom_set_value, id_custom_array_col_channel)) {
            if(hid_write(device, payload.data(), payload.size() * sizeof(uint8_t)) == -1) {
                deviceMutex.unlock();
                return;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        loadDimmedKeys();
        unformattedPayload = getUnformattedPayload(custom_leds);

        for(uint8_t data : getUnformattedPayload(dimmedKeysRGB)) {
            unformattedPayload.push_back(data);
        }

        for(std::vector<uint8_t> payload : getPayloads(unformattedPayload, id_custom_set_value, id_custom_array_led_channel)) {
            if(hid_write(device, payload.data(), payload.size() * sizeof(uint8_t)) == -1) {
                deviceMutex.unlock();
                return;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        hid_write(device, DRAW_PACKET, (KeychronV6PayloadLength + 1) * sizeof(uint8_t));

        framebuffer = emptyFramebuffer;

        deviceMutex.unlock();
    }
};

#endif
