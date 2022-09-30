#include "GwidiTickHandler.h"
#include "GwidiOptions.h"
#include "spdlog/spdlog.h"
#include <chrono>

#if defined(WIN32) || defined(WIN64)
#include "Winuser.h"
#elif defined(__linux__)
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

void sendInput(int key) {
    // TODO
}

int keyToHk(const std::string& key) {
    auto it = hk_map.find(key);
    if(it == hk_map.end()) {
        return 0x00;   // just some dummy for now
    }
    return it->second;
}

#endif

// used to determine when to swap octaves
int starting_octave = 0;
void swapOctaveUp() {
    spdlog::debug("--BEGIN swapping octave up--");
    spdlog::debug("old octave: {}", starting_octave);
    sendInput(keyToHk("0"));
    starting_octave++;
    spdlog::debug("new octave: {}", starting_octave);
    spdlog::debug("--END swapping octave up--");
}

void swapOctaveDown() {
    spdlog::debug("--BEGIN swapping octave down--");
    spdlog::debug("old octave: {}", starting_octave);
    sendInput(keyToHk("9"));
    starting_octave--;
    spdlog::debug("new octave: {}", starting_octave);
    spdlog::debug("--END swapping octave down--");
}


// TODO: Refactor to a playback class to respond to play actions
int main() {
    spdlog::set_level(spdlog::level::debug);

    auto data = GwidiMidiParser::getInstance().readFile(R"(E:\Tools\repos\gwidi_midi_parser\assets\test2_data.mid)", gwidi::options::MidiParseOptions{
            gwidi::options::InstrumentOptions::Instrument::HARP
    });
    auto tickHandler = GwidiTickHandler();
    tickHandler.setOptions(GwidiTickOptions{
        GwidiTickOptions::ActionOctaveBehavior::HIGHEST,
        gwidi::options::InstrumentOptions::Instrument::HARP
    });
    tickHandler.assignData(data);

    // Play logic
    auto instrName = gwidi::options::InstrumentOptions::nameForInstrument(gwidi::options::InstrumentOptions::Instrument::HARP);
    auto &instrumentMapping = gwidi::options::InstrumentOptions::getInstance().getMapping().instrumentMapping[instrName];

    starting_octave = instrumentMapping.startingOctave;
    auto startTime = clock();
    for(auto i = 0; i < 300; i++) {
        auto deltaTime = clock() - startTime;
        startTime = clock();
        auto action = tickHandler.processTick(deltaTime);

        spdlog::debug("BEGIN Action notes-------");
        spdlog::debug("Chosen octave: {}", action->chosen_octave);
        while(action->chosen_octave != -1 && action->chosen_octave != starting_octave) {
            if(action->chosen_octave > starting_octave) {
                swapOctaveUp();
            }
            else if(action->chosen_octave < starting_octave) {
                swapOctaveDown();
            }
        }
        for(auto &n : action->notes) {
            spdlog::debug("note: {}, start_offset: {}, duration: {}", n->note.letter, n->note.start_offset, n->note.duration);

            // for testing, react to the actions
            if(n->note.octave == action->chosen_octave) {
                // TODO: Get the proper key from the GwidiData instrument definitions
                // TODO: Use the currently assigned instrument in gwidi options to determine which one to use
                // TODO: Need to still determine which octaves we are assigning to the ones available for the instrument
                sendInput(keyToHk(n->instrumentAction.key));
            }
        }
        spdlog::debug("END Action notes---------");

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    delete data;
    return 0;
}
