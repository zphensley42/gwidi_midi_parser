#ifndef GWIDI_MIDI_PARSER_GWIDIOPTIONS_H
#define GWIDI_MIDI_PARSER_GWIDIOPTIONS_H

#include <map>
#include <unordered_map>
#include <vector>
#include <string>

namespace gwidi::options {

struct InstrumentNote {
    int midiOctave;
    std::vector<std::string> letters;
};

struct InstrumentOctaveNotes {
    // set of instrument note values mapped to their {midi octave, letters}
    std::map<int, std::vector<InstrumentNote>> notes;
};

struct InstrumentOctaves {
    // set of instrument octaves mapped to their note values
    std::map<int, InstrumentOctaveNotes> octaves;
    std::unordered_map<int, std::string> keys;
    int startingOctave{0};
};

struct InstrumentSettings {
    std::map<std::string, InstrumentOctaves> instrumentMapping;
};

class InstrumentOptions {
public:
    enum Instrument {
        UNKNOWN = 0,
        HARP = 1,
        FLUTE = 2,
        BELL = 3
    };

    struct InstrumentNoteAttributes {
        int instrumentOctave{-1};
        int instrumentNoteNumber{-1};
        std::string key{};
        std::vector<std::string> letters{};
    };

    static InstrumentOptions &getInstance();
    inline InstrumentSettings &getMapping() {
        return mapping;
    }

    static const char *nameForInstrument(Instrument in);
    static Instrument enumForInstrument(const char *in);

    std::vector<int> supportedMidiOctaves(const std::string &instrument);
    InstrumentNoteAttributes instrumentNoteAttributesForMidi(const std::string &instrument, int octave, const std::string &letter);

private:
    InstrumentOptions();
    void fillMapping();

    InstrumentSettings mapping;
};

struct MidiParseOptions {
    InstrumentOptions::Instrument instrument;
};

}

#endif //GWIDI_MIDI_PARSER_GWIDIOPTIONS_H
