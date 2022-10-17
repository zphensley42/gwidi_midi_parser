#include <iostream>
#include <sstream>
#include "spdlog/spdlog.h"
#include "gwidi_midi_parser.h"
#include "GwidiMidiData.h"
#include "MidiFile.h"
#include "GwidiOptions2.h"

namespace gwidi::midi {

const char* nameForInstrument(Instrument instr) {
    static std::map<Instrument, const char*> m {
            {Instrument::UNKNOWN, "unknown"},
            {Instrument::HARP, "harp"},
            {Instrument::FLUTE, "flute"},
            {Instrument::BELL, "bell"},
    };
    return m[instr];
}
Instrument instrumentForName(const char* instr) {
    static std::map<const char*, Instrument> m {
            {"unknown", Instrument::UNKNOWN},
            {"harp", Instrument::HARP},
            {"flute", Instrument::FLUTE},
            {"bell", Instrument::BELL},
    };
    return m[instr];
}


gwidi::data::midi::GwidiMidiData* GwidiMidiParser::readFile(const char* midiName, const MidiParseOptions& options) {
    auto &instrumentOptions = gwidi::options2::GwidiOptions2::getInstance();   // initialize our instrument mapping

    auto outData = new gwidi::data::midi::GwidiMidiData();

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

        std::vector<gwidi::data::midi::Note> notes;
        std::string instrument;
        std::string track_name;
        double trackDurationInSeconds{0};

        for(auto j = 0; j < ec; j++) {
            spdlog::debug("Event: {}", j);
            auto &event = track[j];

            printType(event);

            if(event.isNoteOn()) {
                std::stringstream eventDetails;
                spdlog::debug("startSeconds: {}\nnoteOn: {}\nduration: {}\noctave: {}\nnumber: {}\nletter: {}", event.seconds, event.isNoteOn(), event.getDurationInSeconds(), event.getKeyOctave(), event.getKeyNumber(), event.getKeyLetter());

                // When adding a note, determine the 'Note' class variables via our instrumentMapping options
                // If a note doesn't exist in our mapping, it shouldn't be used
                auto instrName = nameForInstrument(options.instrument);
                auto optionsNote = instrumentOptions.optionsNoteFromMidiNote(instrName, event.getKeyOctave(), event.getKeyLetter());
                // Use the event if we have a valid mapping for it
                if(!optionsNote.letters.empty()) {
                    notes.emplace_back(gwidi::data::midi::Note{
                            event.seconds,
                            event.getDurationInSeconds(),
                            optionsNote.instrument_octave,
                            event.getKeyLetter(),
                            "",
                            i,
                            optionsNote.key
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
            else if(event.isEndOfTrack()) {
                spdlog::debug("EndOfTrack seconds: {}", event.seconds);
                trackDurationInSeconds = event.seconds;
            }
        }

        // This is still required down here because we do other things with track 1 (like assign tempo)
        if(i != options.chosen_track) {
            spdlog::debug("Skipping track: {}, chosen_track: {}", i, options.chosen_track);
            continue;
        }
        outData->addTrack(instrument, track_name, notes, trackDurationInSeconds);
    }

    outData->fillTickMap();
    return outData;
}

// Used to let users choose which track to pick when midi importing (passed in MidiParseOptions)
GwidiMidiParser::TrackMeta GwidiMidiParser::getTrackMetaMap(const char *midiName) {
    TrackMeta ret;

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
        ret[i] = MidiParseTrackStats{};
        ret[i].num_notes = 0;

        spdlog::debug("Track: {}", i);
        auto &track = midiFile[i];
        auto ec = track.getEventCount();

        for(auto j = 0; j < ec; j++) {
            spdlog::debug("Event: {}", j);
            auto &event = track[j];

            if(event.isNoteOn()) {
                ret[i].num_notes++;
            }
            else if(event.isTempo()) {
                ret[i].tempo = event.getTempoSeconds();
            }
            else if(event.isInstrumentName()) {
                if(ret[i].instrument.empty()) {
                    ret[i].instrument = event.getInstrument();
                }
            }
            else if(event.isTrackName()) {
                ret[i].name = event.getMetaContent();
            }
            else if(event.isTimbre()) {
                if(ret[i].instrument.empty()) {
                    ret[i].instrument = event.getInstrument();
                }
            }
            else if(event.isEndOfTrack()) {
                ret[i].duration = event.seconds;
            }
        }
    }

    return ret;
}

}

