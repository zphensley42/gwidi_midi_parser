#include "gwidi_midi_parser.h"
#include "spdlog/spdlog.h"
#include "GwidiOptions.h"

void testWriteAndRead(GwidiData* in) {
    in->writeToFile("test_out.gwd");
    auto readData = GwidiData::readFromFile("test_out.gwd");

    auto &tickMap = readData->getTickMap();
    spdlog::debug("printing tick map, size: {}", tickMap.size());
    for(auto &entry : tickMap) {
        spdlog::debug("track: {}, # entries: {}", entry.first, entry.second.size());
        for(auto &trackEntry : entry.second) {
            spdlog::debug("\tstart_offset: {}, # of notes: {}", trackEntry.first, trackEntry.second.size());
            for(auto &n : trackEntry.second) {
                spdlog::debug("\t\tnote: {}, duration: {}, instrumentOctave: {}, instrumentNoteNumber: {}, instrumentKey: {}", n.letter, n.duration, n.octave, n.instrument_note_number, n.key);
            }
        }
    }

    FMT_ASSERT(in->getTempo() == readData->getTempo(), "tempo does not match");
    FMT_ASSERT(in->getTempoMicro() == readData->getTempoMicro(), "tempoMicro does not match");

    auto &tracks1 = in->getTracks();
    auto &tracks2 = readData->getTracks();
    FMT_ASSERT(tracks1.size() == tracks2.size(), "track count does not match");

    FMT_ASSERT(*in == *readData, "data does not match!");
}

int main() {
    spdlog::set_level(spdlog::level::debug);

//    auto data = GwidiMidiParser::getInstance().readFile(R"(E:\Tools\repos\gwidi_midi_parser\assets\pollyanna.mid)");
    auto data = GwidiMidiParser::getInstance().readFile(R"(E:\Tools\repos\gwidi_midi_parser\assets\test2_data.mid)", gwidi::options::MidiParseOptions{
            gwidi::options::InstrumentOptions::Instrument::HARP
    });
    auto& tracks = data->getTracks();
    for(auto &t : tracks) {
        spdlog::debug("track: {}, tempo: [{}, {}], instrument: {}, # notes: {}", t.track_name, data->getTempo(), data->getTempoMicro(), t.instrument_name, t.notes.size());
    }

    testWriteAndRead(data);

    delete data;
    return 0;
}
