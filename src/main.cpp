#include <stdio.h>
#include <RGBLib/devices/SteelSeries/Rival600.hpp>
#include <RGBLib/devices/Keychron/KeychronV6.hpp>

#include <signal.h>

#include <math.h>

#include "wave.hpp"


static Wave* wave;

void keyboardWaveUpdater(KeychronV6* keyboard) {
    keyboard->set_effect();

    while(wave->updaterThreadRunning()) {
        for(size_t col = 0; col < keyboard->leds.size(); col++) {
            for(size_t row = 0; row < keyboard->leds[col].size(); row++) {
                keyboard->set_led(keyboard->leds[col][row], wave->getRGB(row));
            }
        }

        keyboard->draw_frame();
    }
}

void mouseWaveUpdater(Rival600* mouse) {
    size_t maxMouseRows = 0;

    for(size_t i = 0; i < mouse->leds.size(); i++) {
        if(mouse->leds[i].size() > maxMouseRows) maxMouseRows = mouse->leds[i].size();
    }

    while(wave->updaterThreadRunning()) {
        for(size_t col = 0; col < mouse->leds.size(); col++) {
            for(size_t row = 0; row < mouse->leds[col].size(); row++) {
                mouse->set_led(mouse->leds[col][row], wave->getRGB(row + (wave->getRowsLen() - maxMouseRows)));
            }
        }
    }
}

void cleanup(KeychronV6* keyboard, Rival600* mouse) {
    keyboard->draw_frame();

    for(size_t col = 0; col < mouse->leds.size(); col++) {
        for(size_t row = 0; row < mouse->leds[col].size(); row++) {
            mouse->set_led(mouse->leds[col][row], { 0x00, 0x00, 0x00 });
        }
    }

    delete keyboard;
    delete mouse;
    delete wave;

    hid_exit();
}


void onSIGINT(int signal) {
    printf("SIGINT RECEIVED! SHUTTING DOWN...\n");

    wave->stopUpdaterThread();
}

int main() {
    signal(SIGINT, onSIGINT);

    Rival600* mouse = new Rival600();
    KeychronV6* keyboard = new KeychronV6();


    size_t maxKeyboardRows = 0;
    size_t maxMouseRows = 0;

    for(size_t i = 0; i < keyboard->leds.size(); i++) {
        if(keyboard->leds[i].size() > maxKeyboardRows) maxKeyboardRows = keyboard->leds[i].size();
    }

    for(size_t i = 0; i < mouse->leds.size(); i++) {
        if(mouse->leds[i].size() > maxMouseRows) maxMouseRows = mouse->leds[i].size();
    }

    wave = new Wave(maxKeyboardRows + maxMouseRows, {240, 1, 1}, {284, 1, 1}, 120, WaveDirection::WAVELEFT);
    wave->startUpdaterThread(0.2);

    std::thread keyboardWaveUpdaterThread(keyboardWaveUpdater, keyboard);
    std::thread mouseWaveUpdaterThread(mouseWaveUpdater, mouse);

    keyboardWaveUpdaterThread.join();
    mouseWaveUpdaterThread.join();

    cleanup(keyboard, mouse);

    return 0;
}
