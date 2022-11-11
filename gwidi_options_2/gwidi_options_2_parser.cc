#include "GwidiOptions2.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <spdlog/spdlog.h>
#include <linux/input-event-codes.h>
#include <strutil.h>


#if defined(WIN32) || defined(WIN64)
#define CONFIG_DIR R"(E:\Tools\repos\gwidi_midi_parser\config)"
#elif defined(__linux__)
#define CONFIG_DIR R"(/home/zhensley/repos/gwidi_godot/gwidi_midi_parser/config)"
#endif


using namespace nlohmann;

namespace gwidi {
namespace options2 {


GwidiOptions2::GwidiOptions2() {
    parseConfigs();
}

GwidiOptions2 &GwidiOptions2::getInstance() {
    static GwidiOptions2 instance;
    return instance;
}

void GwidiOptions2::parseConfigs() {
    {
        std::stringstream ss;
        ss << CONFIG_DIR << "/instruments/manifest.json";
        std::ifstream instrumentsConfigFile(ss.str());
        json instrumentsJson;
        instrumentsConfigFile >> instrumentsJson;
        for(auto &instrument : instrumentsJson) {
            spdlog::debug("Instrument: {}", instrument);

            std::stringstream ss2;
            ss2 << CONFIG_DIR << "/instruments/" << instrument.get<std::string>() << ".json";
            std::string instrFileName = ss2.str();
            std::ifstream instrumentConfigFile(instrFileName);
            json instrumentJson;
            instrumentConfigFile >> instrumentJson;

            Instrument i;
            i.supports_held_notes = instrumentJson["supports_held_notes"];
            i.starting_octave = instrumentJson["starting_octave"];
            for(auto& octave : instrumentJson["octaves"]) {
                Octave o;
                o.num = octave["num"];
                for(auto& note : octave["notes"]) {
                    Note n;
                    n.midi_octave = note["midi_octave"];
                    n.instrument_octave = o.num;
                    n.key = note["key"];
                    for(auto &letter : note["letters"]) {
                        n.letters.emplace_back(letter);
                    }
                    o.notes.emplace_back(n);
                }
                i.octaves.emplace_back(o);
            }
            instruments[instrument] = i;

            instrumentConfigFile.close();
        }
        instrumentsConfigFile.close();
    }

    {
        std::stringstream ss;
        ss << CONFIG_DIR << "/playback.json";
        std::ifstream playbackConfigFile(ss.str());
        json playbackJson;
        playbackConfigFile >> playbackJson;
        m_tempo = playbackJson["tempo"].get<double>();
        playbackConfigFile.close();
    }
}

GwidiOptions2::operator std::string() const {
    std::stringstream ss;
    for(auto &instrumentMapEntry : instruments) {
        ss << instrumentMapEntry.first + "\n";
        ss << "\tsupports_held_notes: " << (instrumentMapEntry.second.supports_held_notes ? "true" : "false") << "\n";
        ss << "\tstarting_octave: " << instrumentMapEntry.second.starting_octave << "\n";
        ss << "\toctaves:\n";
        for(auto &octave : instrumentMapEntry.second.octaves) {
            ss << "\t\tnum: " << octave.num << "\n";
            ss << "\t\tnotes:\n";
            for(auto &note : octave.notes) {
                ss << "\t\t\tmidi_octave: " << note.midi_octave << "\n";
                ss << "\t\t\tkey: " << note.key << "\n";
                ss << "\t\t\tletters:\n";
                for(auto &letter : note.letters) {
                    ss << "\t\t\t\t" << letter << "\n";
                }
            }
        }
    }
    return ss.str();
}

Note GwidiOptions2::optionsNoteFromMidiNote(const std::string &instrument, int in_midiOctave, const std::string &letter) {
    for(auto &octave : instruments[instrument].octaves) {
        for(auto &note : octave.notes) {
            if(note.midi_octave == in_midiOctave && std::find(note.letters.begin(), note.letters.end(), letter) != note.letters.end()) {
                return note;
            }
        }
    }
    return Note {
            {},
            -1,
            -1,
            ""
    };
}

double GwidiOptions2::tempo() {
    return m_tempo;
}

void GwidiOptions2::addNewConfig(const std::string &configInstrumentName, const Instrument &instrument) {
    instruments[configInstrumentName] = instrument;

    storeConfigs();
}

void GwidiOptions2::storeConfigs() {
    // write each to file
    for(auto &entry : instruments) {
        json instrumentJson;
        instrumentJson["supports_held_notes"] = entry.second.supports_held_notes;
        instrumentJson["starting_octave"] = entry.second.starting_octave;
        instrumentJson["octaves"] = json::array();
        for(auto &entryOctave : entry.second.octaves) {
            json octaveJson;
            octaveJson["num"] = entryOctave.num;
            octaveJson["notes"] = json::array();
            for(auto &entryNote : entryOctave.notes) {
                json noteJson;
                noteJson["midi_octave"] = entryNote.midi_octave;
                noteJson["key"] = entryNote.key;

                json lettersJson = json::array();
                for(auto &letter : entryNote.letters) {
                    lettersJson.emplace_back(letter);
                }
                noteJson["letters"] = lettersJson;

                octaveJson["notes"].emplace_back(noteJson);
            }
            instrumentJson["octaves"].emplace_back(octaveJson);
        }

        std::stringstream outName;
        outName << CONFIG_DIR << "/instruments/" << entry.first << ".json";
        std::ofstream instrumentJsonFile(outName.str());
        instrumentJsonFile << instrumentJson.dump(4);
        instrumentJsonFile.close();
    }

    // write manifest to file
    {
        json instrumentsJson = json::array();
        for(auto &entry : instruments) {
            instrumentsJson.emplace_back(entry.first);
        }

        std::stringstream outName;
        outName << CONFIG_DIR << "/instruments/manifest.json";
        std::ofstream instrumentsJsonFile(outName.str());
        instrumentsJsonFile << instrumentsJson.dump(4);
        instrumentsJsonFile.close();
    }
}

void GwidiOptions2::removeConfig(const std::string &configInstrumentName) {
    auto instrEntry = instruments.find(configInstrumentName);
    if(instrEntry != instruments.end()) {
        instruments.erase(instrEntry, instruments.end());
    }
    storeConfigs();
}


std::map<std::string, int> HotkeyOptions::m_keyNameMapping{};

HotkeyOptions::HotkeyOptions() {
    // ASCII codes go from 0 -> 9 (48 -> 57)
    // Linux input codes go from 1 -> 0 (2 -> 11)
    // There isn't a straightforward approach to converting, just make it explicit
    m_keyNameMapping["0"] = KEY_0;
    m_keyNameMapping["1"] = KEY_1;
    m_keyNameMapping["2"] = KEY_2;
    m_keyNameMapping["3"] = KEY_3;
    m_keyNameMapping["4"] = KEY_4;
    m_keyNameMapping["5"] = KEY_5;
    m_keyNameMapping["6"] = KEY_6;
    m_keyNameMapping["7"] = KEY_7;
    m_keyNameMapping["8"] = KEY_8;
    m_keyNameMapping["9"] = KEY_9;

    // Same deal as above, ASCII -> Linux input doesn't match the same code format because of the numbers
    m_keyNameMapping["a"] = KEY_A;
    m_keyNameMapping["b"] = KEY_B;
    m_keyNameMapping["c"] = KEY_C;
    m_keyNameMapping["d"] = KEY_D;
    m_keyNameMapping["e"] = KEY_E;
    m_keyNameMapping["f"] = KEY_F;
    m_keyNameMapping["g"] = KEY_G;
    m_keyNameMapping["h"] = KEY_H;
    m_keyNameMapping["i"] = KEY_I;
    m_keyNameMapping["j"] = KEY_J;
    m_keyNameMapping["k"] = KEY_K;
    m_keyNameMapping["l"] = KEY_L;
    m_keyNameMapping["m"] = KEY_M;
    m_keyNameMapping["n"] = KEY_N;
    m_keyNameMapping["o"] = KEY_O;
    m_keyNameMapping["p"] = KEY_P;
    m_keyNameMapping["q"] = KEY_Q;
    m_keyNameMapping["r"] = KEY_R;
    m_keyNameMapping["s"] = KEY_S;
    m_keyNameMapping["t"] = KEY_T;
    m_keyNameMapping["u"] = KEY_U;
    m_keyNameMapping["v"] = KEY_V;
    m_keyNameMapping["w"] = KEY_W;
    m_keyNameMapping["x"] = KEY_X;
    m_keyNameMapping["y"] = KEY_Y;
    m_keyNameMapping["z"] = KEY_Z;


    // Add the other misc. keys (shift, ctrl, space, enter, esc, etc.)
    m_keyNameMapping["left shift"] = KEY_LEFTSHIFT;
    m_keyNameMapping["left ctrl"] = KEY_LEFTCTRL;
    m_keyNameMapping["left alt"] = KEY_LEFTALT;
    m_keyNameMapping["right shift"] = KEY_RIGHTSHIFT;
    m_keyNameMapping["right ctrl"] = KEY_RIGHTCTRL;
    m_keyNameMapping["right alt"] = KEY_RIGHTALT;
    m_keyNameMapping["space"] = KEY_SPACE;
    m_keyNameMapping["enter"] = KEY_ENTER;
    m_keyNameMapping["escape"] = KEY_ESC;
    parseConfig();
}

int HotkeyOptions::keyNameToCode(const std::string& key) {
    return m_keyNameMapping[key];
}

void HotkeyOptions::parseConfig() {
    m_hotkeyMapping.clear();

    std::stringstream ss;
    ss << CONFIG_DIR << "/hotkeys.json";
    std::ifstream hotkeysConfigFile(ss.str());
    json hotkeysJson;
    hotkeysConfigFile >> hotkeysJson;

    for(auto &entry : hotkeysJson.items()) {
        const auto& name = entry.key();
        auto keys = entry.value().get<std::string>();

        std::vector<int> keysVector;
        auto split = strutil::ss::splitSV(keys, "+");
        for(auto &keyName : split) {
            keysVector.emplace_back(keyNameToCode(keyName));
        }
        auto keysHash = hashFromKeys(keysVector);
        m_hotkeyMapping[keysHash] = HotKey{
            name,
            keysVector
        };
    }
}

HotkeyOptions &HotkeyOptions::getInstance() {
    static HotkeyOptions options;
    return options;
}

std::size_t HotkeyOptions::hashFromKeys(const std::vector<int> &keys) {
    std::size_t retH{0};
    int index = 0;
    for(auto &k : keys) {
        std::size_t h = std::hash<int>{}(k);
        retH = retH ^ (h << index);
        index++;
    }
    return retH;
}

std::string HotkeyOptions::codeToKeyName(int code) {
    auto it = std::find_if(m_keyNameMapping.begin(), m_keyNameMapping.end(), [code](std::pair<std::string, int> pair){
        return pair.second == code;
    });
    if(it != m_keyNameMapping.end()) {
        return it->first;
    }
    return "";
}

void HotkeyOptions::updateMapping(const HotKey& key) {
    auto it = std::find_if(m_hotkeyMapping.begin(), m_hotkeyMapping.end(), [&key](const std::pair<std::size_t, HotkeyOptions::HotKey>& entry){
        return entry.second.name == key.name;
    });
    // First, remove any existing entries matching the hotkey name
    if(it != m_hotkeyMapping.end()) {
        m_hotkeyMapping.erase(it);
    }

    // Then, add our new hotkey
    auto keysHash = hashFromKeys(key.keys);
    m_hotkeyMapping[keysHash] = key;

    storeConfig();
}

void HotkeyOptions::storeConfig() {
    json hotkeysJson;
    for(auto &entry : m_hotkeyMapping) {
        std::stringstream ss;
        bool first = true;
        for(auto &k : entry.second.keys) {
            if(!first) {
                ss << "+";
            }
            ss << codeToKeyName(k);
            first = false;
        }
        hotkeysJson[entry.second.name] = ss.str();
    }

    std::stringstream outName;
    outName << CONFIG_DIR << "/hotkeys.json";
    std::ofstream hotkeysJsonFile(outName.str());
    hotkeysJsonFile << hotkeysJson;
    hotkeysJsonFile.close();
}

void HotkeyOptions::reloadConfig() {
    parseConfig();
}

}
}
