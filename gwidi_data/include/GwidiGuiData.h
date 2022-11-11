#ifndef GWIDI_MIDI_PARSER_GWIDIGUIDATA_H
#define GWIDI_MIDI_PARSER_GWIDIGUIDATA_H

#include <vector>
#include <string>
#include <map>

namespace gwidi::data::gui {

struct Note {
    std::vector<std::string> letters;
    int measure{0};
    int octave{0};
    int time{0};
    std::string key{};
    bool activated{false};

    std::size_t hash() const {
        std::size_t h1 = std::hash<double>{}(time);
        std::size_t h2 = std::hash<double>{}(measure);
        std::size_t h3 = std::hash<int>{}(octave);
        std::size_t h4 = std::hash<std::string>{}(key);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
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

    GwidiGuiData() : GwidiGuiData("default") {}
    explicit GwidiGuiData(const std::string& instrument);

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

    std::string m_instrument;

    double m_tempo{0.0};
    TickMapType m_tickMap;
    std::vector<Measure> measures;
};

}

#endif //GWIDI_MIDI_PARSER_GWIDIGUIDATA_H
