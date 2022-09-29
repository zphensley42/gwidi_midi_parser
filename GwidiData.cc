#include "GwidiData.h"
#include "spdlog/spdlog.h"


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

bool GwidiData::operator==(const GwidiData& rhs) const {
    bool equal = true;
    if(tracks.size() != rhs.tracks.size()) {
        return false;
    }

    if(tempo != rhs.tempo) {
        return false;
    }

    if(tempoMicro != rhs.tempoMicro) {
        return false;
    }

    for(auto i = 0; i < tracks.size(); i++) {
        auto &track = tracks.at(i);
        auto &rhsTrack = rhs.tracks.at(i);
        if(track.instrument_name != rhsTrack.instrument_name || track.track_name != rhsTrack.track_name || track.notes.size() != rhsTrack.notes.size()) {
            return false;
        }

        for(auto j = 0; j < track.notes.size(); j++) {
            auto &note = track.notes.at(j);
            auto &rhsNote = rhsTrack.notes.at(j);

            if(
                    note.start_offset != rhsNote.start_offset ||
                    note.duration != rhsNote.duration ||
                    note.octave != rhsNote.octave ||
                    note.letter != rhsNote.letter ||
                    note.instrument != rhsNote.instrument ||
                    note.track != rhsNote.track ||
                    note.instrument_note_number != rhsNote.instrument_note_number ||
                    note.key != rhsNote.key
            ) {
                return false;
            }
        }
    }
    return true;
}
