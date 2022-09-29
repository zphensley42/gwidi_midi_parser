#include <iostream>
#include <sstream>
#include <fstream>
#include "GwidiOptions.h"
#include "spdlog/spdlog.h"
#include "inipp.h"
#include <string_view>
#include "util.h"

#define CONFIG_DIR R"(E:\Tools\repos\gwidi_midi_parser\config)"

namespace gwidi::options {

const char *InstrumentOptions::nameForInstrument(InstrumentOptions::Instrument in) {
    static std::unordered_map<Instrument, const char *> instrument_name{
            {Instrument::HARP,  "harp"},
            {Instrument::FLUTE, "flute"},
            {Instrument::BELL,  "bell"},
    };
    auto it = instrument_name.find(in);
    if (it == instrument_name.end()) {
        return "";
    }
    return it->second;
}

InstrumentOptions::Instrument InstrumentOptions::enumForInstrument(const char *in) {
    static std::unordered_map<const char *, Instrument> instrument_enum{
            {"harp",  Instrument::HARP},
            {"flute", Instrument::FLUTE},
            {"bell",  Instrument::BELL},
    };
    auto it = instrument_enum.find(in);
    if (it == instrument_enum.end()) {
        return Instrument::UNKNOWN;
    }
    return it->second;
}

InstrumentOptions &InstrumentOptions::getInstance() {
    static InstrumentOptions instance;
    return instance;
}

void InstrumentOptions::fillMapping() {
    // Parse our ini files for the instruments
    // for each instrument, build mapping per note of instrument octave -> midi octave
    inipp::Ini<char> ini;
    std::stringstream ss;
    ss << CONFIG_DIR << "\\instruments.ini";
    std::ifstream is(ss.str());
    ini.parse(is);

    // Use the parsed file to build the map
    std::string delim = ",";
    std::string instrumentData;
    for (auto &section: ini.sections) {
        spdlog::debug("InstrumentOptions2 instruments.ini section: {}", section.first);
        if (section.first == "instruments") {
            for (auto &sectionEntry: section.second) {
                if (sectionEntry.first == "delim") {
                    delim = sectionEntry.second;
                } else if (sectionEntry.first == "instrument") {
                    instrumentData = sectionEntry.second;
                }
                spdlog::debug("\tInstrumentOptions2 instruments.ini entry: {}", sectionEntry.first);
                spdlog::debug("\tInstrumentOptions2 instruments.ini value: {}", sectionEntry.second);
            }
        }
    }
    std::string_view instrumentView{instrumentData};
    std::string_view delimView{delim};
    auto instruments = splitSV(instrumentView, delimView);

    mapping.instrumentMapping.clear();

    for (auto &instrument: instruments) {
        inipp::Ini<char> instrumentIni;
        std::stringstream ss2;
        ss2 << CONFIG_DIR << "\\" << instrument << ".ini";
        std::ifstream is2(ss2.str());
        instrumentIni.parse(is2);

        mapping.instrumentMapping[std::string{instrument}] = InstrumentOctaves{};
        InstrumentOctaves &octaves = mapping.instrumentMapping[std::string{instrument}];
        octaves.keys.clear();

        // default keys
        for(auto i = 1; i < 9; i++) {
            octaves.keys[i] = std::to_string(i);
        }

        int numOctaves = 0;
        bool supportsHeldNotes = false;
        for (auto &section: instrumentIni.sections) {
            if (section.first == "instrument") {
                for (auto &entry: section.second) {
                    if (entry.first == "numOctaves") {
                        numOctaves = std::stoi(entry.second);
                    } else if (entry.first == "supportsHeldNotes") {
                        supportsHeldNotes = entry.second == "true";
                    }
                    else if(entry.first == "keys") {
                        octaves.keys.clear();
                        std::string_view keysView{entry.second};
                        auto keysSplit = splitSV(keysView, {","});
                        for(auto i = 0; i < keysSplit.size(); i++) {
                            octaves.keys[i] = std::string{keysSplit.at(i)};
                        }
                    }
                    else if(entry.first == "startingOctave") {
                        octaves.startingOctave = std::stoi(entry.second);
                    }
                }
            } else {
                for (auto i = 0; i < numOctaves; i++) {
                    std::string octaveSection = "octave" + std::to_string(i);
                    if (section.first == octaveSection) {

                        octaves.octaves[i] = InstrumentOctaveNotes{};
                        InstrumentOctaveNotes &octaveNotes = octaves.octaves[i];
//                        [octave2]
//                        1=5:C,C#
//                        2=5:D,D#
//                        3=5:E
//                        4=5:F,F#
//                        5=5:G,G#
//                        6=5:A,A#
//                        7=5:B
//                        8=6:C,C#

                        for (auto &entry: section.second) {
                            // each entry is keyed by its instrument note
                            // each entry's value is a mapping of its octave and letters
                            int noteVal = std::stoi(entry.first);

                            octaveNotes.notes[noteVal] = std::vector<InstrumentNote>();

                            std::string_view instrumentNoteStrView{entry.second};
                            auto noteSettingSplit = splitSV(instrumentNoteStrView, {":"});
                            // 0 == midi octave
                            // 1 = letters
                            int noteMidiOctave = std::stoi(std::string{noteSettingSplit[0]});
                            octaveNotes.notes[noteVal].emplace_back(InstrumentNote{
                                    noteMidiOctave,
                                    {}
                            });
                            auto &note = octaveNotes.notes[noteVal].back();
                            auto noteLetterSplit = splitSV(noteSettingSplit[1], {","});
                            for(auto &noteLetter : noteLetterSplit) {
                                note.letters.emplace_back(std::string{noteLetter});
                            }
                        }
                    }
                }
            }
        }

        is2.close();
    }

    is.close();

    for (auto &entry: mapping.instrumentMapping) {
        spdlog::debug("mapping instrument: {}", entry.first);

        for (auto &octaveEntry: entry.second.octaves) {
            spdlog::debug("\toctave: {}", octaveEntry.first);

            spdlog::debug("\t--note -> {midi octave, letters} mapping--");
            for (auto &noteEntry: octaveEntry.second.notes) {
                spdlog::debug("\t\t{}", noteEntry.first);
                for(auto &noteSettingEntry : noteEntry.second) {
                    spdlog::debug("\t\t\tmidiOctave: {}", noteSettingEntry.midiOctave);
                    spdlog::debug("\t\t\t--letters--");
                    for(auto &letterEntry : noteSettingEntry.letters) {
                        spdlog::debug("\t\t\t\t{}", letterEntry);
                    }
                }
            }
        }
    }
}

InstrumentOptions::InstrumentOptions() {
    fillMapping();
}

std::vector<int> InstrumentOptions::supportedMidiOctaves(const std::string &instrument) {
    std::unordered_map<int, int> midiOctaves;

    for(auto &entry : mapping.instrumentMapping[instrument].octaves) {
        for(auto &innerEntry : entry.second.notes) {
            for(auto &innerInnerEntry : innerEntry.second) {
                if(midiOctaves.find(innerInnerEntry.midiOctave) == midiOctaves.end()) {
                    midiOctaves[innerInnerEntry.midiOctave] = 0;
                }
                midiOctaves[innerInnerEntry.midiOctave]++;
            }
        }
    }

    std::vector<int> ret;
    for(auto &entry : midiOctaves) {
        ret.emplace_back(entry.first);
    }

    return ret;
}

InstrumentOptions::InstrumentNoteAttributes InstrumentOptions::instrumentNoteAttributesForMidi(const std::string &instrument, int in_midiOctave, const std::string &letter) {
    InstrumentNoteAttributes ret;

    auto &instrumentMapping =  mapping.instrumentMapping[instrument];
    for(auto &mappingEntry : instrumentMapping.octaves) {
        int instrumentOctave = mappingEntry.first;
        for(auto &innerEntry : mappingEntry.second.notes) {
            int instrumentNote = innerEntry.first;
            for(auto &innerInnerEntry : innerEntry.second) {
                int midiOctave = innerInnerEntry.midiOctave;
                if(midiOctave == in_midiOctave) {
                    auto it = std::find(innerInnerEntry.letters.begin(), innerInnerEntry.letters.end(), letter);
                    if(it != innerInnerEntry.letters.end()) {
                        // Both midi octave and letter match {Octave 4, C#} for example
                        ret.letters = innerInnerEntry.letters;
                        ret.instrumentOctave = instrumentOctave;
                        ret.instrumentNoteNumber = instrumentNote;
                        ret.key = instrumentMapping.keys[instrumentNote];
                        return ret;
                    }
                }
            }
        }
    }

    return ret;
}

}