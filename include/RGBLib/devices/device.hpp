#ifndef __RGBLIB_DEVICE_HPP__
#define __RGBLIB_DEVICE_HPP__

#include "../util/rgb.hpp"

#include <thread>
#include <vector>
#include <functional>
#include <stdexcept>
#include <mutex>

#include <hidapi/hidapi.h>
#include <stdint.h>

class Device {
protected:
    std::function<void()> onDeviceConnect;

    std::mutex deviceMutex;
    hid_device* device;

    int initDevice() {
        hid_device_info* devices = hid_enumerate(VENDOR_ID, PRODUCT_ID);
        hid_device_info* current_device = devices;

        while(current_device) {
            if(USAGE_PAGE == 0x00 && USAGE == 0x00) {
                device = hid_open_path(current_device->path);
                break;
            }

            if(current_device->usage_page == USAGE_PAGE && current_device->usage == USAGE) {
                device = hid_open_path(current_device->path);
                break;
            }

            current_device = current_device->next;
        }

        hid_free_enumeration(devices);

        if(!device) {
            device = NULL;

            return -1;
        }

        return 0;
    }


    std::thread deviceCheckerThread;
    bool deviceCheckerThreadActive;

    void stopCheckDeviceThread() {
        if(!deviceCheckerThreadActive) return;

        deviceCheckerThreadActive = false;
        deviceCheckerThread.join();
    }

    void startCheckDeviceThread() {
        deviceCheckerThreadActive = true;

        deviceCheckerThread = std::thread([this]() -> void {
            while(this->deviceCheckerThreadActive) {
                hid_device_info* info = hid_enumerate(VENDOR_ID, PRODUCT_ID);
                if(!info || !device) {
                    deviceMutex.lock();
                    hid_free_enumeration(info);

                    if(device) {
                        printf("HID Device with VID PID %.4X:%.4X disconnected. trying to reconnect...\n", VENDOR_ID, PRODUCT_ID);

                        hid_close(this->device);
                        this->device = NULL;
                    }

                    if(this->initDevice() == 0) {
                        printf("Sucessfully reconnected!\n");
                        this->onDeviceConnect();

                        deviceMutex.unlock();

                        continue;
                    }

                    deviceMutex.unlock();

                    printf("Failed reconecting. Trying again in 5 seconds...\n");
                    std::this_thread::sleep_for(std::chrono::seconds(4));
                }
                else {
                    hid_free_enumeration(info);
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    }

public:
    const unsigned int VENDOR_ID;
    const unsigned int PRODUCT_ID;

    const unsigned int USAGE_PAGE;
    const unsigned int USAGE;

    const std::vector<std::vector<uint8_t>> leds;

    Device(unsigned int VENDOR_ID, unsigned int PRODUCT_ID, unsigned int usage_page, unsigned int usage, std::vector<std::vector<uint8_t>> leds, std::function<void()> onDeviceConnect) :
        onDeviceConnect(onDeviceConnect), VENDOR_ID(VENDOR_ID), PRODUCT_ID(PRODUCT_ID), USAGE_PAGE(usage_page), USAGE(usage), leds(leds) {
        initDevice();

        if(device) {
            this->onDeviceConnect();
        }
        else {
            printf("Failed to open HID device %.4X:%.4X\n", VENDOR_ID, PRODUCT_ID);
        }

        startCheckDeviceThread();
    }

    ~Device() {
        stopCheckDeviceThread();

        if(device) {
            hid_close(device);
        }
    }

    virtual void set_led(unsigned char led, RGB rgb) = 0;
};

#endif
