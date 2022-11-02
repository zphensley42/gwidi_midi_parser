#include "GwidiOptions2.h"
#include <spdlog/spdlog.h>

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

    return 0;
}
