#ifndef GWIDI_MIDI_PARSER_GWIDI_OPTIONS_2_PARSER_H
#define GWIDI_MIDI_PARSER_GWIDI_OPTIONS_2_PARSER_H

// TODO: Build data definition to hold the instrument settings
#include <map>
#include <vector>
#include <string>

struct Note {
    std::vector<std::string> letters;
    int midi_octave{0};
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
    static GwidiOptions2 &getInstance();
private:
    GwidiOptions2();

    void parseConfigs();

    std::map<std::string, Instrument> instruments;
};


#endif //GWIDI_MIDI_PARSER_GWIDI_OPTIONS_2_PARSER_H
