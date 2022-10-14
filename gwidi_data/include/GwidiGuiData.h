#ifndef GWIDI_MIDI_PARSER_GWIDIGUIDATA_H
#define GWIDI_MIDI_PARSER_GWIDIGUIDATA_H

#include <vector>
#include <string>
#include <map>

namespace gwidi { namespace data { namespace gui {

enum Instrument {
    UNKNOWN = 0,
    HARP = 1,
    FLUTE = 2,
    BELL = 3
};
const char* nameForInstrument(Instrument instr);
Instrument instrumentForName(const char* instr);

struct Note {
    std::vector<std::string> letters;
    int measure{0};
    int octave{0};
    int time{0};
    std::string key{};
    bool activated{false};
};

struct Octave {
    int num{0};
    int measure{0};
    std::map<int, std::vector<Note>> notes;
};

struct Measure {
    int num{0};
    std::vector<Octave> octaves{};
};

// TODO: Hook this data up to our Godot program
class GwidiGuiData {
public:
    GwidiGuiData() : GwidiGuiData(Instrument::HARP) {}
    explicit GwidiGuiData(Instrument instr);

    void addMeasure(); // Add to gui data
    void toggleNote(){} // Add to tick map

    inline std::vector<Measure>& getMeasures() {
        return measures;
    }

    // TODO: Need to be able to convert this data to GwidiData (at least in 2 pieces: tick map for playback and data for saving)
    // TODO: Need to be able to convert GwidiData to this type (for loading and MIDI import)

private:
    Instrument instrument;

    std::vector<Measure> measures;
};

}}}

#endif //GWIDI_MIDI_PARSER_GWIDIGUIDATA_H
