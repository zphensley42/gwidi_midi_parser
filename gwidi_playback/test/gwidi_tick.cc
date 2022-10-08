#include "GwidiTickHandler.h"
#include "GwidiOptions.h"
#include "spdlog/spdlog.h"
#include <chrono>

#if defined(WIN32) || defined(WIN64)
#include "WindowsSendInput.h"

#define TEST_FILE R"(E:\Tools\repos\gwidi_midi_parser\assets\super_mario.mid)"
#elif defined(__linux__)
#include "LinuxSendInput.h"

//#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/test3_data.mid)"
//#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/pollyanna7.mid)"
//#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/super_mario.mid)"
//#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/undertale_snowy.mid)"
//#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/whats_new_scooby_doo.mid)"
#define TEST_FILE R"(/home/zhensley/repos/gwidi_godot/gwidi_midi_parser/assets/moana.mid)"
#endif


#if defined(WIN32) || defined(WIN64)
WSendInput* input{nullptr};
void sendInput(const std::string& key) {
    auto err = input->sendInput(key);
    if(err != 0) {
        spdlog::debug("Failed to SendInput: {}", err);
    }
}
#elif defined(__linux__)
SendInput* input{nullptr};
void sendInput(const std::string& key) {
    input->sendInput(key);
}
#endif

// used to determine when to swap octaves
int starting_octave = 0;
void swapOctaveUp() {
    spdlog::info("--BEGIN swapping octave up--");
    spdlog::info("old octave: {}", starting_octave);
    sendInput("0");
//    sendInput(keyToHk("0"));
    starting_octave++;
    spdlog::info("new octave: {}", starting_octave);
    spdlog::info("--END swapping octave up--");
}

void swapOctaveDown() {
    spdlog::info("--BEGIN swapping octave down--");
    spdlog::info("old octave: {}", starting_octave);
    sendInput("9");
//    sendInput(keyToHk("9"));
    starting_octave--;
    spdlog::info("new octave: {}", starting_octave);
    spdlog::info("--END swapping octave down--");
}

int octaveSwapDelay(GwidiAction *action) {
    if(action->chosen_octave != -1) {
        int diff = abs(action->chosen_octave - starting_octave);
        return diff > 1 ? 200 : 10;
    }
    return 0;
}

std::chrono::milliseconds curTime() {
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
}

// TODO: Refactor to a playback class to respond to play actions
int main() {
#if defined(WIN32) || defined(WIN64)
    input = new WSendInput();
#elif defined(__linux__)
    input = new SendInput();
#endif

    spdlog::set_level(spdlog::level::debug);

    auto data = GwidiMidiParser::getInstance().readFile(TEST_FILE, gwidi::options::MidiParseOptions{
            gwidi::options::InstrumentOptions::Instrument::HARP
    });
    auto tickHandler = GwidiTickHandler();
    tickHandler.setOptions(GwidiTickOptions{
        GwidiTickOptions::ActionOctaveBehavior::HIGHEST
    });
    tickHandler.assignData(data);

    // Play logic
    auto instrName = gwidi::options::InstrumentOptions::nameForInstrument(gwidi::options::InstrumentOptions::Instrument::HARP);
    auto &instrumentMapping = gwidi::options::InstrumentOptions::getInstance().getMapping().instrumentMapping[instrName];

    starting_octave = instrumentMapping.startingOctave;

    auto startTime = curTime();
    bool endReached = false;
    while(!endReached) {
        auto cTime = curTime();
        auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(cTime - startTime).count();

//        spdlog::debug("startTime: {}, cTime: {}, delta: {}", startTime.count(), cTime.count(), deltaTime);
        startTime = curTime();
        auto action = tickHandler.processTick(deltaTime);

        spdlog::debug("BEGIN Action notes-------");
        spdlog::debug("Chosen octave: {}", action->chosen_octave);
        int delay = octaveSwapDelay(action);
        while(action->chosen_octave != -1 && action->chosen_octave != starting_octave) {
            if(action->chosen_octave > starting_octave) {
                swapOctaveUp();
            }
            else if(action->chosen_octave < starting_octave) {
                swapOctaveDown();
                // Need some delay if we are swapping multiple octaves, seems like the time it takes to swap the skill bar
                // interferes with doing multiple octave swaps quickly
            }
            spdlog::info("waiting octave swap delay: {}", delay);
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            delay = octaveSwapDelay(action);    // update to reduce the next swap delay if needed
        }
        for(auto &n : action->notes) {
            spdlog::debug("note: {}, start_offset: {}, duration: {}", n->note.letter, n->note.start_offset, n->note.duration);

            // for testing, react to the actions
            if(n->note.octave == action->chosen_octave) {
                // pick our key from the note
                spdlog::info("Sending input key: {}", n->note.key);
                sendInput(n->note.key);
            }
        }
        spdlog::debug("END Action notes---------");

        endReached = action->end_reached;
        if(action->end_reached) {
            spdlog::info("Reached end of track, exiting...");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds (10));
    }
    delete data;
    delete input;
    return 0;
}
