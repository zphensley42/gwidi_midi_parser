#ifndef GWIDI_MIDI_PARSER_GWIDIOPTIONS2_H
#define GWIDI_MIDI_PARSER_GWIDIOPTIONS2_H

// TODO: Build data definition to hold the instrument settings
#include <map>
#include <vector>
#include <string>
#include <functional>

namespace gwidi {
namespace options2 {

struct Note {
    std::vector<std::string> letters;
    int midi_octave{0};
    int instrument_octave{0};
    std::string key;
};

struct Octave {
    int num{0};
    std::vector<Note> notes;
};

struct Instrument {
    bool supports_held_notes{false};
    int starting_octave{0};
    std::vector<Octave> octaves;
};

class GwidiOptions2 {
public:
    using Mapping = std::map<std::string, Instrument>;

    static GwidiOptions2 &getInstance();

    explicit operator std::string() const;
    inline Mapping& getMapping() {
        return instruments;
    }
    Note optionsNoteFromMidiNote(const std::string &instrument, int in_midiOctave, const std::string &letter);

    inline int notesPerMeasure() {
        return 16;  // for now, just force 16th notes
    }

    // Should read this from config, override this when we import a midi or load a file
    double tempo();

private:
    GwidiOptions2();
    void parseConfigs();

    std::map<std::string, Instrument> instruments;
    double m_tempo{0.0};
};

class HotkeyOptions {
public:
    struct HotKey {
        int key; // int == code from input-event-codes assigned to the function
        std::function<void()> cb;
    };
private:
    static int keyNameToCode(const std::string &key); // probably just use a local mapping of the keys available

    HotkeyOptions();
    void parseConfig();

    // the list of keys associated with a hotkey all need to be activated at the same time to determine that the hotkey should call its cb() function
    std::map<std::string, std::vector<HotKey>> m_hotkeyMapping; // mapping is against names in hotkeys (play, pause, stop, etc)
    static std::map<std::string, int> m_keyNameMapping;
};

}
}



#endif //GWIDI_MIDI_PARSER_GWIDIOPTIONS2_H
