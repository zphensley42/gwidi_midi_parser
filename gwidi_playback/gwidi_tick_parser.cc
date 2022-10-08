#include <iostream>
#include <sstream>
#include <map>
#include "spdlog/spdlog.h"
#include "GwidiTickHandler.h"

void GwidiTickHandler::assignData(GwidiData *d) {
    this->data = d;
    auto &tm = data->getTickMap();

    // Copy to our wrapper type
    tickMap.clear();
    for(auto &entry : tm) {
        tickMap[entry.first] = std::map<double, std::vector<NoteWrapper>>();
        for(auto &trackEntry : entry.second) {
            tickMap[entry.first][trackEntry.first] = std::vector<NoteWrapper>();
            for(auto &n : trackEntry.second) {
                tickMap[entry.first][trackEntry.first].emplace_back(NoteWrapper{n});
            }
        }
    }
}

std::unordered_map<int, double> GwidiTickHandler::currentTickMapFloorKey() {
    std::unordered_map<int, double> ret;
    for(auto &entry : tickMap) {

        double bound_key = -1.0;
        auto bound_itr = entry.second.lower_bound(cur_time);
        if(bound_itr != entry.second.end()) {
            if(bound_itr == entry.second.begin()) {
                bound_key = bound_itr->first;
            }
            else {
                bound_key = (--bound_itr)->first;
            }
        }
        ret[entry.first] = bound_key;
        spdlog::debug("currentTickMapFloorKey cur_time: {}, bound_key: {}", cur_time, bound_key);
    }
    return ret;
}

GwidiAction* GwidiTickHandler::processTick(double delta) {
    spdlog::debug("processTick, delta: {}", delta);
    cur_time += delta > 0 ? delta /  1000.0 : 0;
    if(!this->data) {
        return nullptr;
    }

    // Search for the closest map of notes per our delta
    // The map cannot come before our delta, so this is a floor of our delta
    // Mark the notes as played such that we don't trigger them more than once
    // TODO: Need to handle start/stop for flute-type instruments. i.e. start and hold for duration, then stop at the end of the duration
    // TODO: For this, probably need actions to not just be list of notes
    // TODO: Instead, make actions be related to the options for the instruments? (i.e. play via number and stop or so on)

    auto action = new GwidiAction();
    if(cur_time >= data->longestTrackDuration()) {
        action->end_reached = true;
    }

    auto floorKeys = currentTickMapFloorKey();
    spdlog::debug("processTick, cur_time: {}", cur_time);
    spdlog::debug("processTick, -----BEGIN floorKeys------");
    for(auto &entry : floorKeys) {
        spdlog::debug("track: {}, key: {}", entry.first, entry.second);
        if(entry.second == -1.0) {
            continue;
        }
        auto &notes = tickMap[entry.first][entry.second];
        for(auto& n : notes) {
            if(!n.activated) {
                n.activated = true;
                action->notes.emplace_back(&n);
            }
        }
    }
    spdlog::debug("processTick, -----END floorKeys------");


    // TODO: Need to handle octave switches on the action
    // TODO: A single action cannot play notes across octaves
    // TODO: There is some timing needed to play a note, switch an octave, then play a new note

    // Some determinations can be made for how to treat cases where multiple octaves are present:
    // Option 1: Always choose the lowest octave (kill other notes)
    // Option 2: Always choose the higest octave (kill other notes)
    // Option 3: Always choose the octave with the most notes in the action (kill other notes)
    switch(this->options.octaveBehavior) {
        case GwidiTickOptions::ActionOctaveBehavior::LOWEST: {
            // First, find the lowest octave
            int octave = -1;
            for(auto &n : action->notes) {
                if(octave == -1 || n->note.octave < octave) {
                    octave = n->note.octave;
                }
            }

            // Then, disregard notes that are not of this chosen octave
            action->chosen_octave = octave;
            break;
        }
        case GwidiTickOptions::ActionOctaveBehavior::HIGHEST: {
            // First, find the highest octave
            int octave = -1;
            for(auto &n : action->notes) {
                if(octave == -1 || n->note.octave > octave) {
                    octave = n->note.octave;
                }
            }
            action->chosen_octave = octave;
            break;
        }
        case GwidiTickOptions::ActionOctaveBehavior::MOST: {
            // Build list of octave counts
            std::unordered_map<int, int> octave_counts;
            for(auto &n : action->notes) {
                if(octave_counts.find(n->note.octave) == octave_counts.end()) {
                    octave_counts[n->note.octave] = 0;
                }
                octave_counts[n->note.octave]++;
            }
            // Then pick the most
            int octave_count = -1;
            int octave = -1;
            for(auto &entry : octave_counts) {
                if(octave == -1 || entry.second > octave_count) {
                    octave = entry.first;
                    octave_count = entry.second;
                }
            }
            action->chosen_octave = octave;
            break;
        }
    }


    return action;
}

void GwidiTickHandler::setOptions(GwidiTickOptions o) {
    this->options = o;
}
