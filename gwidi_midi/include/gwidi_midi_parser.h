#ifndef GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H
#define GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H

#include "GwidiData.h"

namespace gwidi {
namespace midi {

class Instrument {
public:
    enum Value {
        UNKNOWN = 0,
        HARP = 1,
        FLUTE = 2,
        BELL = 3
    };

    Instrument() : Instrument(UNKNOWN) {}
    explicit Instrument(Value val) : val{val} {
        static std::unordered_map<Value, std::string> m {
                {UNKNOWN, "unknown"},
                {HARP, "harp"},
                {FLUTE, "flute"},
                {BELL, "bell"},
        };
        name = m[val];
    }

    explicit Instrument(const char* instr) : name{instr} {
        static std::unordered_map<const char*, Value> m {
                {"unknown", UNKNOWN},
                {"harp", HARP},
                {"flute", FLUTE},
                {"bell", BELL},
        };
        val = m[name.c_str()];
    }

    static const std::string nameForInstrument(Value instr);
    static Value enumForInstrument(const char* instr);

    inline const std::string getName() const {
        return name;
    }

    inline Value getValue() const {
        return val;
    }
private:
    Value val;
    std::string name;
};

struct MidiParseOptions {
    Instrument instrument;
};

class GwidiMidiParser {
public:
    static GwidiMidiParser& getInstance() {
        static GwidiMidiParser instance;
        return instance;
    }

    GwidiData* readFile(const char* midiName, const MidiParseOptions& options);
};


}
}

#endif //GWIDI_MIDI_PARSER_GWIDI_MIDI_PARSER_H
