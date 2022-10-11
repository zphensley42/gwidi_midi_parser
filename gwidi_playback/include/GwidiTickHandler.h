#ifndef GWIDI_MIDI_PARSER_GWIDITICKHANDLER_H
#define GWIDI_MIDI_PARSER_GWIDITICKHANDLER_H

#include "GwidiOptions2.h"

#include "GwidiData.h"
#include "gwidi_midi_parser.h"

namespace gwidi {
namespace tick {

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
    using WrapperTickMapType = std::map<int, std::map<double, std::vector<Note>>>;
    void setOptions(GwidiTickOptions options);
    void assignData(GwidiData* data);
    GwidiAction* processTick(double delta);

private:
    std::unordered_map<int, double> currentTickMapFloorKey();

    GwidiTickOptions options;
    GwidiData* data{nullptr};
    WrapperTickMapType tickMap;
    double cur_time{0.0};
};

}
}

#endif //GWIDI_MIDI_PARSER_GWIDITICKHANDLER_H
