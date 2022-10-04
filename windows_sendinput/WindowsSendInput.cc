#include "WindowsSendInput.h"
#include <iostream>
#include <unordered_map>
#include "windows.h"

#define HK_0key 0x0b
#define HK_1key 0x02
#define HK_2key 0x03
#define HK_3key 0x04
#define HK_4key 0x05
#define HK_5key 0x06
#define HK_6key 0x07
#define HK_7key 0x08
#define HK_8key 0x09
#define HK_9key 0x0a

std::unordered_map<std::string, int> WSendInput::hk_map {
        {"0", HK_0key}, // octave up
        {"1", HK_1key},
        {"2", HK_2key},
        {"3", HK_3key},
        {"4", HK_4key},
        {"5", HK_5key},
        {"6", HK_6key},
        {"7", HK_7key},
        {"8", HK_8key},
        {"9", HK_9key}, // octave down
};

long WSendInput::sendInput(const std::string& sKey) {
    int key = keyToHk(sKey);

    INPUT inputs[2] = {};
    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.time = 0;
    inputs[0].ki.wVk = 0;
    inputs[0].ki.dwExtraInfo = 0;

    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;
    inputs[0].ki.wScan = key;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.time = 0;
    inputs[1].ki.wVk = 0;
    inputs[1].ki.dwExtraInfo = 0;

    inputs[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    inputs[1].ki.wScan = key;

    UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if(uSent != ARRAYSIZE(inputs)) {
        return -1;
//        return HRESULT_FROM_WIN32(GetLastError());
    }

    return 0;
}

WSendInput::WSendInput() {
}

WSendInput::~WSendInput() {
}

int WSendInput::keyToHk(const std::string& key) {
    auto it = hk_map.find(key);
    if(it == hk_map.end()) {
        return VK_CANCEL;   // just some dummy for now
    }
    return it->second;
}
