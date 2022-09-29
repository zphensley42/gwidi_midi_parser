#include <iostream>
#include <sstream>
#include "spdlog/spdlog.h"
#include "MidiFile.h"
#include "gwidi_midi_parser.h"
#include "GwidiData.h"

// TODO: When building gwidi data from midi, we need to determine which octaves to use as our set of octaves that we support
// TODO: against instruments
// TODO: Part of this is determining: 1 which instrument we are using to thus determine how many octaves we can support and
// TODO: 2 determining which midi octaves map to which gwidi octaves
GwidiData* GwidiMidiParser::readFile(const char* midiName, gwidi::options::MidiParseOptions options) {
    auto &instrumentOptions = gwidi::options::InstrumentOptions::getInstance();   // initialize our instrument mapping

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

                // When adding a note, determine the 'Note' class variables via our instrumentMapping options
                // If a note doesn't exist in our mapping, it shouldn't be used
                auto instrName = gwidi::options::InstrumentOptions::nameForInstrument(options.instrument);
                auto instrumentNoteAttribs = instrumentOptions.instrumentNoteAttributesForMidi(instrName, event.getKeyOctave(), event.getKeyLetter());
                // Use the event if we have a valid mapping for it
                if(!instrumentNoteAttribs.letters.empty()) {
                    notes.emplace_back(Note{
                            event.seconds,
                            event.getDurationInSeconds(),
                            instrumentNoteAttribs.instrumentOctave,
                            event.getKeyLetter(),
                            "",
                            i,
                            instrumentNoteAttribs.instrumentNoteNumber,
                            instrumentNoteAttribs.key
                    });
                }
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

    outData->fillTickMap();
    return outData;
}

void GwidiData::writeToFile(const std::string& filename) {
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

            out.write(reinterpret_cast<const char *>(&(n.instrument_note_number)), sizeof(int));

            size_t key_size = n.key.size();
            out.write(reinterpret_cast<const char *>(&key_size), sizeof(size_t));
            out.write(reinterpret_cast<const char *>(&(n.key[0])), sizeof(char) * key_size);
        }
    }
    out.close();
}

GwidiData *GwidiData::readFromFile(const std::string &filename) {
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
            std::string letter; letter.resize(letter_size);
            in.read(reinterpret_cast<char *>(&letter[0]), sizeof(char) * letter_size);

            size_t instrument_size;
            in.read(reinterpret_cast<char *>(&instrument_size), sizeof(size_t));
            std::string instrument; instrument.resize(instrument_size);
            in.read(reinterpret_cast<char *>(&instrument[0]), sizeof(char) * instrument_size);

            int track;
            in.read(reinterpret_cast<char *>(&track), sizeof(int));

            int instrument_note_number;
            in.read(reinterpret_cast<char *>(&instrument_note_number), sizeof(int));

            size_t key_size;
            in.read(reinterpret_cast<char *>(&key_size), sizeof(size_t));
            std::string key; key.resize(key_size);
            in.read(reinterpret_cast<char *>(&key[0]), sizeof(char) * key_size);

            notes.emplace_back(Note{
                    start_offset,
                    duration,
                    octave,
                    letter,
                    instrument,
                    track,
                    instrument_note_number,
                    key
            });
        }
        outData->addTrack(instrument_name, track_name, notes);
    }

    in.close();
    outData->fillTickMap();
    return outData;
}

