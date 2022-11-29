#include <spdlog/spdlog.h>
#include "gwidi_hotkey.h"

#include <linux/input.h>
#include <csignal>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <filesystem>
#include <utility>

#include "GwidiServerClient.h"

#define EVT_DEVICE "/dev/input/event7"

namespace gwidi::hotkey {

GwidiHotkeyAssignmentPressDetector::~GwidiHotkeyAssignmentPressDetector() {
    if(m_thAlive.load()) {
        stopListening();
    }
}

// No need for threading anymore? Do all the work on the callback thread for the socket?
// Be sure to clear the callback when we die -- do we need a single callback or multiple to support possible multiple listeners to the event thread?
// TODO: We probably need multiple to, if nothing else, handle focus events separately
void GwidiHotkeyAssignmentPressDetector::stopListening() {
    m_thAlive.store(false);

    auto listener = gwidi::udpsocket::GwidiServerClientManager::instance().serverListener();
    listener->removeEventCb("PressDetector");
}

void GwidiHotkeyAssignmentPressDetector::beginListening() {
    if(m_thAlive.load()) {
        return;
    }

    m_thAlive.store(true);

    auto listener = gwidi::udpsocket::GwidiServerClientManager::instance().serverListener();

    std::vector<int> pressedKeys;
    listener->addEventCb("PressDetector", [this, &pressedKeys](udpsocket::ServerEventType type, udpsocket::ServerEvent event) {
        if(type == udpsocket::ServerEventType::EVENT_KEY) {
            auto keyType = event.keyEvent.eventType;    // pressed/released
            auto keyCode = event.keyEvent.code;

            if(keyCode < 0x100) {
                if(keyType == 0) {
                    spdlog::debug("Key released: {}", keyCode);
                    pressedKeys.erase(std::remove(pressedKeys.begin(), pressedKeys.end(), keyCode), pressedKeys.end());
                }
                else if(keyType == 1) {
                    spdlog::debug("Key pressed: {}", keyCode);
                    pressedKeys.emplace_back(keyCode);

                    if(m_tempPressedKeys.size() < 5) {  // some number based on the max individual keys to press to activate a hotkey
                        // Disallow duplicates
                        if(std::find(m_tempPressedKeys.begin(), m_tempPressedKeys.end(), keyCode) == m_tempPressedKeys.end()) {
                            m_tempPressedKeys.emplace_back(keyCode);
                            if(m_pressedKeyCb) {
                                m_pressedKeyCb();
                            }
                        }
                    }
                }
            }
        }
    });
}



void GwidiHotkeyAssignmentPressDetector::clearPressedKeys() {
    m_tempPressedKeys.clear();
}

std::vector<GwidiHotkeyAssignmentPressDetector::DetectedKey> GwidiHotkeyAssignmentPressDetector::pressedKeys() {
    std::vector<GwidiHotkeyAssignmentPressDetector::DetectedKey> ret;
    for(auto &entry : m_tempPressedKeys) {
        auto key = GwidiHotkeyAssignmentPressDetector::DetectedKey{
                gwidi::options2::HotkeyOptions::codeToKeyName(entry),
                entry
        };
        ret.emplace_back(key);
    }
    return ret;
}





int GwidiHotkey::m_timeoutMs{5000};

GwidiHotkey::~GwidiHotkey() {
    if(m_thAlive.load()) {
        stopListening();
    }
}

void GwidiHotkey::stopListening() {
    m_thAlive.store(false);

    auto listener = gwidi::udpsocket::GwidiServerClientManager::instance().serverListener();
    listener->removeEventCb("HotkeyDetector");
}

void GwidiHotkey::beginListening() {
    if(m_thAlive.load()) {
        return;
    }

    m_thAlive.store(true);

    // Get our config to use when looking for inputs to see if a hotkey was detected
    auto &options = gwidi::options2::HotkeyOptions::getInstance();
    auto &mapping = options.getHotkeyMapping();

    auto listener = gwidi::udpsocket::GwidiServerClientManager::instance().serverListener();

    std::vector<int> pressedKeys;
    bool doDetect = true;
    listener->addEventCb("HotkeyDetector", [this, &pressedKeys, &doDetect, &mapping](udpsocket::ServerEventType type, udpsocket::ServerEvent event) {
        if (type == udpsocket::ServerEventType::EVENT_KEY) {
            if(event.keyEvent.code < 0x100) {
                if(event.keyEvent.eventType == 0) {
                    spdlog::info("Key released: {}", event.keyEvent.code);
                    pressedKeys.erase(std::remove(pressedKeys.begin(), pressedKeys.end(), event.keyEvent.code), pressedKeys.end());
                }
                else if(event.keyEvent.eventType == 1) {
                    spdlog::info("Key pressed: {}", event.keyEvent.code);
                    pressedKeys.emplace_back(event.keyEvent.code);
                    doDetect = true;
                }
            }

            if(doDetect) {
                auto hash = gwidi::options2::HotkeyOptions::hashFromKeys(pressedKeys);
                auto it = mapping.find(hash);
                if(it != mapping.end()) {
                    hotkeyDetected(it->second);
                }
                doDetect = false;
            }
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

}