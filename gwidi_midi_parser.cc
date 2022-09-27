#include <iostream>
#include <sstream>
#include "spdlog/spdlog.h"
#include "MidiFile.h"
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

void GwidiData::fillTickMap() {
    tickMap.clear();
    for(auto i = 0; i < tracks.size(); i++) {
        auto &t = tracks.at(i);
        tickMap[i] = std::map<double, std::vector<Note>>();
        auto &map = tickMap[i];
        spdlog::debug("fillTickMap  filling notes for track with #{} notes", t.notes.size());
        for(auto &n : t.notes) {
            if(map.find(n.start_offset) == map.end()) {
                spdlog::debug("fillTickMap  start_offset {} not found, adding", n.start_offset);
                map[n.start_offset] = std::vector<Note>();
            }
            map[n.start_offset].emplace_back(Note(n));
        }
    }
}

// TODO: When building gwidi data from midi, we need to determine which octaves to use as our set of octaves that we support
// TODO: against instruments
// TODO: Part of this is determining: 1 which instrument we are using to thus determine how many octaves we can support and
// TODO: 2 determining which midi octaves map to which gwidi octaves
GwidiData* GwidiMidiParser::readFile(const char* midiName, MidiParseOptions options) {
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

                // determine if we care about this note and what actual octave it should be in our instrument mapping
                int midiNoteOctave = event.getKeyOctave();
                auto it = std::find_if(options.midiOctaveChoices.begin(), options.midiOctaveChoices.end(), [midiNoteOctave](std::pair<int, int> entry){
                    return entry.second == midiNoteOctave;
                });
                if(it != options.midiOctaveChoices.end()) {
                    // Note should be used (but use the octave it is mapped to instead)
                    midiNoteOctave = it->first;
                }

                notes.emplace_back(Note{
                    event.seconds,
                    event.getDurationInSeconds(),
                    midiNoteOctave,
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

    // After filling our data, filter by the octave choices
    // Move to a member method on data?
//    auto &tracks = outData->getTracks();
//    for(auto &track : tracks) {
//        track.notes.erase(
//                std::remove_if(track.notes.begin(), track.notes.end(), [&options](const Note& n){
//                    // true to remove, false to keep
//                    auto it = std::find(options.midiOctaveChoices.begin(), options.midiOctaveChoices.end(), n.octave);
//                    return it == options.midiOctaveChoices.end();
//                }),
//                track.notes.end()
//        );
//    }

    // Assign octave numbers from our chosen instrument to the octaves we have available after filtering
    // i.e. harp has 0,1,2,3 -> point to the choices given in midi options
    // TODO: Error cases where midi options don't give the right amounts, maybe just continue picking the next octaves until we run out?
//    auto &instrumentNotes = InstrumentOptions::notesForInstrument(options.instrument);
//    auto notesListOctaves = InstrumentOptions::octaveListForNotesList(instrumentNotes);
//    GwidiData::OctaveMapping mapping;
//    for(auto i = 0; i < notesListOctaves.size(); i++) {
//        if(i < options.midiOctaveChoices.size()) {
//            mapping[notesListOctaves[i]] = options.midiOctaveChoices[i];
//        }
//        else {
//            mapping[notesListOctaves[i]] = -1;
//        }
//    }

//    outData->assignOctaveMapping(mapping);
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
    outData->fillTickMap();
    return outData;
}



const char* InstrumentOptions::nameForInstrument(InstrumentOptions::Instrument in) {
    static std::unordered_map<Instrument, const char*> instrument_name {
            {Instrument::HARP, "harp"},
            {Instrument::FLUTE, "flute"},
            {Instrument::BELL, "bell"},
    };
    auto it = instrument_name.find(in);
    if(it == instrument_name.end()) {
        return "";
    }
    return it->second;
}

InstrumentOptions::Instrument InstrumentOptions::enumForInstrument(const char* in) {
    static std::unordered_map<const char*, Instrument> instrument_enum {
            {"harp", Instrument::HARP},
            {"flute", Instrument::FLUTE},
            {"bell", Instrument::BELL},
    };
    auto it = instrument_enum.find(in);
    if(it == instrument_enum.end()) {
        return Instrument::UNKNOWN;
    }
    return it->second;
}


InstrumentOptions::GwidiInstrumentOctaves InstrumentOptions::empty_GwidiInstrumentOctaves{};
InstrumentOptions::GwidiInstrumentOctaves& InstrumentOptions::notesForInstrument(Instrument instrument) {
    static std::unordered_map<Instrument, GwidiInstrumentOctaves> notes_for_instrument {
            {Instrument::HARP, {
                                       GwidiInstrumentNote{0, "C", "1"},
                                       GwidiInstrumentNote{0, "C#", "1"},
                                       GwidiInstrumentNote{0, "D", "2"},
                                       GwidiInstrumentNote{0, "D#", "2"},
                                       GwidiInstrumentNote{0, "E", "3"},
                                       GwidiInstrumentNote{0, "E#", "3"},
                                       GwidiInstrumentNote{0, "F", "4"},
                                       GwidiInstrumentNote{0, "F#", "4"},
                                       GwidiInstrumentNote{0, "G", "5"},
                                       GwidiInstrumentNote{0, "G#", "5"},
                                       GwidiInstrumentNote{0, "A", "6"},
                                       GwidiInstrumentNote{0, "A#", "6"},
                                       GwidiInstrumentNote{0, "B", "7"},
                                       GwidiInstrumentNote{0, "B#", "7"},

                                       GwidiInstrumentNote{1, "C", "1"},
                                       GwidiInstrumentNote{1, "C#", "1"},
                                       GwidiInstrumentNote{1, "D", "2"},
                                       GwidiInstrumentNote{1, "D#", "2"},
                                       GwidiInstrumentNote{1, "E", "3"},
                                       GwidiInstrumentNote{1, "E#", "3"},
                                       GwidiInstrumentNote{1, "F", "4"},
                                       GwidiInstrumentNote{1, "F#", "4"},
                                       GwidiInstrumentNote{1, "G", "5"},
                                       GwidiInstrumentNote{1, "G#", "5"},
                                       GwidiInstrumentNote{1, "A", "6"},
                                       GwidiInstrumentNote{1, "A#", "6"},
                                       GwidiInstrumentNote{1, "B", "7"},
                                       GwidiInstrumentNote{1, "B#", "7"},

                                       GwidiInstrumentNote{2, "C", "1"},
                                       GwidiInstrumentNote{2, "C#", "1"},
                                       GwidiInstrumentNote{2, "D", "2"},
                                       GwidiInstrumentNote{2, "D#", "2"},
                                       GwidiInstrumentNote{2, "E", "3"},
                                       GwidiInstrumentNote{2, "E#", "3"},
                                       GwidiInstrumentNote{2, "F", "4"},
                                       GwidiInstrumentNote{2, "F#", "4"},
                                       GwidiInstrumentNote{2, "G", "5"},
                                       GwidiInstrumentNote{2, "G#", "5"},
                                       GwidiInstrumentNote{2, "A", "6"},
                                       GwidiInstrumentNote{2, "A#", "6"},
                                       GwidiInstrumentNote{2, "B", "7"},
                                       GwidiInstrumentNote{2, "B#", "7"},

                                       GwidiInstrumentNote{3, "C", "8"}, // For some reason, GW2 sets up a final octave but only 1 note
                                       GwidiInstrumentNote{3, "C#", "8"}, // For some reason, GW2 sets up a final octave but only 1 note
                               }},
            {Instrument::FLUTE, {
                                       GwidiInstrumentNote{0, "C", "1"},
                                       GwidiInstrumentNote{0, "D", "2"},
                                       GwidiInstrumentNote{0, "E", "3"},
                                       GwidiInstrumentNote{0, "F", "4"},
                                       GwidiInstrumentNote{0, "G", "5"},
                                       GwidiInstrumentNote{0, "A", "6"},
                                       GwidiInstrumentNote{0, "B", "7"},

                                       GwidiInstrumentNote{1, "C", "1"},
                                       GwidiInstrumentNote{1, "D", "2"},
                                       GwidiInstrumentNote{1, "E", "3"},
                                       GwidiInstrumentNote{1, "F", "4"},
                                       GwidiInstrumentNote{1, "G", "5"},
                                       GwidiInstrumentNote{1, "A", "6"},
                                       GwidiInstrumentNote{1, "B", "7"},

                                       GwidiInstrumentNote{2, "C", "8"}, // For some reason, GW2 sets up a final octave but only 1 note
                               }},
            {Instrument::BELL, {
                                       GwidiInstrumentNote{0, "C", "1"},
                                       GwidiInstrumentNote{0, "D", "2"},
                                       GwidiInstrumentNote{0, "E", "3"},
                                       GwidiInstrumentNote{0, "F", "4"},
                                       GwidiInstrumentNote{0, "G", "5"},
                                       GwidiInstrumentNote{0, "A", "6"},
                                       GwidiInstrumentNote{0, "B", "7"},

                                       GwidiInstrumentNote{1, "C", "1"},
                                       GwidiInstrumentNote{1, "D", "2"},
                                       GwidiInstrumentNote{1, "E", "3"},
                                       GwidiInstrumentNote{1, "F", "4"},
                                       GwidiInstrumentNote{1, "G", "5"},
                                       GwidiInstrumentNote{1, "A", "6"},
                                       GwidiInstrumentNote{1, "B", "7"},

                                       GwidiInstrumentNote{2, "C", "1"},
                                       GwidiInstrumentNote{2, "D", "2"},
                                       GwidiInstrumentNote{2, "E", "3"},
                                       GwidiInstrumentNote{2, "F", "4"},
                                       GwidiInstrumentNote{2, "G", "5"},
                                       GwidiInstrumentNote{2, "A", "6"},
                                       GwidiInstrumentNote{2, "B", "7"},

                                       GwidiInstrumentNote{3, "C", "8"}, // For some reason, GW2 sets up a final octave but only 1 note
                               }}
    };
    auto it = notes_for_instrument.find(instrument);
    if(it == notes_for_instrument.end()) {
        return empty_GwidiInstrumentOctaves;
    }
    return it->second;
}

std::vector<int> InstrumentOptions::octaveListForNotesList(GwidiInstrumentOctaves& notesList) {
    std::map<int, int> c;
    for(auto &entry : notesList) {
        if(c.find(entry.octave) == c.end()) {
            c[entry.octave] = 0;
        }
        c[entry.octave]++;
    }
    std::vector<int> v;
    for(auto &entry : c) {
        v.push_back(entry.first);
    }
    return v;
}

int InstrumentOptions::startingOctaveForInstrument(InstrumentOptions::Instrument instrument) {
    switch(instrument) {
        case Instrument::HARP: {
            return 1;
        }
        case Instrument::FLUTE: {
            return 0;
        }
        case Instrument::BELL: {
            return 1;
        }
        default: return 0;
    }
}
