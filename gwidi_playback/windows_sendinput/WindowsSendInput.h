#ifndef EVDEV_TEST_WINDOWSSENDINPUT_H
#define EVDEV_TEST_WINDOWSSENDINPUT_H

#include <unordered_map>
#include <string>

class WSendInput {
public:
    WSendInput();
    ~WSendInput();
    long sendInput(const std::string& key);
private:
    static std::unordered_map<std::string, int> hk_map;
    static int keyToHk(const std::string& key);

};

#endif //EVDEV_TEST_WINDOWSSENDINPUT_H
