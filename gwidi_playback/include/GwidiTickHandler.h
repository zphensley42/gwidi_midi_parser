#ifndef GWIDI_MIDI_PARSER_GWIDITICKHANDLER_H
#define GWIDI_MIDI_PARSER_GWIDITICKHANDLER_H

#include "GwidiData.h"
#include "gwidi_midi_parser.h"

struct InstrumentAction {
    int octave{0};
    std::string key{""}; // on the keyboard for direct input
    bool isNull() const {
        return key.empty();
    }
};

struct NoteWrapper {
    Note note;
    bool activated{false};
    InstrumentAction instrumentAction;
};

struct GwidiAction {
    std::vector<NoteWrapper*> notes{};
    int chosen_octave{-1};
};

struct GwidiTickOptions {
    enum ActionOctaveBehavior {
        LOWEST = 0,
        HIGHEST = 1,
        MOST = 2
    };

    ActionOctaveBehavior octaveBehavior{ActionOctaveBehavior{LOWEST}};
    InstrumentOptions::Instrument instrument{InstrumentOptions::Instrument::HARP};
};

class GwidiTickHandler {
public:
    using WrapperTickMapType = std::map<int, std::map<double, std::vector<NoteWrapper>>>;
    void setOptions(GwidiTickOptions options);
    void assignData(GwidiData* data);
    GwidiAction* processTick(double delta);

private:
    std::unordered_map<int, double> currentTickMapFloorKey();
    InstrumentAction actionForNote(InstrumentOptions::Instrument instrument, Note& note);

    GwidiTickOptions options;
    GwidiData* data{nullptr};
    WrapperTickMapType tickMap;
    double cur_time{0.0};
};

#endif //GWIDI_MIDI_PARSER_GWIDITICKHANDLER_H
