#ifndef GWIDI_MIDI_PARSER_GWIDIOPTIONS2_H
#define GWIDI_MIDI_PARSER_GWIDIOPTIONS2_H

// TODO: Build data definition to hold the instrument settings
#include <map>
#include <vector>
#include <string>

namespace gwidi {
namespace options2 {

struct Note {
    std::vector<std::string> letters;
    int midi_octave{0};
    int instrument_octave{0};
    std::string key;
};

struct Octave {
    int num{0};
    std::vector<Note> notes;
};

struct Instrument {
    bool supports_held_notes{false};
    int starting_octave{0};
    std::vector<Octave> octaves;
};

class GwidiOptions2 {
public:
    using Mapping = std::map<std::string, Instrument>;

    static GwidiOptions2 &getInstance();

    explicit operator std::string() const;
    inline std::map<std::string, Instrument>& getMapping() {
        return instruments;
    }
    Note optionsNoteFromMidiNote(const std::string &instrument, int in_midiOctave, const std::string &letter);

private:
    GwidiOptions2();
    void parseConfigs();

    std::map<std::string, Instrument> instruments;
};

}
}



#endif //GWIDI_MIDI_PARSER_GWIDIOPTIONS2_H
