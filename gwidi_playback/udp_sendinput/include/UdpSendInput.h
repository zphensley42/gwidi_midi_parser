#ifndef EVDEV_TEST_UDPSENDINPUT_H
#define EVDEV_TEST_UDPSENDINPUT_H

#include <string>

class SendInput {
public:
    SendInput();
    ~SendInput();
    void sendInput(const std::string& key);
};

#endif //EVDEV_TEST_UDPSENDINPUT_H
