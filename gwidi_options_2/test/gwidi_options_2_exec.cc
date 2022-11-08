#include "GwidiOptions2.h"
#include <spdlog/spdlog.h>
#include <memory>

std::shared_ptr<gwidi::options2::HotkeyOptions::HotKey> findHotkey(const std::string &name) {
    auto &hotkeyOptions = gwidi::options2::HotkeyOptions::getInstance();
    auto &mapping = hotkeyOptions.getHotkeyMapping();

    for(auto &entry : mapping) {
        if(entry.second.name == name) {
            auto copy = gwidi::options2::HotkeyOptions::HotKey(entry.second);
            return std::make_shared<gwidi::options2::HotkeyOptions::HotKey>(copy);
        }
    }
    return nullptr;
}

int main() {
    spdlog::set_level(spdlog::level::debug);
    auto &options = gwidi::options2::GwidiOptions2::getInstance();
    spdlog::debug("====Options====\n{}", (std::string)options);

    auto &hotkeyOptions = gwidi::options2::HotkeyOptions::getInstance();
    auto &mapping = hotkeyOptions.getHotkeyMapping();

    assert(mapping.size() == 2);
    for(auto &entry : mapping) {
        auto item = entry.first;
        auto itemHash = gwidi::options2::HotkeyOptions::hashFromKeys(entry.second.keys);
        assert(item == itemHash);
    }

    auto hotkey = findHotkey("play_pause");
    hotkey->keys.emplace_back(gwidi::options2::HotkeyOptions::keyNameToCode("space"));
    hotkeyOptions.updateMapping(*hotkey);

    hotkeyOptions.reloadConfig();
    hotkey = findHotkey("play_pause");
    assert(hotkey->keys.size() == 4);
    assert(hotkey->keys.back() == gwidi::options2::HotkeyOptions::keyNameToCode("space"));

    hotkey->keys.pop_back();
    hotkeyOptions.updateMapping(*hotkey);

    hotkeyOptions.reloadConfig();
    hotkey = findHotkey("play_pause");
    assert(hotkey->keys.size() == 3);
    assert(hotkey->keys.back() != gwidi::options2::HotkeyOptions::keyNameToCode("space"));

    return 0;
}
