#include <spdlog/spdlog.h>
#include "gwidi_hotkey.h"

#include <linux/input.h>
#include <csignal>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <filesystem>
#include <utility>

#define EVT_DEVICE "/dev/input/event7"

namespace gwidi::hotkey {

int GwidiHotkey::m_timeoutMs{5000};

bool supports_key_events(const int &fd) {
    unsigned long evbit = 0;
    ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), &evbit);
    return (evbit & (1 << EV_KEY));
}

GwidiHotkey::~GwidiHotkey() {
    if(m_thAlive.load() && m_th.joinable()) {
        m_thAlive.store(false);
        m_th.join();
    }
    else {
        m_thAlive.store(false);
    }
}

void GwidiHotkey::findInputDevices() {
    // Requires root
    auto uid = getuid();
    if(uid != 0) {
        spdlog::warn("User is not root, cross-process hotkeys may not function!");
    }

    char eventPathStart[] = "/dev/input/event";
    m_inputDevices.clear();
    for (auto &i : std::filesystem::directory_iterator("/dev/input")) {
        if(i.is_character_file()) {
            std::string view(i.path());
            if (view.compare(0, sizeof(eventPathStart)-1, eventPathStart) == 0) {
                int evfile = open(view.c_str(), O_RDONLY | O_NONBLOCK);
                if(supports_key_events(evfile)) {
                    spdlog::debug("Adding {}", i.path().c_str());
                    m_inputDevices.push_back({evfile, POLLIN, 0});
                } else {
                    close(evfile);
                }
            }
        }
    }
}

void GwidiHotkey::stopListening() {
    m_thAlive.store(false);
}

void GwidiHotkey::beginListening() {
    m_thAlive.store(true);

    m_th = std::thread([this] {
        findInputDevices();

        // Requires root
        auto uid = getuid();
        if(uid != 0) {
            spdlog::warn("User is not root, cross-process hotkeys may not function!");
        }

        // Get our config to use when looking for inputs to see if a hotkey was detected
        auto &options = gwidi::options2::HotkeyOptions::getInstance();
        auto &mapping = options.getHotkeyMapping();

        std::vector<int> pressedKeys;
        bool doDetect = true;

        input_event ev{};
        while(m_thAlive.load()) {
            poll(m_inputDevices.data(), m_inputDevices.size(), m_timeoutMs);
            for (auto &pfd : m_inputDevices) {
                if (pfd.revents & POLLIN) {
                    if(read(pfd.fd, &ev, sizeof(ev)) == sizeof(ev)) {
                        // value: or 0 for EV_KEY for release, 1 for keypress and 2 for autorepeat
                        if(ev.type == EV_KEY && ev.code < 0x100) {
                            if(ev.value == 0) {
                                spdlog::debug("Key released: {}", ev.code);
                                pressedKeys.erase(std::remove(pressedKeys.begin(), pressedKeys.end(), ev.code), pressedKeys.end());
                            }
                            else if(ev.value == 1) {
                                spdlog::debug("Key pressed: {}", ev.code);
                                pressedKeys.emplace_back(ev.code);
                                doDetect = true;

                                if(m_tempPressedKeys.size() < 5) {  // some number based on the max individual keys to press to activate a hotkey
                                    // Disallow duplicates
                                    if(std::find(m_tempPressedKeys.begin(), m_tempPressedKeys.end(), ev.code) == m_tempPressedKeys.end()) {
                                        m_tempPressedKeys.emplace_back(ev.code);
                                        if(m_pressedKeyCb) {
                                            m_pressedKeyCb();
                                        }
                                    }
                                }
                            }
                            else if(ev.value == 2) {
                                // spdlog::info("Key autorepeat: {}", ev.code);
                            }
                        }
                    }
                }
            }

            // Check for hotkeys matching the combination
            // Only trigger when the vector changes
            if(doDetect) {
                auto hash = gwidi::options2::HotkeyOptions::hashFromKeys(pressedKeys);
                auto it = mapping.find(hash);
                if(it != mapping.end()) {
                    hotkeyDetected(it->second);
                }
                doDetect = false;
            }
        }

        for(auto &pfd : m_inputDevices) {
            close(pfd.fd);
        }
    });
}

void GwidiHotkey::hotkeyDetected(options2::HotkeyOptions::HotKey &hotKey) {
    spdlog::info("hotkeyDetected: {}", hotKey.name);
    auto it = m_hotkeyCbs.find(hotKey.name);
    if(it != m_hotkeyCbs.end()) {
        it->second();
    }
}

void GwidiHotkey::assignHotkeyFunction(const std::string &name, std::function<void()> cb) {
    m_hotkeyCbs[name] = std::move(cb);
}

void GwidiHotkey::clearPressedKeys() {
    m_tempPressedKeys.clear();
}

std::vector<GwidiHotkey::DetectedKey> GwidiHotkey::pressedKeys() {
    std::vector<GwidiHotkey::DetectedKey> ret;
    for(auto &entry : m_tempPressedKeys) {
        auto key = GwidiHotkey::DetectedKey{
                gwidi::options2::HotkeyOptions::codeToKeyName(entry),
                entry
        };
        ret.emplace_back(key);
    }
    return ret;
}

}