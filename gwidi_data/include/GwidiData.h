#ifndef GWIDI_MIDI_PARSER_GWIDIDATA_H
#define GWIDI_MIDI_PARSER_GWIDIDATA_H

#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include "GwidiOptions2.h"

namespace gwidi::data::midi {

struct Note {
    double start_offset{0.0};
    double duration{0.0};
    int octave{0};
    std::string letter{};
    std::string instrument{};
    int track{0};
    std::string key{};
};

struct Track {
    std::vector<Note> notes;
    std::string instrument_name;
    std::string track_name;
    double durationInSeconds;
};


// TODO: Rename GwidiData to GwidiMidiData
class GwidiData {
public:
    using TickMapType = std::map<double, std::vector<Note>>;

    GwidiData() = default;

    explicit GwidiData(const std::vector<Track> &tracks);
    explicit GwidiData(std::vector<Track> &&tracks);
    void assignNotes(int track, const std::vector<Note> &notes);
    void addTrack(std::string instrument, std::string track_name, const std::vector<Note> &notes, double trackDurationInSeconds);
    void addNote(int track, Note &note);
    void assignTempo(double tempo, double tempoMicro);
    void fillTickMap();

    inline std::vector<Track> &getTracks() {
        return tracks;
    }

    inline double getTempo() const {
        return tempo;
    }

    inline double getTempoMicro() const {
        return tempoMicro;
    }

    inline TickMapType &getTickMap() {
        return tickMap;
    }

    void writeToFile(const std::string &filename);
    static GwidiData *readFromFile(const std::string &filename);
    bool operator==(const GwidiData &rhs) const;
    double longestTrackDuration();

private:
    std::vector<Track> tracks;
    double tempo{0.0};
    double tempoMicro{0.0};

    // Map of track -> [start_time, Note[]]
    TickMapType tickMap;
};

}
#endif //GWIDI_MIDI_PARSER_GWIDIDATA_H
