#ifndef GWIDI_MIDI_PARSER_GWIDIGUIDATA_H
#define GWIDI_MIDI_PARSER_GWIDIGUIDATA_H

#include <vector>
#include <string>
#include <map>

namespace gwidi::data::gui {

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

class GwidiGuiData {
public:
    // key -> time in offset
    // value -> list of notes that are activated for that time
    using TickMapType = std::map<double, std::vector<Note>>;

    GwidiGuiData() : GwidiGuiData(Instrument::HARP) {}
    explicit GwidiGuiData(Instrument instr);

    void addMeasure(); // Add to gui data
    void toggleNote(Note* note);

    double trackDuration();
    double timeIndexToTickOffset(Note* note) const;

    inline std::vector<Measure>& getMeasures() {
        return measures;
    }

    inline TickMapType & getTickMap() {
        return m_tickMap;
    }

    double getTempo() const;

    // TODO: Need to be able to convert this data to GwidiData (at least in 2 pieces: tick map for playback and data for saving)
    // TODO: Need to be able to convert GwidiData to this type (for loading and MIDI import)

private:
    friend class GwidiDataConverter;

    Instrument instrument;

    double m_tempo{0.0};
    TickMapType m_tickMap;
    std::vector<Measure> measures;
};

}

#endif //GWIDI_MIDI_PARSER_GWIDIGUIDATA_H
