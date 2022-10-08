#ifndef GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H
#define GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H

#include <gwidi_options-1.0/GwidiOptions.h>
#include <gwidi_data-1.0/GwidiData.h>

class GwidiMidiParser {
public:
    static GwidiMidiParser& getInstance() {
        static GwidiMidiParser instance;
        return instance;
    }

    GwidiData* readFile(const char* midiName, gwidi::options::MidiParseOptions options);
};

#endif //GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H
