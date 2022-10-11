#include "GwidiOptions2.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <spdlog/spdlog.h>


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
    std::stringstream ss;
    ss << CONFIG_DIR << "/instruments.json";
    std::ifstream instrumentsConfigFile(ss.str());
    json instrumentsJson;
    instrumentsConfigFile >> instrumentsJson;
    for(auto &instrument : instrumentsJson) {
        spdlog::debug("Instrument: {}", instrument);

        std::stringstream ss2;
        ss2 << CONFIG_DIR << "/" << instrument.get<std::string>() << ".json";
        std::string instrFileName = ss2.str();
        std::ifstream instrumentConfigFile(instrFileName);
        json instrumentJson;
        instrumentConfigFile >> instrumentJson;

        // TODO: Parse the instrument
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


}
}
