#include <iostream>
#include <sstream>
#include <map>
#include "spdlog/spdlog.h"
#include "GwidiTickHandler.h"

namespace gwidi::tick {

void GwidiTickHandler::assignData(gwidi::data::midi::GwidiMidiData *data) {
    auto impl = std::make_shared<GwidiTickHandler_MidiImpl>();
    impl->assignData(data);
    m_impl = impl;
}

void GwidiTickHandler::assignData(gwidi::data::gui::GwidiGuiData *data) {
    auto impl = std::make_shared<GwidiTickHandler_GuiImpl>();
    impl->assignData(data);
    m_impl = impl;
}

//Note GwidiTickHandler::fromNote(gwidi::data::midi::Note &note) {
//    return Note{
//        note.start_offset,
//        note.duration,
//        note.octave,
//        note.key,
//        false
//    };
//}
//Note GwidiTickHandler::fromNote(gwidi::data::gui::Note &note) {
//    return Note {
//        m_gui_data->timeIndexToTickOffset(&note),
//        1.0,    // no sense of 'duration' for gui currently, everything is a product of 1/<num notes per measure>
//        note.octave,
//        note.key,
//        false   // tick 'activated' is whether it is played or not (instead of whether it is drawn as enabled or not from gui)
//    };
//}

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
                if (octave == -1 || n.octave < octave) {
                    octave = n.octave;
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
                if (octave == -1 || n.octave > octave) {
                    octave = n.octave;
                }
            }
            action->chosen_octave = octave;
            break;
        }
        case GwidiTickOptions::ActionOctaveBehavior::MOST: {
            // Build list of octave counts
            std::unordered_map<int, int> octave_counts;
            for (auto &n: action->notes) {
                if (octave_counts.find(n.octave) == octave_counts.end()) {
                    octave_counts[n.octave] = 0;
                }
                octave_counts[n.octave]++;
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

    GwidiAction* action = m_impl->processTick(cur_time);
    filterByOctaveBehavior(action);
    return action;
}

void GwidiTickHandler::setOptions(GwidiTickOptions o) {
    this->options = o;
}

void GwidiTickHandler::reset() {
    cur_time = 0;
    if(m_impl) {
        m_impl->reset();
    }
}


void GwidiTickHandler_MidiImpl::assignData(gwidi::data::midi::GwidiMidiData *data) {
    m_midi_data = data;
}

GwidiAction *GwidiTickHandler_MidiImpl::processTick(double time) {
    auto &tickMap = m_midi_data->getTickMap();

    // Search for the closest map of notes per our delta
    // The map cannot come before our delta, so this is a floor of our delta
    // Mark the notes as played such that we don't trigger them more than once
    // TODO: Need to handle start/stop for flute-type instruments. i.e. start and hold for duration, then stop at the end of the duration
    // TODO: For this, probably need actions to not just be list of notes
    // TODO: Instead, make actions be related to the options for the instruments? (i.e. play via number and stop or so on)

    auto action = new GwidiAction();
    if (time >= m_midi_data->longestTrackDuration()) {
        action->end_reached = true;
    }

    auto floorKey = tickMapFloorKey(time);
    spdlog::debug("processTick, cur_time: {}", time);
    spdlog::debug("processTick, -----BEGIN floorKeys------");
    if(floorKey != -1.0) {
        spdlog::debug("key: {}", floorKey);
        auto &notes = tickMap[floorKey];
        // TODO: More efficient here would be to remove from the map after we complete the action
        // TODO: Need a feedback mechanism? Maybe not, maybe we just assume the return of the action is enough
        if(m_tick_tracking.find(floorKey) == m_tick_tracking.end()) {
            m_tick_tracking[floorKey] = std::vector<size_t>();
        }

        for (auto &n: notes) {
            auto &tracking = m_tick_tracking[floorKey];
            auto hash = n.hash();
            auto activated = std::find(tracking.begin(), tracking.end(), hash) != tracking.end();
            if(!activated) {
                ActionNote an{
                        n.start_offset,
                        n.octave,
                        n.key
                };
                m_tick_tracking[floorKey].emplace_back(hash);
                action->notes.emplace_back(an);
            }
        }
    }
    spdlog::debug("processTick, -----END floorKeys------");

    return action;
}

double GwidiTickHandler_MidiImpl::tickMapFloorKey(double time) {
    if(!m_midi_data) {
        return -1.f;
    }
    auto &tickMap = m_midi_data->getTickMap();

    double bound_key = -1.0;
    auto bound_itr = tickMap.lower_bound(time);

    // We have 3 cases:
    // 1 - cur_time is < all keys, lower_bound returns begin, return first()
    // 2 - cur_time is > some keys, < some keys, lower_bound returns first >=, return --itr
    // 3 - cur_time is > all keys, lower_bound returns end, return back()
    if(bound_itr == tickMap.begin()) {
        bound_key = bound_itr->first;
    }
    else if(bound_itr == tickMap.end()) {
        bound_key = (--tickMap.end())->first;
    }
    else {
        bound_key = (--bound_itr)->first;
    }
    spdlog::debug("currentTickMapFloorKey cur_time: {}, bound_key: {}", time, bound_key);

    return bound_key;
}

void GwidiTickHandler_MidiImpl::reset() {
    m_tick_tracking.clear();
}



void GwidiTickHandler_GuiImpl::assignData(gwidi::data::gui::GwidiGuiData *data) {
    m_gui_data = data;
}

GwidiAction *GwidiTickHandler_GuiImpl::processTick(double time) {
    auto &tickMap = m_gui_data->getTickMap();

    // Search for the closest map of notes per our delta
    // The map cannot come before our delta, so this is a floor of our delta
    // Mark the notes as played such that we don't trigger them more than once
    // TODO: Need to handle start/stop for flute-type instruments. i.e. start and hold for duration, then stop at the end of the duration
    // TODO: For this, probably need actions to not just be list of notes
    // TODO: Instead, make actions be related to the options for the instruments? (i.e. play via number and stop or so on)

    auto action = new GwidiAction();
    if (time >= m_gui_data->trackDuration()) {
        action->end_reached = true;
    }

    auto floorKey = tickMapFloorKey(time);
    spdlog::debug("processTick, cur_time: {}", time);
    spdlog::debug("processTick, -----BEGIN floorKeys------");
    if(floorKey != -1.0) {
        spdlog::debug("key: {}", floorKey);
        auto &notes = tickMap[floorKey];
        // TODO: More efficient here would be to remove from the map after we complete the action
        // TODO: Need a feedback mechanism? Maybe not, maybe we just assume the return of the action is enough
        if(m_tick_tracking.find(floorKey) == m_tick_tracking.end()) {
            m_tick_tracking[floorKey] = std::vector<size_t>();
        }


        for (auto &n: notes) {
            auto &tracking = m_tick_tracking[floorKey];
            auto hash = n.hash();
            auto activated = std::find(tracking.begin(), tracking.end(), hash) != tracking.end();
            if(!activated) {
                ActionNote an{
                    m_gui_data->timeIndexToTickOffset(&n),
                    n.octave,
                    n.key
                };
                m_tick_tracking[floorKey].emplace_back(hash);
                action->notes.emplace_back(an);
            }
        }
    }
    spdlog::debug("processTick, -----END floorKeys------");

    return action;
}

double GwidiTickHandler_GuiImpl::tickMapFloorKey(double time) {
    if(!m_gui_data) {
        return -1.f;
    }
    auto &tickMap = m_gui_data->getTickMap();

    double bound_key = -1.0;
    auto bound_itr = tickMap.lower_bound(time);

    // We have 3 cases:
    // 1 - cur_time is < all keys, lower_bound returns begin, return first()
    // 2 - cur_time is > some keys, < some keys, lower_bound returns first >=, return --itr
    // 3 - cur_time is > all keys, lower_bound returns end, return back()
    if(bound_itr == tickMap.begin()) {
        bound_key = bound_itr->first;
    }
    else if(bound_itr == tickMap.end()) {
        bound_key = (--tickMap.end())->first;
    }
    else {
        bound_key = (--bound_itr)->first;
    }
    spdlog::debug("currentTickMapFloorKey cur_time: {}, bound_key: {}", time, bound_key);

    return bound_key;
}

void GwidiTickHandler_GuiImpl::reset() {
    m_tick_tracking.clear();
}
}
