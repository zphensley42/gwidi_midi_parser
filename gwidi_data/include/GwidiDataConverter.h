#ifndef GWIDI_MIDI_PARSER_GWIDIDATACONVERTER_H
#define GWIDI_MIDI_PARSER_GWIDIDATACONVERTER_H

#include "GwidiMidiData.h"
#include "GwidiGuiData.h"

namespace gwidi::data {

class GwidiDataConverter {
public:
    static GwidiDataConverter& getInstance();

    gui::GwidiGuiData* midiToGui(midi::GwidiMidiData* data);
    midi::GwidiMidiData* guiToMidi(gui::GwidiGuiData* data);
private:
    GwidiDataConverter() {}
};

}

#endif //GWIDI_MIDI_PARSER_GWIDIDATACONVERTER_H
