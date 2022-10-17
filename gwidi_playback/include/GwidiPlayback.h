#ifndef GWIDI_MIDI_PARSER_GWIDIPLAYBACK_H
#define GWIDI_MIDI_PARSER_GWIDIPLAYBACK_H

#if defined(WIN32) || defined(WIN64)
#include "WindowsSendInput.h"
#elif defined(__linux__)
#include "LinuxSendInput.h"
#endif

#include <thread>
#include <mutex>
#include "GwidiTickHandler.h"
#include <chrono>
#include <string>

namespace gwidi::playback {

class GwidiPlayback {
public:
    GwidiPlayback() : GwidiPlayback(gwidi::data::gui::nameForInstrument(gwidi::data::gui::Instrument::HARP)) {}
    explicit GwidiPlayback(const std::string &instrument);
    ~GwidiPlayback();

    void assignData(gwidi::data::midi::GwidiMidiData* data, gwidi::tick::GwidiTickOptions options);
    void assignData(gwidi::data::gui::GwidiGuiData* data, gwidi::tick::GwidiTickOptions options);

    void play();
    void pause();
    void stop();

    inline bool isPlaying() const {
        return m_threadState == PlayThreadState::STARTED;
    }
    inline bool isPaused() const {
        return m_threadState == PlayThreadState::PAUSED;
    }
    inline bool isStopped() const {
        return m_threadState == PlayThreadState::STOPPED;
    }

private:
    void thread_cb();

    void sendInput(const std::string &key);

    void swapOctaveUp();
    void swapOctaveDown();
    int octaveSwapDelay(gwidi::tick::GwidiAction *action) const;
    void swapOctaves(gwidi::tick::GwidiAction *action);
    void playNotes(gwidi::tick::GwidiAction *action);

    static std::chrono::milliseconds curTime();
    static double getDelta(std::chrono::milliseconds startTime);

    SendInput m_input;
    int m_startingOctave {0};

    enum PlayThreadState {
        STOPPED = 0,
        STARTED = 1,
        PAUSED = 2
    };
    std::thread m_playbackThread;
    PlayThreadState m_threadState{STOPPED};
    std::mutex m_playbackThreadMutex;

    gwidi::tick::GwidiTickHandler m_handler;
};

}

#endif //GWIDI_MIDI_PARSER_GWIDIPLAYBACK_H
