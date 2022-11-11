#include <spdlog/spdlog.h>
#include "GwidiTickHandler.h"
#include "GwidiPlayback.h"

#if defined(WIN32) || defined(WIN64)
#include "WindowsSendInput.h"

#define TEST_FILE R"(E:\Tools\repos\gwidi_midi_parser\assets\super_mario.mid)"
#elif defined(__linux__)
#include "LinuxSendInput.h"
#define TEST_FILE R"(/home/zhensley/repos/gwidi_godot/gwidi_midi_parser/assets/slow_scale.mid)"
#endif

void testMidi() {
    auto data = gwidi::midi::GwidiMidiParser::getInstance().readFile(TEST_FILE, gwidi::midi::MidiParseOptions {
        "default",
        1
    });

    auto playback = gwidi::playback::GwidiPlayback("default");
    playback.assignData(data, gwidi::tick::GwidiTickOptions{
            gwidi::tick::GwidiTickOptions::ActionOctaveBehavior::HIGHEST
    });

    playback.play();
    // for now, don't hook up controls
    while(playback.isPlaying()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    delete data;    // we COULD delete this earlier, after the assignData -> the tickmap in playback is a copy
}

void testGui() {
    auto data = new gwidi::data::gui::GwidiGuiData();
    data->addMeasure();

    // test data -> 2 measures, single octave with each note activated, moving down the key list
    auto &measure1 = data->getMeasures().at(0);
    auto &measure2 = data->getMeasures().at(1);

    auto &measure1Times = measure1.octaves.front().notes;
    auto &measure2Times = measure2.octaves.front().notes;

    int index = 0;
    for(auto &time : measure1Times) {
        if(index >= time.second.size()) {
            index = 0;
        }
        auto &note = time.second.at(index);
        data->toggleNote(&note);
        index++;
    }

    index = 0;
    for(auto &time : measure2Times) {
        if(index >= time.second.size()) {
            index = 0;
        }
        auto &note = time.second.at(index);
        data->toggleNote(&note);
        index++;
    }

    auto playback = gwidi::playback::GwidiPlayback("default");
    playback.assignData(data, gwidi::tick::GwidiTickOptions{
            gwidi::tick::GwidiTickOptions::ActionOctaveBehavior::HIGHEST
    });

    playback.play();
    // for now, don't hook up controls
    int count = 0;
    while(!playback.isStopped()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        if(count == 1) {
            playback.pause();
            assert(playback.isPaused());
        }

        if(count == 10) {
            playback.play();
            assert(playback.isPlaying());
        }
        count++;
    }

    delete data;    // we COULD delete this earlier, after the assignData -> the tickmap in playback is a copy
}

int main() {

    spdlog::set_level(spdlog::level::debug);

    testMidi();
    testGui();

    return 0;
}
