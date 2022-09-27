#ifndef GWIDI_MIDI_PARSER_GWIDIDATA_H
#define GWIDI_MIDI_PARSER_GWIDIDATA_H

#include <vector>
#include <string>
#include <unordered_map>
#include <map>

struct GwidiInstrumentNote {
    int octave{0};
    std::string letter{""};
    std::string key{""};
};

struct Note {
    double start_offset {0.0};
    double duration {0.0};
    int octave {0};
    std::string letter {""};
    std::string instrument {""};
    int track {0};
};

struct Track {
    std::vector<Note> notes;
    std::string instrument_name;
    std::string track_name;
};

struct InstrumentOptions {
    enum Instrument {
        UNKNOWN = 0,
        HARP = 1,
        FLUTE = 2,
        BELL = 3
    };

    static const char* nameForInstrument(Instrument in);
    static Instrument enumForInstrument(const char* in);

    using GwidiInstrumentOctaves = std::vector<GwidiInstrumentNote>;
    static GwidiInstrumentOctaves empty_GwidiInstrumentOctaves;

    static GwidiInstrumentOctaves& notesForInstrument(Instrument instrument);
    static std::vector<int> octaveListForNotesList(GwidiInstrumentOctaves& notesList);

    static int startingOctaveForInstrument(Instrument instrument);
};

struct MidiParseOptions {
    InstrumentOptions::Instrument instrument;
    std::map<int, int> midiOctaveChoices;
};



class GwidiData {
public:
    using TickMapType = std::map<int, std::map<double, std::vector<Note>>>;
    using OctaveMapping = std::unordered_map<int, int>;

    GwidiData() = default;
    explicit GwidiData(const std::vector<Track>& tracks);
    explicit GwidiData(std::vector<Track>&& tracks);

    void assignNotes(int track, const std::vector<Note>& notes);
    void addTrack(std::string instrument, std::string track_name, const std::vector<Note>& notes);
    void addNote(int track, Note& note);
    void assignTempo(double tempo, double tempoMicro);

    void fillTickMap();

    inline std::vector<Track>& getTracks() {
        return tracks;
    }

    inline double getTempo() {
        return tempo;
    }

    inline double getTempoMicro() {
        return tempoMicro;
    }

    inline TickMapType& getTickMap() {
        return tickMap;
    }

//    inline void assignOctaveMapping(OctaveMapping &mapping) {
//        octaveMapping = mapping;
//    }
//
//    inline OctaveMapping& getOctaveMapping() {
//        return octaveMapping;
//    }

    void writeToFile(const std::string& filename);
    static GwidiData* readFromFile(const std::string &filename);

private:
    std::vector<Track> tracks;
    double tempo{0.0};
    double tempoMicro{0.0};

    // Map of track -> [start_time, Note[]]
    TickMapType tickMap;

//    OctaveMapping octaveMapping;
};

class GwidiMidiParser {
public:
    static GwidiMidiParser& getInstance() {
        static GwidiMidiParser instance;
        return instance;
    }

    GwidiData* readFile(const char* midiName, MidiParseOptions options);
};

#endif //GWIDI_MIDI_PARSER_GWIDIDATA_H
