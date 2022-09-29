#ifndef GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H
#define GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H

#include "GwidiOptions.h"
#include "GwidiData.h"

class GwidiMidiParser {
public:
    static GwidiMidiParser& getInstance() {
        static GwidiMidiParser instance;
        return instance;
    }

    GwidiData* readFile(const char* midiName, gwidi::options::MidiParseOptions options);
};

#endif //GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H
