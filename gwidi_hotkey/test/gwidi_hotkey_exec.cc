#include <linux/input-event-codes.h>
#include <spdlog/spdlog.h>
#include "gwidi_hotkey.h"

void testPressDetector() {
    auto detector = std::make_shared<gwidi::hotkey::GwidiHotkeyAssignmentPressDetector>();
    detector->assignPressedKeyListener([detector](){
        // Keys were pressed, check them
        auto pressedKeys = detector->pressedKeys();
        for(const auto& pressedKey : pressedKeys) {
            spdlog::info("pressed key: {}, name: {}", pressedKey.key, pressedKey.name);
        }
        if(pressedKeys.size() == 3) {
            // For testing, stop press detector when we see 3 keys
            detector->stopListening();
        }
    });
    detector->beginListening();

    while(detector->isAlive()) {}

    spdlog::info("testPressDetector finished");
}

void testHotkeyDetector() {
    auto hotkeys = std::make_shared<gwidi::hotkey::GwidiHotkey>();
    hotkeys->assignHotkeyFunction("stop", [hotkeys](){
        hotkeys->stopListening();
    });
    hotkeys->beginListening();

    while(hotkeys->isAlive()) {}

    spdlog::info("testHotkeyDetector finished");
}


int main() {
    auto listener = gwidi::udpsocket::GwidiServerClientManager::instance().serverListener();
    listener->start();

    // For now, only watch for a subset of keys for testing
//    listener->sendWatchedKeysReconfig({
//        KEY_LEFTSHIFT,
//        KEY_LEFTCTRL,
//        KEY_1,
//        KEY_2
//    });

    // For testing, use the special case of an empty list == all keys
    listener->sendWatchedKeysReconfig({});

    testPressDetector();
    testHotkeyDetector();

    listener->stop();
    return 0;
}