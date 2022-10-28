#ifndef GWIDI_MIDI_PARSER_GWIDI_HOTKEY_H
#define GWIDI_MIDI_PARSER_GWIDI_HOTKEY_H

#include <sys/poll.h>
#include <vector>

namespace gwidi::hotkey {

class GwidiHotkey {
public:
    // TODO: Listen on a thread? Or should the owner call this on a thread? (maybe the latter is okay)
    void beginListening();
    void assignHotkeys(); // TODO: Just use this via options, need to link to that lib and so on
private:
    void findInputDevices();

    std::vector<pollfd> m_inputDevices;
    static int m_timeoutMs;
};

}
#endif //GWIDI_MIDI_PARSER_GWIDI_HOTKEY_H
