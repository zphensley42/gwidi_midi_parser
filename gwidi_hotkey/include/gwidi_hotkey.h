#ifndef GWIDI_MIDI_PARSER_GWIDI_HOTKEY_H
#define GWIDI_MIDI_PARSER_GWIDI_HOTKEY_H

#include <sys/poll.h>
#include <vector>
#include <thread>
#include <atomic>
#include "GwidiOptions2.h"

namespace gwidi::hotkey {

class GwidiHotkey {
public:
    struct DetectedKey {
        std::string name;
        int key;
    };

    // TODO: Listen on a thread? Or should the owner call this on a thread? (maybe the latter is okay)
    void beginListening();
    void stopListening();

    inline bool isAlive() {
        return m_thAlive.load();
    }

    void assignHotkeyFunction(const std::string& name, std::function<void()> cb);

    // These 2 methods can be used in combination to initiate listening / listing keys that are pressed from the moment we ask to the moment we retrieve the list
    void clearPressedKeys();
    inline void assignPressedKeyListener(std::function<void()> cb) {
        m_pressedKeyCb = cb;
    }
    std::vector<DetectedKey> pressedKeys();

    ~GwidiHotkey();
private:
    void findInputDevices();
    void hotkeyDetected(gwidi::options2::HotkeyOptions::HotKey& hotKey);

    std::vector<pollfd> m_inputDevices;
    static int m_timeoutMs;

    std::atomic_bool m_thAlive{false};
    std::thread m_th;

    std::map<std::string, std::function<void()>> m_hotkeyCbs;

    std::vector<int> m_tempPressedKeys;
    std::function<void()> m_pressedKeyCb;
};

}
#endif //GWIDI_MIDI_PARSER_GWIDI_HOTKEY_H
