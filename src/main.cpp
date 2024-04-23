#include <stdio.h>
#include <RGBLib/devices/SteelSeries/Rival600.hpp>
#include <RGBLib/devices/Keychron/KeychronV6.hpp>

#include <signal.h>

#include <math.h>

#include "wave.hpp"
#include "virt_utils.hpp"


static Wave* wave;

void keyboardWaveUpdater(KeychronV6* keyboard) {
    keyboard->set_effect();

    size_t frame = 0;
    while(wave->updaterThreadRunning()) {
        // set around every 5 seconds
        if(frame % (30 * 5) == 0) {
            keyboard->set_effect();
        }

        for(size_t col = 0; col < keyboard->getCols(); col++) {
            keyboard->set_col(col, wave->getRGB(col));
        }

        keyboard->draw_frame();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000/15));
        frame++;
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

        std::this_thread::sleep_for(std::chrono::milliseconds(1000/15));
    }
}

void onSIGINT(int) {
    printf("SIGINT RECEIVED! SHUTTING DOWN...\n");

    wave->stopUpdaterThread();
}

void cleanup(KeychronV6* keyboard, Rival600* mouse, std::thread* keyboardWaveUpdaterThread, std::thread* mouseWaveUpdaterThread, std::thread* virtCheckerThread) {
    signal(SIGINT, onSIGINT);

    keyboardWaveUpdaterThread->join();
    mouseWaveUpdaterThread->join();

    usleep(1000 * 100);

    for(std::pair<const uint8_t, RGB> pair : keyboard->get_custom_leds()) {
        keyboard->unset_custom_led(pair.first);
    }

    keyboard->draw_frame();

    for(size_t col = 0; col < mouse->leds.size(); col++) {
        for(size_t row = 0; row < mouse->leds[col].size(); row++) {
            mouse->set_led(mouse->leds[col][row], { 0x00, 0x00, 0x00 });
        }
    }

    virtCheckerThread->join();

    delete keyboard;
    delete mouse;
    delete wave;

    hid_exit();
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
    wave->startUpdaterThread(0.15);

    std::thread keyboardWaveUpdaterThread(keyboardWaveUpdater, keyboard);
    std::thread mouseWaveUpdaterThread(mouseWaveUpdater, mouse);

    std::thread virtCheckerThread([](KeychronV6* keyboard) -> void {
        VirtConnection con = VirtConnection("qemu:///system");

        while(wave->updaterThreadRunning()) {
            bool isOn = VirtUtils::VirtualMachineOn(con, "win10");
            if(isOn) {
                keyboard->unset_custom_led(14);
            }
            else {
                keyboard->set_custom_led(14, { 69, 3, 1 });
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

    }, keyboard);

    cleanup(keyboard, mouse, &keyboardWaveUpdaterThread, &mouseWaveUpdaterThread, &virtCheckerThread);

    return 0;
}
