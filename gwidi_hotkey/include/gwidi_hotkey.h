#ifndef GWIDI_MIDI_PARSER_GWIDI_HOTKEY_H
#define GWIDI_MIDI_PARSER_GWIDI_HOTKEY_H

#include <sys/poll.h>
#include <utility>
#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include "GwidiOptions2.h"

#include "GwidiServerClient.h"

namespace gwidi::hotkey {

class GwidiHotkeyAssignmentPressDetector {
public:
    struct DetectedKey {
        std::string name;
        int key;
    };

    void beginListening();
    void stopListening();

    inline bool isAlive() {
        return m_thAlive.load();
    }

    // These 2 methods can be used in combination to initiate listening / listing keys that are pressed from the moment we ask to the moment we retrieve the list
    void clearPressedKeys();
    inline void assignPressedKeyListener(std::function<void()> cb) {
        m_pressedKeyCb = std::move(cb);
    }
    std::vector<DetectedKey> pressedKeys();

    ~GwidiHotkeyAssignmentPressDetector();

private:
    std::atomic_bool m_thAlive{false};

    std::vector<int> m_pressedKeys;
    std::vector<int> m_tempPressedKeys;
    std::function<void()> m_pressedKeyCb;
};

class GwidiHotkey {
public:
    void beginListening();
    void stopListening();

    inline bool isAlive() {
        return m_thAlive.load();
    }

    void assignHotkeyFunction(const std::string& name, std::function<void()> cb);

    ~GwidiHotkey();
private:
    void hotkeyDetected(gwidi::options2::HotkeyOptions::HotKey& hotKey);

    std::vector<pollfd> m_inputDevices;
    static int m_timeoutMs;

    std::vector<int> m_pressedKeys;
    std::atomic_bool m_thAlive{false};
    std::shared_ptr<std::thread> m_th;

    std::map<std::string, std::function<void()>> m_hotkeyCbs;
};

}
#endif //GWIDI_MIDI_PARSER_GWIDI_HOTKEY_H
