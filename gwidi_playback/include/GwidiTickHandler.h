#ifndef GWIDI_MIDI_PARSER_GWIDITICKHANDLER_H
#define GWIDI_MIDI_PARSER_GWIDITICKHANDLER_H

#include <memory>

#include "GwidiOptions2.h"

#include "GwidiMidiData.h"
#include "GwidiGuiData.h"
#include "gwidi_midi_parser.h"

namespace gwidi::tick {

struct ActionNote {
    double start_offset;
    int octave;
    std::string key;
};

struct GwidiAction {
    std::vector<ActionNote> notes{};
    int chosen_octave{-1};
    bool end_reached{false};
};

struct GwidiTickOptions {
    enum ActionOctaveBehavior {
        LOWEST = 0,
        HIGHEST = 1,
        MOST = 2
    };

    ActionOctaveBehavior octaveBehavior{ActionOctaveBehavior{LOWEST}};
};

class GwidiTickHandler_Impl {
public:
    virtual double tickMapFloorKey(double time) = 0;
    virtual GwidiAction* processTick(double time) = 0;
    virtual bool hasData() = 0;
    virtual void reset() = 0;
};

class GwidiTickHandler_MidiImpl : public GwidiTickHandler_Impl {
public:
    using TickMapTrackingType = std::map<double, std::vector<size_t>>; // int is a hash of the note's attributes (start_offset, octave, key)

    void assignData(gwidi::data::midi::GwidiMidiData* data);

    double tickMapFloorKey(double time) override;
    GwidiAction* processTick(double time)  override;

    inline bool hasData() override {
        return m_midi_data != nullptr;
    }

    void reset() override;

private:
    gwidi::data::midi::GwidiMidiData* m_midi_data{nullptr};
    TickMapTrackingType m_tick_tracking;
};

class GwidiTickHandler_GuiImpl : public GwidiTickHandler_Impl {
public:
    using TickMapTrackingType = std::map<double, std::vector<size_t>>; // int is a hash of the note's attributes (start_offset, octave, key)

    void assignData(gwidi::data::gui::GwidiGuiData* data);

    double tickMapFloorKey(double time) override;
    GwidiAction* processTick(double time)  override;

    inline bool hasData() override {
        return m_gui_data != nullptr;
    }

    void reset() override;

private:
    gwidi::data::gui::GwidiGuiData* m_gui_data{nullptr};
    TickMapTrackingType m_tick_tracking;
};

class GwidiTickHandler {
public:
    void setOptions(GwidiTickOptions options);
    void assignData(gwidi::data::midi::GwidiMidiData* data);
    void assignData(gwidi::data::gui::GwidiGuiData* data);
    GwidiAction* processTick(double delta);

    void reset();

    inline bool hasData() {
        return m_impl && m_impl->hasData();
    }

    inline double curTime() {
        return cur_time;
    }

private:
    std::shared_ptr<GwidiTickHandler_Impl> m_impl;
    void filterByOctaveBehavior(GwidiAction *action) const;\


//    double currentTickMapFloorKey();
//    void filterByOctaveBehavior(GwidiAction *action) const;

//    GwidiAction* processMidiTick();
//    GwidiAction* processGuiTick();

//    Note fromNote(gwidi::data::midi::Note &note);
//    Note fromNote(gwidi::data::gui::Note &note);

    GwidiTickOptions options;
    double cur_time{0.0};
};

}

#endif //GWIDI_MIDI_PARSER_GWIDITICKHANDLER_H
