#include "GwidiMidiData.h"
#include "spdlog/spdlog.h"
#include <fstream>

namespace gwidi::data::midi {

GwidiMidiData::GwidiMidiData(const std::vector<Track> &tracks) {
    this->tracks.clear();
    for (auto &t: tracks) {
        this->tracks.emplace_back(Track(t));
    }
}

GwidiMidiData::GwidiMidiData(std::vector<Track> &&tracks) {
    this->tracks = std::move(tracks);
}

void GwidiMidiData::assignNotes(int track, const std::vector<Note> &in_notes) {
    this->tracks[track] = Track();
    for (auto &note: in_notes) {
        this->tracks[track].notes.emplace_back(Note(note));
    }
}

void GwidiMidiData::addNote(int track, Note &note) {
    if (track >= 0 && track < this->tracks.size()) {
        this->tracks[track].notes.emplace_back(Note(note));
    }
}

void GwidiMidiData::assignTempo(double t, double tm) {
    this->tempo = t;
    this->tempoMicro = tm;
}

void GwidiMidiData::addTrack(std::string instrument, std::string track_name, const std::vector<Note> &notes,
                             double trackDurationInSeconds) {
    this->tracks.emplace_back(Track{
            std::vector<Note>(),
            std::move(instrument),
            std::move(track_name),
            trackDurationInSeconds
    });
    for (auto &n: notes) {
        this->tracks.back().notes.emplace_back(Note(n));
    }
}

void GwidiMidiData::fillTickMap() {
    tickMap.clear();

    // Assume 1 track, due to chosen_track options in the midi parsing
    auto &t = tracks.front();
    spdlog::debug("fillTickMap  filling notes for track with #{} notes", t.notes.size());
    for (auto &n: t.notes) {
        if (tickMap.find(n.start_offset) == tickMap.end()) {
            spdlog::debug("fillTickMap  start_offset {} not found, adding", n.start_offset);
            tickMap[n.start_offset] = std::vector<Note>();
        }
        tickMap[n.start_offset].emplace_back(Note(n));
    }
}

bool GwidiMidiData::operator==(const GwidiMidiData &rhs) const {
    if (tracks.size() != rhs.tracks.size()) {
        return false;
    }

    if (tempo != rhs.tempo) {
        return false;
    }

    if (tempoMicro != rhs.tempoMicro) {
        return false;
    }

    for (auto i = 0; i < tracks.size(); i++) {
        auto &track = tracks.at(i);
        auto &rhsTrack = rhs.tracks.at(i);
        if (track.durationInSeconds != rhsTrack.durationInSeconds) {
            return false;
        }

        if (track.instrument_name != rhsTrack.instrument_name || track.track_name != rhsTrack.track_name ||
            track.notes.size() != rhsTrack.notes.size()) {
            return false;
        }

        for (auto j = 0; j < track.notes.size(); j++) {
            auto &note = track.notes.at(j);
            auto &rhsNote = rhsTrack.notes.at(j);

            if (
                    note.start_offset != rhsNote.start_offset ||
                    note.duration != rhsNote.duration ||
                    note.octave != rhsNote.octave ||
                    note.letter != rhsNote.letter ||
                    note.instrument != rhsNote.instrument ||
                    note.track != rhsNote.track ||
                    note.key != rhsNote.key
                    ) {
                return false;
            }
        }
    }
    return true;
}

double GwidiMidiData::longestTrackDuration() {
    double longest{0};
    for (auto &t: tracks) {
        if (t.durationInSeconds > longest) {
            longest = t.durationInSeconds;
        }
    }
    return longest;
}


void GwidiMidiData::writeToFile(const std::string &filename) {
    std::ofstream out;
    out.open(filename, std::ios::out | std::ios::binary);

    size_t track_count = tracks.size();
    out.write(reinterpret_cast<const char *>(&track_count), sizeof(size_t));
    out.write(reinterpret_cast<const char *>(&tempo), sizeof(double));
    out.write(reinterpret_cast<const char *>(&tempoMicro), sizeof(double));
    for (auto &t: tracks) {
        out.write(reinterpret_cast<const char *>(&(t.durationInSeconds)), sizeof(double));

        size_t instrument_name_size = t.instrument_name.size();
        out.write(reinterpret_cast<const char *>(&instrument_name_size), sizeof(size_t));
        out.write(reinterpret_cast<const char *>(&t.instrument_name[0]), sizeof(char) * instrument_name_size);

        size_t track_name_size = t.track_name.size();
        out.write(reinterpret_cast<const char *>(&track_name_size), sizeof(size_t));
        out.write(reinterpret_cast<const char *>(&t.track_name[0]), sizeof(char) * track_name_size);

        size_t note_count = t.notes.size();
        out.write(reinterpret_cast<const char *>(&note_count), sizeof(size_t));

        for (auto &n: t.notes) {
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

            size_t key_size = n.key.size();
            out.write(reinterpret_cast<const char *>(&key_size), sizeof(size_t));
            out.write(reinterpret_cast<const char *>(&(n.key[0])), sizeof(char) * key_size);
        }
    }
    out.close();
}

GwidiMidiData *GwidiMidiData::readFromFile(const std::string &filename) {
    auto outData = new GwidiMidiData();
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

    for (auto i = 0; i < track_count; i++) {
        double trackDurationInSeconds;
        in.read(reinterpret_cast<char *>(&trackDurationInSeconds), sizeof(double));

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
        for (auto j = 0; j < notes_size; j++) {
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

            size_t key_size;
            in.read(reinterpret_cast<char *>(&key_size), sizeof(size_t));
            std::string key;
            key.resize(key_size);
            in.read(reinterpret_cast<char *>(&key[0]), sizeof(char) * key_size);

            notes.emplace_back(Note{
                    start_offset,
                    duration,
                    octave,
                    letter,
                    instrument,
                    track,
                    key
            });
        }
        outData->addTrack(instrument_name, track_name, notes, trackDurationInSeconds);
    }

    in.close();
    outData->fillTickMap();
    return outData;
}

}
