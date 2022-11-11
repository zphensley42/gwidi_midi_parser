#ifndef GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H
#define GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H

#include "GwidiMidiData.h"

namespace gwidi::midi {


struct MidiParseOptions {
    std::string instrument;
    int chosen_track{0};
};

struct MidiParseTrackStats {
    std::string name;
    std::string instrument;
    int num_notes;
    double tempo;
    double duration;
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
    gwidi::data::midi::GwidiMidiData* readFile(const char* midiName, const MidiParseOptions& options);
};


}

#endif //GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H
