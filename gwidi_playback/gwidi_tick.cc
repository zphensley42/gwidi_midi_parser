#include "GwidiTickHandler.h"
#include "GwidiOptions.h"
#include "spdlog/spdlog.h"
#include <chrono>

#if defined(WIN32) || defined(WIN64)
#include "Winuser.h"

#define TEST_FILE R"(E:\Tools\repos\gwidi_midi_parser\assets\test2_data.mid)"
#elif defined(__linux__)
#include "LinuxSendInput.h"

//#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/test3_data.mid)"
//#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/pollyanna7.mid)"
//#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/super_mario.mid)"
//#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/undertale_snowy.mid)"
//#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/whats_new_scooby_doo.mid)"
#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/moana.mid)"
#endif


#if defined(WIN32) || defined(WIN64)

#define VK_0key 0x30
#define VK_1key 0x31
#define VK_2key 0x32
#define VK_3key 0x33
#define VK_4key 0x34
#define VK_5key 0x35
#define VK_6key 0x36
#define VK_7key 0x37
#define VK_8key 0x38
#define VK_9key 0x39

#define HK_0key 0x0b
#define HK_1key 0x02
#define HK_2key 0x03
#define HK_3key 0x04
#define HK_4key 0x05
#define HK_5key 0x06
#define HK_6key 0x07
#define HK_7key 0x08
#define HK_8key 0x09
#define HK_9key 0x0a
static std::unordered_map<std::string, int> vk_map {
        {"0", VK_0key}, // octave up
        {"1", VK_1key},
        {"2", VK_2key},
        {"3", VK_3key},
        {"4", VK_4key},
        {"5", VK_5key},
        {"6", VK_6key},
        {"7", VK_7key},
        {"8", VK_8key},
        {"9", VK_9key}, // octave down
};

static std::unordered_map<std::string, int> hk_map {
        {"0", HK_0key}, // octave up
        {"1", HK_1key},
        {"2", HK_2key},
        {"3", HK_3key},
        {"4", HK_4key},
        {"5", HK_5key},
        {"6", HK_6key},
        {"7", HK_7key},
        {"8", HK_8key},
        {"9", HK_9key}, // octave down
};

int keyToVk(const std::string& key) {
    auto it = vk_map.find(key);
    if(it == vk_map.end()) {
        return VK_CANCEL;   // just some dummy for now
    }
    return it->second;
}

int keyToHk(const std::string& key) {
    auto it = hk_map.find(key);
    if(it == hk_map.end()) {
        return 0x00;   // just some dummy for now
    }
    return it->second;
}

void sendInput(int key) {
    INPUT inputs[2] = {};
    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.time = 0;
    inputs[0].ki.wVk = 0;
    inputs[0].ki.dwExtraInfo = 0;

    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;
    inputs[0].ki.wScan = key;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.time = 0;
    inputs[1].ki.wVk = 0;
    inputs[1].ki.dwExtraInfo = 0;

    inputs[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    inputs[1].ki.wScan = key;

    UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if(uSent != ARRAYSIZE(inputs)) {
        spdlog::debug("Failed to SendInput: {}", HRESULT_FROM_WIN32(GetLastError()));
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
    input = new SendInput();

    spdlog::set_level(spdlog::level::info);

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
                // TODO: Get the proper key from the GwidiData instrument definitions
                // TODO: Use the currently assigned instrument in gwidi options to determine which one to use
                // TODO: Need to still determine which octaves we are assigning to the ones available for the instrument

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
