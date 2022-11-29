#include <linux/input-event-codes.h>
#include "gwidi_hotkey.h"

gwidi::hotkey::GwidiHotkey* hotKeys{nullptr};

void onHotKeyStop() {
    hotKeys->stopListening();
}


int main() {
    auto listener = gwidi::udpsocket::GwidiServerClientManager::instance().serverListener();
    listener->start();

    // For now, only watch for a subset of keys for testing
    listener->sendWatchedKeysReconfig({
        KEY_LEFTSHIFT,
        KEY_LEFTCTRL,
        KEY_1,
        KEY_2
    });

    hotKeys = new gwidi::hotkey::GwidiHotkey();
    hotKeys->assignHotkeyFunction("stop", onHotKeyStop);
    hotKeys->beginListening();

    while(hotKeys->isAlive()) {}

    delete hotKeys;
    hotKeys = nullptr;

    listener->stop();
    return 0;
}