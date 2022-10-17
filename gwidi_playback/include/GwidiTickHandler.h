#ifndef GWIDI_MIDI_PARSER_GWIDITICKHANDLER_H
#define GWIDI_MIDI_PARSER_GWIDITICKHANDLER_H

#include "GwidiOptions2.h"

#include "GwidiMidiData.h"
#include "GwidiGuiData.h"
#include "gwidi_midi_parser.h"

namespace gwidi::tick {

struct Note {
    double start_offset {0.0};
    double duration {0.0};
    int octave{0};
    std::string key{};
    bool activated{false};
};

struct GwidiAction {
    std::vector<Note*> notes{};
    int chosen_octave{-1};
    bool end_reached{false};
};

struct GwidiTickOptions {
    enum ActionOctaveBehavior {
        LOWEST = 0,
        HIGHEST = 1,
        MOST = 2
    };

    ActionOctaveBehavior octaveBehavior{ActionOctaveBehavior{LOWEST}};
};

class GwidiTickHandler {
public:
    using WrapperTickMapType = std::map<double, std::vector<Note>>;
    void setOptions(GwidiTickOptions options);
    void assignData(gwidi::data::midi::GwidiMidiData* data);
    void assignData(gwidi::data::gui::GwidiGuiData* data);
    GwidiAction* processTick(double delta);

    inline bool hasData() {
        return m_gui_data != nullptr || m_midi_data != nullptr;
    }

private:
    double currentTickMapFloorKey();

    GwidiAction* processMidiTick();
    GwidiAction* processGuiTick();
    void filterByOctaveBehavior(GwidiAction *action) const;

    Note fromNote(gwidi::data::midi::Note &note);
    Note fromNote(gwidi::data::gui::Note &note);

    GwidiTickOptions options;
    gwidi::data::midi::GwidiMidiData* m_midi_data{nullptr};
    gwidi::data::gui::GwidiGuiData* m_gui_data{nullptr};
    WrapperTickMapType m_tickMap;
    double cur_time{0.0};
};

}

#endif //GWIDI_MIDI_PARSER_GWIDITICKHANDLER_H
