#include <algorithm>
#include "GwidiGuiData.h"
#include "GwidiOptions2.h"

namespace gwidi::data::gui {

const char* nameForInstrument(Instrument instr) {
    static std::map<Instrument, const char*> m {
        {Instrument::UNKNOWN, "unknown"},
        {Instrument::HARP, "harp"},
        {Instrument::FLUTE, "flute"},
        {Instrument::BELL, "bell"},
    };
    return m[instr];
}
Instrument instrumentForName(const char* instr) {
    static std::map<const char*, Instrument> m {
            {"unknown", Instrument::UNKNOWN},
            {"harp", Instrument::HARP},
            {"flute", Instrument::FLUTE},
            {"bell", Instrument::BELL},
    };
    return m[instr];
}

GwidiGuiData::GwidiGuiData(gwidi::data::gui::Instrument instr) : instrument{instr} {
    m_tempo = options2::GwidiOptions2::getInstance().tempo();
    addMeasure();
}

void GwidiGuiData::addMeasure() {
    // Parse our instrument options
    // Use them as the template for each measure of gui data
    auto &options = gwidi::options2::GwidiOptions2::getInstance().getMapping()[nameForInstrument(instrument)];

    Measure measure;
    measure.num = measures.size();
    // 16 times in a measure
    for(auto &octave : options.octaves) {
        Octave o;
        o.num = octave.num;
        o.measure = measure.num;
        for(auto i = 0; i < 16; i++) {
            o.notes[i] = std::vector<Note>();
            for(auto &note : octave.notes) {
                Note n {
                    note.letters,
                    measure.num,
                    o.num,
                    i,
                    note.key,
                    false
                };
                o.notes[i].emplace_back(n);
            }
        }
        measure.octaves.emplace_back(o);
    }

    measures.emplace_back(measure);
}

void GwidiGuiData::toggleNote(Note *note) {
    note->activated = !note->activated;

    // Assign based on offset time (which is a function of time and tempo)
    double offset = timeIndexToTickOffset(note);
    auto tickIt = m_tickMap.find(offset);
    if(tickIt == m_tickMap.end()) {
        m_tickMap[offset] = std::vector<Note>();
    }
    auto &notes = m_tickMap[offset];
    auto notesIt = std::find_if(notes.begin(), notes.end(), [note](const Note& n){
        return n.measure == note->measure && n.octave == note->octave && n.time == note->time && n.key == note->key;
    });
    if(notesIt != notes.end()) {
        notes.erase(notesIt);
    }
    else {
        notes.emplace_back(Note{*note});
    }
}

double GwidiGuiData::trackDuration() {
    auto &noteTimes = measures.back().octaves.front().notes;
    auto endIt = noteTimes.end();
    auto &note = std::prev(endIt)->second.front();
    return timeIndexToTickOffset(&note) + 1.0;
}

double GwidiGuiData::timeIndexToTickOffset(Note* note) const {
    if(!note) {
        return 0.0;
    }
    int num_notes_per_measure = options2::GwidiOptions2::getInstance().notesPerMeasure();
    double sixteenthNoteTPQ = 15 / m_tempo;
    int note_time = (note->measure * num_notes_per_measure) + (note->time);

    return note_time == 0 ? 0 : note_time * sixteenthNoteTPQ;

//    return note_time == 0 ? 0 : (tempo / note_time) / 1000.0;      // bpm -> per second
}

}
