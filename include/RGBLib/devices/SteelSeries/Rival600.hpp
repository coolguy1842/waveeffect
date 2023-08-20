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

        unsigned char header[HEADER_LENGTH];
        for(int i = 0; i < HEADER_LENGTH; i++) {
            header[i] = 0.0f;
        } 
        
        header[REPEAT_INDEX] = 0x01; 
        header[COLOURS_COUNT_INDEX] = 0x01; // gradient count(1)

        // led id indices
        header[0] = led; 
        header[5] = led;

        unsigned char* bytearray = uint_to_little_endian_bytearray(DEFAULT_DURATION, DURATION_LENGTH);
        
        for(int i = 0; i < DURATION_LENGTH; i++) {
            header[DURATION_INDEX + i] = bytearray[i];
        } 

        free(bytearray);

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

        unsigned char* data;
        unsigned char* payload;
        int dataLen;
        int payloadLen;

        merge_bytes(header, HEADER_LENGTH, body, BODY_LENGTH, &data, &dataLen);

        int reportLen = 3;
        unsigned char* report = (unsigned char*)malloc(reportLen * sizeof(unsigned char));
        report[0] = 0x00; // reportid
        report[1] = RIVAL600COMMANDS::SET_LED_COLOR; // command
        report[2] = 0x00; // command

        merge_bytes(report, reportLen, data, dataLen, &payload, &payloadLen);
        free(data);
        free(report);

        hid_send_feature_report(device, payload, payloadLen * sizeof(unsigned char));
        free(payload);
    }
};

#endif