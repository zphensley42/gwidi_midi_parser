#include <iostream>
#include <sstream>
#include <map>
#include "spdlog/spdlog.h"
#include "GwidiTickHandler.h"

namespace gwidi::tick {

void GwidiTickHandler::assignData(gwidi::data::midi::GwidiData *data) {
    m_tickMap.clear();
    m_midi_data = data;

    auto &tickMap = data->getTickMap();
    for(auto &entry : tickMap) {
        auto &time = entry.first;
        auto &notes = entry.second;
        m_tickMap[time] = std::vector<Note>();
        for(auto &note : notes) {
            m_tickMap[time].emplace_back(fromNote(note));
        }
    }
}

void GwidiTickHandler::assignData(gwidi::data::gui::GwidiGuiData *data) {
    m_tickMap.clear();
    m_gui_data = data;

    auto &tickMap = data->getTickMap();
    for(auto &entry : tickMap) {
        auto &time = entry.first;
        auto &notes = entry.second;
        m_tickMap[time] = std::vector<Note>();
        for(auto &note : notes) {
            m_tickMap[time].emplace_back(fromNote(note));
        }
    }
}

Note GwidiTickHandler::fromNote(gwidi::data::midi::Note &note) {
    return Note{
        note.start_offset,
        note.duration,
        note.octave,
        note.key,
        false
    };
}
Note GwidiTickHandler::fromNote(gwidi::data::gui::Note &note) {
    return Note {
        note.timeIndexToTickOffset(),
        1.0,    // no sense of 'duration' for gui currently, everything is a product of 1/<num notes per measure>
        note.octave,
        note.key,
        false   // tick 'activated' is whether it is played or not (instead of whether it is drawn as enabled or not from gui)
    };
}

double GwidiTickHandler::currentTickMapFloorKey() {
    double bound_key = -1.0;
    auto bound_itr = m_tickMap.lower_bound(cur_time);

    // We have 3 cases:
    // 1 - cur_time is < all keys, lower_bound returns begin, return first()
    // 2 - cur_time is > some keys, < some keys, lower_bound returns first >=, return --itr
    // 3 - cur_time is > all keys, lower_bound returns end, return back()
    if(bound_itr == m_tickMap.begin()) {
        bound_key = bound_itr->first;
    }
    else if(bound_itr == m_tickMap.end()) {
        bound_key = (--m_tickMap.end())->first;
    }
    else {
        bound_key = (--bound_itr)->first;
    }
    spdlog::debug("currentTickMapFloorKey cur_time: {}, bound_key: {}", cur_time, bound_key);

    return bound_key;
}

GwidiAction* GwidiTickHandler::processMidiTick() {
    // Search for the closest map of notes per our delta
    // The map cannot come before our delta, so this is a floor of our delta
    // Mark the notes as played such that we don't trigger them more than once
    // TODO: Need to handle start/stop for flute-type instruments. i.e. start and hold for duration, then stop at the end of the duration
    // TODO: For this, probably need actions to not just be list of notes
    // TODO: Instead, make actions be related to the options for the instruments? (i.e. play via number and stop or so on)

    auto action = new GwidiAction();
    if (cur_time >= m_midi_data->longestTrackDuration()) {
        action->end_reached = true;
    }

    auto floorKey = currentTickMapFloorKey();
    spdlog::debug("processTick, cur_time: {}", cur_time);
    spdlog::debug("processTick, -----BEGIN floorKeys------");
    if(floorKey != -1.0) {
        spdlog::debug("key: {}", floorKey);
        auto &notes = m_tickMap[floorKey];
        // TODO: More efficient here would be to remove from the map after we complete the action
        // TODO: Need a feedback mechanism? Maybe not, maybe we just assume the return of the action is enough
        for (auto &n: notes) {
            if (!n.activated) {
                n.activated = true;
                action->notes.emplace_back(&n);
            }
        }
    }
    spdlog::debug("processTick, -----END floorKeys------");

    return action;
}
GwidiAction* GwidiTickHandler::processGuiTick() {
    // Search for the closest map of notes per our delta
    // The map cannot come before our delta, so this is a floor of our delta
    // Mark the notes as played such that we don't trigger them more than once
    // TODO: Need to handle start/stop for flute-type instruments. i.e. start and hold for duration, then stop at the end of the duration
    // TODO: For this, probably need actions to not just be list of notes
    // TODO: Instead, make actions be related to the options for the instruments? (i.e. play via number and stop or so on)

    auto action = new GwidiAction();
    if (cur_time >= m_gui_data->trackDuration()) {
        action->end_reached = true;
    }

    auto floorKey = currentTickMapFloorKey();
    spdlog::debug("processTick, cur_time: {}", cur_time);
    spdlog::debug("processTick, -----BEGIN floorKeys------");
    if(floorKey != -1.0) {
        spdlog::debug("key: {}", floorKey);
        auto &notes = m_tickMap[floorKey];
        // TODO: More efficient here would be to remove from the map after we complete the action
        // TODO: Need a feedback mechanism? Maybe not, maybe we just assume the return of the action is enough
        for (auto &n: notes) {
            if (!n.activated) {
                n.activated = true;
                action->notes.emplace_back(&n);
            }
        }
    }
    spdlog::debug("processTick, -----END floorKeys------");

    return action;
}

void GwidiTickHandler::filterByOctaveBehavior(GwidiAction* action) const {
    if(!action) {
        return;
    }
    // Some determinations can be made for how to treat cases where multiple octaves are present:
    // Option 1: Always choose the lowest octave (kill other notes)
    // Option 2: Always choose the higest octave (kill other notes)
    // Option 3: Always choose the octave with the most notes in the action (kill other notes)
    switch (this->options.octaveBehavior) {
        case GwidiTickOptions::ActionOctaveBehavior::LOWEST: {
            // First, find the lowest octave
            int octave = -1;
            for (auto &n: action->notes) {
                if (octave == -1 || n->octave < octave) {
                    octave = n->octave;
                }
            }

            // Then, disregard notes that are not of this chosen octave
            action->chosen_octave = octave;
            break;
        }
        case GwidiTickOptions::ActionOctaveBehavior::HIGHEST: {
            // First, find the highest octave
            int octave = -1;
            for (auto &n: action->notes) {
                if (octave == -1 || n->octave > octave) {
                    octave = n->octave;
                }
            }
            action->chosen_octave = octave;
            break;
        }
        case GwidiTickOptions::ActionOctaveBehavior::MOST: {
            // Build list of octave counts
            std::unordered_map<int, int> octave_counts;
            for (auto &n: action->notes) {
                if (octave_counts.find(n->octave) == octave_counts.end()) {
                    octave_counts[n->octave] = 0;
                }
                octave_counts[n->octave]++;
            }
            // Then pick the most
            int octave_count = -1;
            int octave = -1;
            for (auto &entry: octave_counts) {
                if (octave == -1 || entry.second > octave_count) {
                    octave = entry.first;
                    octave_count = entry.second;
                }
            }
            action->chosen_octave = octave;
            break;
        }
    }
}

GwidiAction *GwidiTickHandler::processTick(double delta) {
    spdlog::debug("processTick, delta: {}", delta);
    cur_time += delta > 0 ? delta / 1000.0 : 0;

    GwidiAction* action{nullptr};
    if(this->m_midi_data) {
        action = processMidiTick();
    }
    else if(this->m_gui_data) {
        action = processGuiTick();
    }
    else {
        action = nullptr;
    }

    filterByOctaveBehavior(action);
    return action;
}

void GwidiTickHandler::setOptions(GwidiTickOptions o) {
    this->options = o;
}

}
