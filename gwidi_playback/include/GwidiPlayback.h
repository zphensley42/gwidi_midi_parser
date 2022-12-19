#ifndef GWIDI_MIDI_PARSER_GWIDIPLAYBACK_H
#define GWIDI_MIDI_PARSER_GWIDIPLAYBACK_H

#if defined(WIN32) || defined(WIN64)
#include "WindowsSendInput.h"
#elif defined(__linux__)
#include "UdpSendInput.h"
#endif

#include <thread>
#include <mutex>
#include <condition_variable>
#include "GwidiTickHandler.h"
#include <chrono>
#include <string>
#include <functional>

namespace gwidi::playback {

class GwidiPlayback {
public:
    using TickCbFn = std::function<void(double)>;
    using PlayCbFn = std::function<void(gwidi::tick::GwidiAction*)>;
    using PlayEndedCbFn = std::function<void()>;

    GwidiPlayback() : GwidiPlayback("default") {}
    explicit GwidiPlayback(const std::string &instrument);
    ~GwidiPlayback();

    void assignData(gwidi::data::midi::GwidiMidiData* data, gwidi::tick::GwidiTickOptions options);
    void assignData(gwidi::data::gui::GwidiGuiData* data, gwidi::tick::GwidiTickOptions options);

    inline void setTickCb(TickCbFn cb) {
        m_tickCbFn = cb;
    }

    inline void setPlayCb(PlayCbFn cb) {
        m_playCbFn = cb;
    }

    inline void setPlayEndedCb(PlayEndedCbFn cb) {
        m_playEndedCbFn = cb;
    }

    void setRealInput(bool real);

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
    TickCbFn m_tickCbFn;
    PlayCbFn m_playCbFn;
    PlayEndedCbFn m_playEndedCbFn;

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
    std::condition_variable m_pausedCv;
    std::mutex m_playbackThreadMutex;

    gwidi::tick::GwidiTickHandler m_handler;

    bool m_realInput{true};
};

}

#endif //GWIDI_MIDI_PARSER_GWIDIPLAYBACK_H
