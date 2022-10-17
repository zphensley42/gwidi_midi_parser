#include "gwidi_midi_parser.h"
#include "spdlog/spdlog.h"
#include "GwidiOptions2.h"
#include "GwidiGuiData.h"
#include "GwidiDataConverter.h"

#if defined(WIN32) || defined(WIN64)
#define TEST_FILE R"(E:\Tools\repos\gwidi_midi_parser\assets\test2_data.mid)"
#elif defined(__linux__)
#define TEST_FILE R"(/home/zhensley/repos/gwidi_godot/gwidi_midi_parser/assets/slow_scale.mid)"
#endif


void testWriteAndRead(gwidi::data::midi::GwidiMidiData* in) {
    in->writeToFile("test_out.gwd");
    auto readData = gwidi::data::midi::GwidiMidiData::readFromFile("test_out.gwd");

    auto &tickMap = readData->getTickMap();
    spdlog::debug("printing tick map, size: {}", tickMap.size());
    for(auto &entry : tickMap) {
        spdlog::debug("time: {}, # of notes: {}", entry.first, entry.second.size());
        for(auto &n : entry.second) {
            spdlog::debug("\t\tnote: {}, duration: {}, instrumentOctave: {}, instrumentKey: {}", n.letter, n.duration, n.octave, n.key);
        }
    }

    FMT_ASSERT(in->getTempo() == readData->getTempo(), "tempo does not match");
    FMT_ASSERT(in->getTempoMicro() == readData->getTempoMicro(), "tempoMicro does not match");

    auto &tracks1 = in->getTracks();
    auto &tracks2 = readData->getTracks();
    FMT_ASSERT(tracks1.size() == tracks2.size(), "track count does not match");

    FMT_ASSERT(*in == *readData, "data does not match!");
}

void testConversionMidiToGui() {
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

    auto midiData = gwidi::data::GwidiDataConverter::getInstance().guiToMidi(data);

    FMT_ASSERT(midiData->getTracks().size() == 1, "# of tracks does not match expected");
    FMT_ASSERT(midiData->getTracks().front().notes.size() == 32, fmt::format("# of notes does not match expected: 32 vs {}", midiData->getTracks().front().notes.size()).c_str());

    // Compare the old and new tick maps (should functionally be the same)
    auto &midiTickMap = midiData->getTickMap();
    auto &guiTickMap = data->getTickMap();

    FMT_ASSERT(midiTickMap.size() == guiTickMap.size(), "Tick maps did not match!");

    auto midiTickIt = midiTickMap.begin();
    auto guiTickIt = guiTickMap.begin();
    for(auto i = 0; i < midiTickMap.size(); i++) {
        auto midiTime = midiTickIt->first;
        auto guiTime = guiTickIt->first;
        auto epsilon = 0.0001;  // Required due to floating point comparison
        FMT_ASSERT(fabs(midiTime - guiTime) < epsilon, fmt::format("Tick times did not match: {} vs {}", midiTickIt->first, guiTickIt->first).c_str());
        FMT_ASSERT(midiTickIt->second.size() == guiTickIt->second.size(), "Tick size of notes did not match");

        for(auto j = 0; j < midiTickIt->second.size(); j++) {
            auto &midiNote = midiTickIt->second.at(j);
            auto &guiNote = guiTickIt->second.at(j);
            FMT_ASSERT(midiNote.key == guiNote.key, "Notes did not match (key)");
            FMT_ASSERT(midiNote.octave == guiNote.octave, "Notes did not match (key)");
            FMT_ASSERT(std::find(guiNote.letters.begin(), guiNote.letters.end(), midiNote.letter) != guiNote.letters.end(), "Notes did not match (letter)");
        }
        midiTickIt++;
        guiTickIt++;
    }

    delete data;
    delete midiData;
}

void testConversionGuiToMidi() {
    auto data = gwidi::midi::GwidiMidiParser::getInstance().readFile(TEST_FILE, gwidi::midi::MidiParseOptions{
            gwidi::midi::Instrument::HARP,
            1
    });
    auto guiData = gwidi::data::GwidiDataConverter::getInstance().midiToGui(data);

    FMT_ASSERT(guiData->getMeasures().size() == 6, "# of measures does not match expected");

    // Compare the old and new tick maps (should functionally be the same)
    auto &midiTickMap = data->getTickMap();
    auto &guiTickMap = guiData->getTickMap();

    FMT_ASSERT(midiTickMap.size() == guiTickMap.size(), "Tick maps did not match!");

    auto midiTickIt = midiTickMap.begin();
    auto guiTickIt = guiTickMap.begin();
    for(auto i = 0; i < midiTickMap.size(); i++) {
        auto midiTime = midiTickIt->first;
        auto guiTime = guiTickIt->first;
        auto epsilon = 0.0001;  // Required due to floating point comparison
        FMT_ASSERT(fabs(midiTime - guiTime) < epsilon, fmt::format("Tick times did not match: {} vs {}", midiTickIt->first, guiTickIt->first).c_str());
        FMT_ASSERT(midiTickIt->second.size() == guiTickIt->second.size(), "Tick size of notes did not match");

        for(auto j = 0; j < midiTickIt->second.size(); j++) {
            auto &midiNote = midiTickIt->second.at(j);
            auto &guiNote = guiTickIt->second.at(j);
            FMT_ASSERT(midiNote.key == guiNote.key, "Notes did not match (key)");
            FMT_ASSERT(midiNote.octave == guiNote.octave, "Notes did not match (key)");
            FMT_ASSERT(std::find(guiNote.letters.begin(), guiNote.letters.end(), midiNote.letter) != guiNote.letters.end(), "Notes did not match (letter)");
        }
        midiTickIt++;
        guiTickIt++;
    }

    delete guiData;
    delete data;
}

int main() {
    spdlog::set_level(spdlog::level::debug);

//    auto data = GwidiMidiParser::getInstance().readFile(R"(E:\Tools\repos\gwidi_midi_parser\assets\pollyanna.mid)");
    auto data = gwidi::midi::GwidiMidiParser::getInstance().readFile(TEST_FILE, gwidi::midi::MidiParseOptions{
        gwidi::midi::Instrument::HARP,
        1
    });
    auto& tracks = data->getTracks();
    for(auto &t : tracks) {
        spdlog::debug("track: {}, tempo: [{}, {}], instrument: {}, # notes: {}", t.track_name, data->getTempo(), data->getTempoMicro(), t.instrument_name, t.notes.size());
    }

    testWriteAndRead(data);
    testConversionMidiToGui();
    testConversionGuiToMidi();

    delete data;
    return 0;
}
