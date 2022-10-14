#include "GwidiGuiData.h"
#include "GwidiOptions2.h"

namespace gwidi { namespace data { namespace gui {

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

}}}
