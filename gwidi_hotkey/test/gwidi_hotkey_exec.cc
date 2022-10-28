#include "gwidi_hotkey.h"

int main() {
    auto hotKeys = gwidi::hotkey::GwidiHotkey();
    hotKeys.beginListening();
    return 0;
}