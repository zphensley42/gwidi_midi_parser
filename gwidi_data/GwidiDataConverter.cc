#include "GwidiDataConverter.h"
#include <spdlog/spdlog.h>

namespace gwidi::data {

GwidiDataConverter &GwidiDataConverter::getInstance() {
    static GwidiDataConverter instance;
    return instance;
}

gui::GwidiGuiData* GwidiDataConverter::midiToGui(midi::GwidiMidiData* data) {
    auto ret = new gui::GwidiGuiData();
    if(data->getTracks().empty()) {
        return ret;
    }
    // Due to chosen_track in midi parsing, there should only be a single track
    auto &track = data->getTracks().front();
    double originalTempo = 60 / data->getTempo(); // Tempo for midi parsed data is already in TPQ format
    double sixteenthNoteTPQ = 15 / originalTempo;
    int perMeasure = gwidi::options2::GwidiOptions2::getInstance().notesPerMeasure();
    for(auto &note : track.notes) {
        // determine the # of measure from our start time
        // determine the # of octave from the note

        auto timeIndex = int(ceil(note.start_offset / sixteenthNoteTPQ));
        int measureIndex = timeIndex / perMeasure;
        int timeInMeasure = timeIndex % perMeasure;
        spdlog::debug("midiToGui, note start: {}, timeIndex: {}, measureIndex: {}", note.start_offset, timeIndex, measureIndex);

        auto &retMeasures = ret->getMeasures();

        // Ensure we have enough measures to handle the note
        while(measureIndex >= retMeasures.size()) {
            ret->addMeasure();
        }

        auto &measure = retMeasures.at(measureIndex);
        auto &octave = measure.octaves.at(note.octave); // This may be an issue, depending on how octave index vs octave num is handled
        // TODO: possibly add the separation? (octave num vs index) to make it more clear in our data usage
        for(auto &retNote : octave.notes[timeInMeasure]) {
            if(retNote.key == note.key) {
                ret->toggleNote(&retNote);
                break;
            }
        }
    }
    return ret;
}
midi::GwidiMidiData* GwidiDataConverter::guiToMidi(gui::GwidiGuiData* data) {
    auto ret = new midi::GwidiMidiData();
    ret->assignTempo(data->getTempo(), data->getTempo() / 1000000);

    auto &measures = data->getMeasures();
    double sixteenthNoteTPQ = 15 / data->getTempo();

    auto notes = std::vector<gwidi::data::midi::Note>();
    for(auto &measure : measures) {
        auto &octaves = measure.octaves;
        for(auto &octave : octaves) {
            auto &times = octave.notes;
            for(auto &time : times) {
                for(auto &note : time.second) {
                    if(note.activated) {
                        auto midiNote = gwidi::data::midi::Note {
                                data->timeIndexToTickOffset(&note),
                                sixteenthNoteTPQ,
                                note.octave,
                                note.letters.at(0),
                                gwidi::data::gui::nameForInstrument(gwidi::data::gui::Instrument::HARP),
                                0,
                                note.key
                        };
                        notes.emplace_back(midiNote);
                    }
                }
            }
        }
    }
    ret->addTrack(gwidi::data::gui::nameForInstrument(gwidi::data::gui::Instrument::HARP), "gwidi_gui", notes, data->trackDuration());
    ret->fillTickMap();
    return ret;
}

}
