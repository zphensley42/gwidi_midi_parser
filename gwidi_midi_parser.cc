#include <iostream>
#include <sstream>
#include <fstream>
#include "spdlog/spdlog.h"
#include "MidiFile.h"
#include "Options.h"
#include "GwidiData.h"

GwidiData::GwidiData(const std::vector<Track>& tracks) {
    this->tracks.clear();
    for(auto &t : tracks) {
        this->tracks.emplace_back(Track(t));
    }
}

GwidiData::GwidiData(std::vector<Track>&& tracks) {
    this->tracks = std::move(tracks);
}

void GwidiData::assignNotes(int track, const std::vector<Note>& in_notes) {
    this->tracks[track] = Track();
    for(auto &note : in_notes) {
        this->tracks[track].notes.emplace_back(Note(note));
    }
}

void GwidiData::addNote(int track, Note &note) {
    if(track >= 0 && track < this->tracks.size()) {
        this->tracks[track].notes.emplace_back(Note(note));
    }
}

void GwidiData::assignTempo(double t, double tm) {
    this->tempo = t;
    this->tempoMicro = tm;
}

void GwidiData::addTrack(std::string instrument, std::string track_name, const std::vector<Note> &notes) {
    this->tracks.emplace_back(Track{
        std::vector<Note>(),
            std::move(instrument),
            std::move(track_name)
    });
    for(auto &n : notes) {
        this->tracks.back().notes.emplace_back(Note(n));
    }
}

GwidiData* readFile(const char* midiName) {
    auto outData = new GwidiData();

    auto printType = [](smf::MidiEvent &evt) {
        spdlog::debug("event meta type: {}, as STR: ", evt.getMetaType());
        if(evt.isNoteOn()) {  spdlog::debug( "NoteOn,"); }
        if(evt.isInstrumentName()) {  spdlog::debug( "InstrumentName,"); }
        if(evt.isTempo()) {  spdlog::debug( "Tempo,"); }
        if(evt.isAftertouch()) {  spdlog::debug( "Aftertouch,"); }
        if(evt.isController()) {  spdlog::debug( "Controller,"); }
        if(evt.isCopyright()) {  spdlog::debug( "Copyright,"); }
        if(evt.isEmpty()) {  spdlog::debug( "Empty,"); }
        if(evt.isEndOfTrack()) {  spdlog::debug( "EndOfTrack,"); }
        if(evt.isKeySignature()) {  spdlog::debug( "KeySignature,"); }
        if(evt.isLyricText()) {  spdlog::debug( "LyricText,"); }
        if(evt.isMarkerText()) {  spdlog::debug( "MarkerText,"); }
        if(evt.isMeta()) {  spdlog::debug( "Meta,"); }
        if(evt.isMetaMessage()) {  spdlog::debug( "MetaMessage,"); }
        if(evt.isNote()) {  spdlog::debug( "Note,"); }
        if(evt.isNoteOff()) {  spdlog::debug( "NoteOff,"); }
        if(evt.isPatchChange()) {  spdlog::debug( "PatchChange,"); }
        if(evt.isPitchbend()) {  spdlog::debug( "Pitchbend,"); }
        if(evt.isPressure()) {  spdlog::debug( "Pressure,"); }
        if(evt.isProgramChange()) {  spdlog::debug( "ProgramChange,"); }
        if(evt.isSoft()) {  spdlog::debug( "Soft,"); }
        if(evt.isSoftOff()) {  spdlog::debug( "SoftOff,"); }
        if(evt.isSoftOn()) {  spdlog::debug( "SoftOn,"); }
        if(evt.isSustain()) {  spdlog::debug( "Sustain,"); }
        if(evt.isSustainOff()) {  spdlog::debug( "SustainOff,"); }
        if(evt.isSustainOn()) {  spdlog::debug( "SustainOn,"); }
        if(evt.isText()) {  spdlog::debug( "Text,"); }
        if(evt.isTempo()) {  spdlog::debug( "Tempo,"); }
        if(evt.isTimbre()) {  spdlog::debug( "Timbre,"); }
        if(evt.isTimeSignature()) {  spdlog::debug( "TimeSignature,"); }
        if(evt.isTrackName()) {  spdlog::debug( "TrackName,"); }
    };

    smf::MidiFile midiFile;
    midiFile.read(midiName);

    // Some prep for how we plan to use the data
    midiFile.doTimeAnalysis();
    auto linkedNotesCount = midiFile.linkNotePairs();
    spdlog::debug("Linked {} events!", linkedNotesCount);

    auto tc = midiFile.getTrackCount();
    spdlog::debug("Ticks Per Quarter Note: {}", midiFile.getTicksPerQuarterNote());
    spdlog::debug("# Tracks: {}", tc);

    for(auto i = 0; i < tc; i++) {
        spdlog::debug("Track: {}", i);
        auto &track = midiFile[i];
        auto ec = track.getEventCount();

        std::vector<Note> notes;
        std::string instrument;
        std::string track_name;

        for(auto j = 0; j < ec; j++) {
            spdlog::debug("Event: {}", j);
            auto &event = track[j];

            printType(event);

            if(event.isNoteOn()) {
                std::stringstream eventDetails;
                spdlog::debug("startSeconds: {}\nnoteOn: {}\nduration: {}\noctave: {}\nnumber: {}\nletter: {}", event.seconds, event.isNoteOn(), event.getDurationInSeconds(), event.getKeyOctave(), event.getKeyNumber(), event.getKeyLetter());
                notes.emplace_back(Note{
                    event.seconds,
                    event.getDurationInSeconds(),
                    event.getKeyOctave(),
                    event.getKeyLetter(),
                    "",
                    i
                });
            }
            else if(event.isTempo()) {
                outData->assignTempo(event.getTempoSeconds(), event.getTempoMicroseconds());
            }
            else if(event.isInstrumentName()) {
                instrument = event.getInstrument();
            }
            else if(event.isTrackName()) {
                track_name = event.getMetaContent();
            }
            else if(event.isTimbre()) {
                instrument = event.getInstrument();
            }
        }
        outData->addTrack(instrument, track_name, notes);
    }

    return outData;
}

void GwidiData::writeToFile(std::string filename) {
    std::ofstream out;
    out.open(filename, std::ios::out | std::ios::binary);
    size_t track_count = tracks.size();
    out.write(reinterpret_cast<const char *>(&track_count), sizeof(size_t));
    out.write(reinterpret_cast<const char *>(&tempo), sizeof(double));
    out.write(reinterpret_cast<const char *>(&tempoMicro), sizeof(double));
    for(auto& t : tracks) {
        size_t instrument_name_size = t.instrument_name.size();
        out.write(reinterpret_cast<const char *>(&instrument_name_size), sizeof(size_t));
        out.write(reinterpret_cast<const char *>(&t.instrument_name[0]), sizeof(char) * instrument_name_size);

        size_t track_name_size = t.track_name.size();
        out.write(reinterpret_cast<const char *>(&track_name_size), sizeof(size_t));
        out.write(reinterpret_cast<const char *>(&t.track_name[0]), sizeof(char) * track_name_size);

        size_t note_count = t.notes.size();
        out.write(reinterpret_cast<const char *>(&note_count), sizeof(size_t));

        for(auto &n : t.notes) {
            out.write(reinterpret_cast<const char *>(&(n.start_offset)), sizeof(double));
            out.write(reinterpret_cast<const char *>(&(n.duration)), sizeof(double));
            out.write(reinterpret_cast<const char *>(&(n.octave)), sizeof(int));

            size_t letter_size = n.letter.size();
            out.write(reinterpret_cast<const char *>(&letter_size), sizeof(size_t));
            out.write(reinterpret_cast<const char *>(&(n.letter[0])), sizeof(char) * letter_size);

            size_t instrument_size = n.instrument.size();
            out.write(reinterpret_cast<const char *>(&instrument_size), sizeof(size_t));
            out.write(reinterpret_cast<const char *>(&(n.instrument[0])), sizeof(char) * instrument_size);

            out.write(reinterpret_cast<const char *>(&(n.track)), sizeof(int));
        }
    }
    out.close();
}

GwidiData *GwidiData::readFromFile(std::string filename) {
    auto outData = new GwidiData();
    std::ifstream in;
    in.open(filename, std::ios::in | std::ios::binary);

    std::vector<unsigned char> inData(in.tellg());

    size_t track_count;
    in.read(reinterpret_cast<char *>(&track_count), sizeof(size_t));

    double tempo;
    in.read(reinterpret_cast<char *>(&tempo), sizeof(double));

    double tempoMicro;
    in.read(reinterpret_cast<char *>(&tempoMicro), sizeof(double));

    outData->tempo = tempo;
    outData->tempoMicro = tempoMicro;

    for(auto i = 0; i < track_count; i++) {
        size_t instrument_name_size;
        in.read(reinterpret_cast<char *>(&instrument_name_size), sizeof(size_t));
        std::string instrument_name;
        instrument_name.resize(instrument_name_size);
        in.read(reinterpret_cast<char *>(&instrument_name[0]), sizeof(char) * instrument_name_size);

        size_t track_name_size;
        in.read(reinterpret_cast<char *>(&track_name_size), sizeof(size_t));
        std::string track_name;
        track_name.resize(track_name_size);
        in.read(reinterpret_cast<char *>(&track_name[0]), sizeof(char) * track_name_size);

        size_t notes_size;
        in.read(reinterpret_cast<char *>(&notes_size), sizeof(size_t));

        std::vector<Note> notes;
        for(auto j = 0; j < notes_size; j++) {
            double start_offset;
            in.read(reinterpret_cast<char *>(&start_offset), sizeof(double));

            double duration;
            in.read(reinterpret_cast<char *>(&duration), sizeof(double));

            int octave;
            in.read(reinterpret_cast<char *>(&octave), sizeof(int));

            size_t letter_size;
            in.read(reinterpret_cast<char *>(&letter_size), sizeof(size_t));
            std::string letter;
            letter.resize(letter_size);
            in.read(reinterpret_cast<char *>(&letter[0]), sizeof(char) * letter_size);

            size_t instrument_size;
            in.read(reinterpret_cast<char *>(&instrument_size), sizeof(size_t));
            std::string instrument;
            instrument.resize(instrument_size);
            in.read(reinterpret_cast<char *>(&instrument[0]), sizeof(char) * instrument_size);

            int track;
            in.read(reinterpret_cast<char *>(&track), sizeof(int));

            notes.emplace_back(Note{
                    start_offset,
                    duration,
                    octave,
                    letter,
                    instrument,
                    track
            });
        }
        outData->addTrack(instrument_name, track_name, notes);
    }

    in.close();
    return outData;
}

void testWriteAndRead(GwidiData* in) {
    in->writeToFile("test_out.gwd");
    auto readData = GwidiData::readFromFile("test_out.gwd");

    FMT_ASSERT(in->getTempo() == readData->getTempo(), "tempo does not match");
    FMT_ASSERT(in->getTempoMicro() == readData->getTempoMicro(), "tempoMicro does not match");

    auto &tracks1 = in->getTracks();
    auto &tracks2 = readData->getTracks();
    FMT_ASSERT(tracks1.size() == tracks2.size(), "track count does not match");
}

int main() {
    spdlog::set_level(spdlog::level::info);

    auto data = readFile(R"(E:\Tools\repos\gwidi_midi_parser\assets\pollyanna.mid)");
    auto& tracks = data->getTracks();
    for(auto &t : tracks) {
        spdlog::debug("track: {}, tempo: [{}, {}], instrument: {}, # notes: {}", t.track_name, data->getTempo(), data->getTempoMicro(), t.instrument_name, t.notes.size());
    }

    testWriteAndRead(data);

    delete data;
    return 0;
}
