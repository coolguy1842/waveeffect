#include <stdio.h>
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

void onSIGINT(int) {
    printf("SIGINT RECEIVED! SHUTTING DOWN...\n");

    wave->stopUpdaterThread();
}

void cleanup(KeychronV6* keyboard, std::thread* keyboardWaveUpdaterThread, std::thread* virtCheckerThread) {
    signal(SIGINT, onSIGINT);

    keyboardWaveUpdaterThread->join();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for(std::pair<const uint8_t, RGB> pair : keyboard->get_custom_leds()) {
        keyboard->unset_custom_led(pair.first);
    }

    keyboard->draw_frame();

    virtCheckerThread->join();

    delete keyboard;
    delete wave;

    hid_exit();
}


int main() {
    signal(SIGINT, onSIGINT);

    KeychronV6* keyboard = new KeychronV6();

    size_t maxKeyboardRows = 0;
    for(size_t i = 0; i < keyboard->leds.size(); i++) {
        if(keyboard->leds[i].size() > maxKeyboardRows) {
            maxKeyboardRows = keyboard->leds[i].size();
        }
    }

    wave = new Wave(maxKeyboardRows, {240, 1, 1}, {284, 1, 1}, 60, WaveDirection::WAVELEFT);
    wave->startUpdaterThread(0.15);

    std::thread keyboardWaveUpdaterThread(keyboardWaveUpdater, keyboard);
    std::thread virtCheckerThread([](KeychronV6* keyboard) -> void {
        VirtConnection con = VirtConnection("qemu:///system");

        while(wave->updaterThreadRunning()) {
            bool isOn = VirtUtils::VirtualMachineOn(con, "win10");

            if(keyboard->keypressStartTimes.find(KEY_RIGHTALT) != keyboard->keypressStartTimes.end()) {
                if(time(NULL) - keyboard->keypressStartTimes[KEY_RIGHTALT] > 3) {
                    VirtUtils::toggleVM(con, "win10");

                    // stop vm from being toggled until a repress
                    auto it = keyboard->keypressStartTimes.find(KEY_RIGHTALT);
                    // stop race condition
                    if(it != keyboard->keypressStartTimes.end()) {
                        keyboard->keypressStartTimes.erase(it);
                    }
                }
            }

            if(isOn) {
                keyboard->unset_custom_led(14);
            }
            else {
                keyboard->set_custom_led(14, { 69, 3, 1 });
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }, keyboard);

    cleanup(keyboard, &keyboardWaveUpdaterThread, &virtCheckerThread);

    return 0;
}
