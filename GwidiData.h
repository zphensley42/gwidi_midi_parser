#ifndef GWIDI_MIDI_PARSER_GWIDIDATA_H
#define GWIDI_MIDI_PARSER_GWIDIDATA_H

#include <vector>

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

class GwidiData {
public:
    GwidiData() = default;
    explicit GwidiData(const std::vector<Track>& tracks);
    explicit GwidiData(std::vector<Track>&& tracks);

    void assignNotes(int track, const std::vector<Note>& notes);
    void addTrack(std::string instrument, std::string track_name, const std::vector<Note>& notes);
    void addNote(int track, Note& note);
    void assignTempo(double tempo, double tempoMicro);

    inline std::vector<Track>& getTracks() {
        return tracks;
    }

    inline double getTempo() {
        return tempo;
    }

    inline double getTempoMicro() {
        return tempoMicro;
    }

    void writeToFile(std::string filename);
    static GwidiData* readFromFile(std::string filename);

private:
    std::vector<Track> tracks;
    double tempo;
    double tempoMicro;
};

#endif //GWIDI_MIDI_PARSER_GWIDIDATA_H
