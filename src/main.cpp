#include <stdio.h>
#include <RGBLib/devices/SteelSeries/Rival600.hpp>
#include <RGBLib/devices/Keychron/KeychronV6.hpp>

#include <signal.h>

#include <math.h>

#include "wave.hpp"


static bool do_exit = false;

void signalHandler(int signal) {
    do_exit = true;
}

int main() {
    signal(SIGINT, signalHandler);

    Rival600* mouse = new Rival600();
    KeychronV6* keyboard = new KeychronV6();
    keyboard->set_effect();

    size_t maxKeyboardRows = 0;
    size_t maxMouseRows = 0;

    for(size_t i = 0; i < keyboard->leds.size(); i++) {
        if(keyboard->leds[i].size() > maxKeyboardRows) maxKeyboardRows = keyboard->leds[i].size();
    }
    for(size_t i = 0; i < mouse->leds.size(); i++) {
        if(mouse->leds[i].size() > maxMouseRows) maxMouseRows = mouse->leds[i].size();
    }

    Wave* wave = new Wave(maxKeyboardRows + maxMouseRows, {240, 1, 1}, {284, 1, 1}, 20, WaveDirection::WAVELEFT);

    while(!do_exit) {
        for(size_t col = 0; col < keyboard->leds.size(); col++) {
            for(size_t row = 0; row < keyboard->leds[col].size(); row++) {
                keyboard->set_led(keyboard->leds[col][row], wave->getRGB(row));
            }
        }

        keyboard->draw_frame();

        for(size_t col = 0; col < mouse->leds.size(); col++) {
            for(size_t row = 0; row < mouse->leds[col].size(); row++) {
                mouse->set_led(mouse->leds[col][row], wave->getRGB(row + maxKeyboardRows));
            }
        }

        usleep(1000 * 15);

        wave->update(1.5);
    }



    for(size_t col = 0; col < keyboard->leds.size(); col++) {
        for(size_t row = 0; row < keyboard->leds[col].size(); row++) {
            keyboard->set_led(keyboard->leds[col][row], { 0x00, 0x00, 0x00 });
        }
    }

    for(size_t col = 0; col < mouse->leds.size(); col++) {
        for(size_t row = 0; row < mouse->leds[col].size(); row++) {
            mouse->set_led(mouse->leds[col][row], { 0x00, 0x00, 0x00 });
        }
    }

    delete keyboard;
    delete mouse;
    delete wave;

    return 0;
}