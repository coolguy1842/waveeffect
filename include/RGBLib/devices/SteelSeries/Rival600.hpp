#ifndef __RIVAL600_HPP__
#define __RIVAL600_HPP__

#include "../mouse.hpp"
#include "../../util/bytes.hpp"

#include <hidapi/hidapi.h>

const std::vector<std::vector<uint8_t>> Rival600LEDS = {
    { 0x02, 0x00, 0x03 },
    { 0x04, 0x05, },
    { 0x06, 0x01, 0x07 }
};

enum RIVAL600COMMANDS {
    SET_LED_COLOR = 0x05,
    SAVE = 0x09
};


class Rival600 : public Mouse {
public:
    Rival600() : Mouse(0x1038, 0x1724, 0x00, 0x00, Rival600LEDS, [this]() -> void {}) {}
    virtual ~Rival600() {}

    void set_led(uint8_t led, RGB rgb) {
        if(!device) return;

#define HEADER_LENGTH 28
#define BODY_LENGTH 7
#define REPEAT_INDEX 22
#define TRIGGERS_INDEX 23
#define COLOURS_COUNT_INDEX 27
#define DURATION_INDEX 6
#define DURATION_LENGTH 2
#define DEFAULT_DURATION 1000
#define REPORT_LENGTH 3

        unsigned char header[HEADER_LENGTH];
        for(int i = 0; i < HEADER_LENGTH; i++) {
            header[i] = 0.0f;
        }

        header[REPEAT_INDEX] = 0x01;
        header[COLOURS_COUNT_INDEX] = 0x01; // gradient count(1)

        // led id indices
        header[0] = led;
        header[5] = led;

        // duration must be in little endian so swap bytes
        header[DURATION_INDEX] = DEFAULT_DURATION >> 8;
        header[DURATION_INDEX + 1] = DEFAULT_DURATION & 0xFF;


        unsigned char body[BODY_LENGTH]; // rgb rgb offset

        // rgb
        body[0] = rgb.red;
        body[1] = rgb.green;
        body[2] = rgb.blue;
        // rgb
        body[3] = rgb.red;
        body[4] = rgb.green;
        body[5] = rgb.blue;
        // offset
        body[6] = 0;

        unsigned char data[HEADER_LENGTH + BODY_LENGTH];
        size_t dataLen = merge_bytes(header, HEADER_LENGTH, body, BODY_LENGTH, data);

        unsigned char report[REPORT_LENGTH];
        report[0] = 0x00; // reportid
        report[1] = RIVAL600COMMANDS::SET_LED_COLOR; // command
        report[2] = 0x00; // command

        unsigned char payload[REPORT_LENGTH + dataLen];
        size_t payloadLen = merge_bytes(report, REPORT_LENGTH, data, dataLen, payload);

        if(!deviceMutex.try_lock()) return;

        hid_send_feature_report(device, payload, payloadLen * sizeof(unsigned char));

        deviceMutex.unlock();
    }
};

#endif
