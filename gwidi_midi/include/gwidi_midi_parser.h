#ifndef GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H
#define GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H

#include "GwidiData.h"

namespace gwidi::midi {

enum Instrument {
    UNKNOWN = 0,
    HARP = 1,
    FLUTE = 2,
    BELL = 3
};
const char* nameForInstrument(Instrument instr);
Instrument instrumentForName(const char* instr);

struct MidiParseOptions {
    Instrument instrument;
    int chosen_track{0};
};

struct MidiParseTrackStats {
    std::string name;
    int num_notes;
};

class GwidiMidiParser {
public:
    using TrackMeta = std::map<int, MidiParseTrackStats>;
    static GwidiMidiParser& getInstance() {
        static GwidiMidiParser instance;
        return instance;
    }

    // Used to let users choose which track to pick when midi importing (passed in MidiParseOptions)
    TrackMeta getTrackMetaMap(const char* midiName);
    gwidi::data::midi::GwidiData* readFile(const char* midiName, const MidiParseOptions& options);
};


}

#endif //GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H
