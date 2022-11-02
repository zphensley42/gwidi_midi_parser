#include "gwidi_hotkey.h"

gwidi::hotkey::GwidiHotkey* hotKeys{nullptr};

void onHotKeyStop() {
    hotKeys->stopListening();
}


int main() {
    hotKeys = new gwidi::hotkey::GwidiHotkey();
    hotKeys->assignHotkeyFunction("stop", onHotKeyStop);
    hotKeys->beginListening();

    while(hotKeys->isAlive()) {}

    delete hotKeys;
    hotKeys = nullptr;
    return 0;
}