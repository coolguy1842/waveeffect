#ifndef __RGBLIB_DEVICE_HPP__
#define __RGBLIB_DEVICE_HPP__

#include "../util/rgb.hpp"

#include <string.h>
#include <thread>
#include <vector>
#include <functional>
#include <stdexcept>
#include <mutex>

#include <hidapi/hidapi.h>
#include <stdint.h>

#include <libevdev-1.0/libevdev/libevdev.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <fstream>

#include <optional>
#include <map>

class Device {
private:
    static std::vector<unsigned int> getEventIDS(const unsigned int VENDOR_ID, const unsigned int PRODUCT_ID) {
        std::vector<unsigned int> eventIDS;
        printf("finding evdev file(s) for %X:%X\n", VENDOR_ID, PRODUCT_ID);

        std::ifstream stream = std::ifstream("/proc/bus/input/devices");
        std::string line;

        size_t curLine = 0;
        std::optional<size_t> goToLine(std::nullopt);
        while(std::getline(stream, line)) {
            curLine++;

            if(goToLine.has_value() && goToLine.value() == curLine) {
                goToLine.reset();

                unsigned int eventNum;
                sscanf(strstr(line.c_str(), "event"), "event%u", &eventNum);

                eventIDS.push_back(eventNum);
                continue;
            }

            unsigned int vendor = 0;
            unsigned int product = 0;

            int found = sscanf(line.c_str(), "I: Bus=%*d Vendor=%x Product=%x", &vendor, &product);
            if(
                found != 2 ||
                vendor != VENDOR_ID ||
                product != PRODUCT_ID
            ) {
                continue;
            }

            goToLine = curLine + 5;
        }

        stream.close();

        return eventIDS;
    }

protected:
    std::function<void()> onDeviceConnect;

    std::mutex deviceMutex;
    hid_device* device;
    char device_path[512];

    std::thread backgroundEvdevThread;
    bool backgroundEvdevThreadActive;

    int initDevice() {
        hid_device_info* devices = hid_enumerate(VENDOR_ID, PRODUCT_ID);
        hid_device_info* current_device = devices;

        while(current_device) {
            if(USAGE_PAGE == 0x00 && USAGE == 0x00) {
                device = hid_open_path(current_device->path);

                memset(device_path, '\0', sizeof(device_path));
                strcpy(device_path, current_device->path);

                break;
            }

            if(current_device->usage_page == USAGE_PAGE && current_device->usage == USAGE) {
                device = hid_open_path(current_device->path);

                memset(device_path, '\0', sizeof(device_path));
                strcpy(device_path, current_device->path);

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

    void stopEvdevThread() {
        if(!backgroundEvdevThreadActive && !backgroundEvdevThread.joinable()) return;

        backgroundEvdevThreadActive = false;
        backgroundEvdevThread.join();
    }


    virtual void onDeviceEvent(struct libevdev* evdev, struct input_event* event) {}

    void startEvdevThread() {
        stopEvdevThread();
        backgroundEvdevThreadActive = true;

        backgroundEvdevThread = std::thread([this]() -> void {
            std::vector<unsigned int> eventIDS = getEventIDS(VENDOR_ID, PRODUCT_ID);
            std::vector<struct libevdev*> evdevs(eventIDS.size());
            std::vector<int> fileDescriptors(eventIDS.size());


            size_t deviceNum = 0;
            for(unsigned int event : eventIDS) {
                int fd = open(std::string("/dev/input/event").append(std::to_string(event)).c_str(), O_RDONLY|O_NONBLOCK);
                int rc = libevdev_new_from_fd(fd, &evdevs[deviceNum++]);
                if(rc < 0) {
                    fprintf(stderr, "Failed to init libevdev device: event%d (%s)\n", event, strerror(-rc));
                    continue;
                }

                printf("%u\n", event);

                fileDescriptors.push_back(fd);
            }

            printf("\n");

            std::vector<std::thread> readThreads;
            for(struct libevdev* evdev : evdevs) {
                readThreads.push_back(std::thread([this](struct libevdev* evdev) -> void {
                    int rc;

                    do {
                        struct input_event ev;
                        rc = libevdev_has_event_pending(evdev);
                        if(rc > 0) {
                            rc = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

                            if(rc == 0) {
                                onDeviceEvent(evdev, &ev);
                            }
                        }
                        else if(rc == 0) {
                            std::this_thread::yield();
                        }
                    }
                    while ((rc == 1 || rc == 0 || rc == -EAGAIN) && backgroundEvdevThreadActive);

                }, evdev));
            }

            for(std::thread& readThread : readThreads) {
                readThread.join();
            }

            for(struct libevdev* evdev : evdevs) {
                libevdev_free(evdev);
            }

            for(int fd : fileDescriptors) {
                close(fd);
            }

            backgroundEvdevThreadActive = false;
        });
    }



    void stopCheckDeviceThread() {
        if(!deviceCheckerThreadActive && !deviceCheckerThread.joinable()) return;

        deviceCheckerThreadActive = false;
        deviceCheckerThread.join();
    }

    void startCheckDeviceThread() {
        stopCheckDeviceThread();
        deviceCheckerThreadActive = true;

        deviceCheckerThread = std::thread([this]() -> void {
            while(this->deviceCheckerThreadActive) {
                hid_device* check_device = hid_open_path(this->device_path);

                if(!check_device || !device) {
                    hid_close(check_device);

                    deviceMutex.lock();

                    if(device) {
                        printf("HID Device with VID PID %.4X:%.4X disconnected. trying to reconnect...\n", VENDOR_ID, PRODUCT_ID);

                        hid_close(this->device);
                        this->device = NULL;
                    }

                    if(this->initDevice() == 0) {
                        printf("Sucessfully reconnected!\n");

                        deviceMutex.unlock();

                        this->startEvdevThread();
                        this->onDeviceConnect();

                        continue;
                    }

                    deviceMutex.unlock();

                    printf("Failed reconecting. Trying again in 5 seconds...\n");
                    std::this_thread::sleep_for(std::chrono::seconds(4));
                }
                else {
                    hid_close(check_device);
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    }

    // devices need to implement this themselves
    std::map<uint8_t, RGB> custom_leds;
public:
    const unsigned int VENDOR_ID;
    const unsigned int PRODUCT_ID;

    const unsigned int USAGE_PAGE;
    const unsigned int USAGE;

    const std::vector<std::vector<uint8_t>> leds;

    Device(unsigned int VENDOR_ID, unsigned int PRODUCT_ID, unsigned int usage_page, unsigned int usage, std::vector<std::vector<uint8_t>> leds, std::function<void()> onDeviceConnect) :
        onDeviceConnect(onDeviceConnect), VENDOR_ID(VENDOR_ID), PRODUCT_ID(PRODUCT_ID), USAGE_PAGE(usage_page), USAGE(usage), leds(leds) {
        backgroundEvdevThreadActive = false;
        deviceCheckerThreadActive = false;

        initDevice();

        if(device) {
            this->startEvdevThread();
            this->onDeviceConnect();
        }
        else {
            printf("Failed to open HID device %.4X:%.4X\n", VENDOR_ID, PRODUCT_ID);
        }

        startCheckDeviceThread();
    }

    ~Device() {
        stopEvdevThread();
        stopCheckDeviceThread();

        if(device) {
            hid_close(device);
        }
    }

    virtual void set_led(unsigned char led, RGB rgb) = 0;


    std::map<uint8_t, RGB> get_custom_leds() {
        return custom_leds;
    }

    void set_custom_led(unsigned char led, RGB rgb) {
        custom_leds[led] = rgb;
    }

    void unset_custom_led(unsigned char led) {
        if(custom_leds.find(led) == custom_leds.end()) return;
        custom_leds.erase(custom_leds.find(led));
    }

    std::optional<RGB> get_custom_led(unsigned char led) {
        if(custom_leds.find(led) == custom_leds.end()) {
            return std::optional<RGB>();
        }

        return std::optional<RGB>(custom_leds[led]);
    }
};

#endif
