#include <spdlog/spdlog.h>
#include "GwidiPlayback.h"

#if defined(WIN32) || defined(WIN64)
#include "WindowsSendInput.h"
#define TEST_FILE R"(E:\Tools\repos\gwidi_midi_parser\assets\super_mario.mid)"
#elif defined(__linux__)
#include "UdpSendInput.h"

//#define TEST_FILE R"(/home/zhensley/repos/gwidi_godot/gwidi_midi_parser/assets/test3_data.mid)"
#define TEST_FILE R"(/home/zhensley/repos/gwidi_godot/gwidi_midi_parser/assets/slow_scale.mid)"
//#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/pollyanna7.mid)"
//#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/super_mario.mid)"
//#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/undertale_snowy.mid)"
//#define TEST_FILE R"(/home/zphensley42/repos/gwidi_midi_parser/assets/whats_new_scooby_doo.mid)"
//#define TEST_FILE R"(/home/zhensley/repos/gwidi_godot/gwidi_midi_parser/assets/moana.mid)"
#endif


namespace gwidi::playback {

GwidiPlayback::GwidiPlayback(const std::string &instrument) {
    auto mapping = gwidi::options2::GwidiOptions2::getInstance().getMapping()[instrument];
    m_startingOctave = mapping.starting_octave;
}

GwidiPlayback::~GwidiPlayback() {
    stop();
}

void GwidiPlayback::assignData(gwidi::data::midi::GwidiMidiData* data, gwidi::tick::GwidiTickOptions options) {
    m_handler.setOptions(options);
    m_handler.assignData(data);
}
void GwidiPlayback::assignData(gwidi::data::gui::GwidiGuiData* data, gwidi::tick::GwidiTickOptions options) {
    m_handler.setOptions(options);
    m_handler.assignData(data);
}

void GwidiPlayback::sendInput(const std::string &key) {
    if(m_realInput) {
        m_input.sendInput(key);
    }
}

void GwidiPlayback::swapOctaveUp() {
    spdlog::info("--BEGIN swapping octave up--");
    spdlog::info("old octave: {}", m_startingOctave);
    sendInput("0");
    m_startingOctave++;
    spdlog::info("new octave: {}", m_startingOctave);
    spdlog::info("--END swapping octave up--");
}
void GwidiPlayback::swapOctaveDown() {
    spdlog::info("--BEGIN swapping octave down--");
    spdlog::info("old octave: {}", m_startingOctave);
    sendInput("9");
    m_startingOctave--;
    spdlog::info("new octave: {}", m_startingOctave);
    spdlog::info("--END swapping octave down--");
}
int GwidiPlayback::octaveSwapDelay(gwidi::tick::GwidiAction *action) const {
    if(action->chosen_octave != -1) {
        int diff = abs(action->chosen_octave - m_startingOctave);
        return diff > 1 ? 200 : 10;
    }
    return 0;
}

std::chrono::milliseconds GwidiPlayback::curTime() {
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
}

double GwidiPlayback::getDelta(std::chrono::milliseconds startTime) {
    auto cTime = curTime();
    auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(cTime - startTime).count();
    return deltaTime;
}

void GwidiPlayback::play() {
    m_pausedCv.notify_all();
    std::lock_guard<std::mutex> lock(m_playbackThreadMutex);
    switch(m_threadState) {
        case STOPPED: {
            m_threadState = STARTED;
            m_playbackThread = std::thread(&GwidiPlayback::thread_cb, this);
            m_playbackThread.detach();
            break;
        }
        case STARTED: {
            return;
        }
        case PAUSED: {
            m_threadState = STARTED;
            return;
        }
    }
}

void GwidiPlayback::pause() {
    std::lock_guard<std::mutex> lock(m_playbackThreadMutex);
    switch(m_threadState) {
        case STOPPED: {
            return;
        }
        case STARTED: {
            m_threadState = PAUSED;
            return;
        }
        case PAUSED: {
            return;
        }
    }
}

void GwidiPlayback::stop() {
    m_pausedCv.notify_all();
    std::lock_guard<std::mutex> lock(m_playbackThreadMutex);
    switch(m_threadState) {
        case STOPPED: {
            return;
        }
        case STARTED:
        case PAUSED: {
            m_threadState = STOPPED;
            return;
        }
    }
}

void GwidiPlayback::thread_cb() {
    if(!m_handler.hasData()) {
        m_threadState = STOPPED;
        return;
    }

    // TODO: Wait/notify on state when it moves to paused
    auto startTime = curTime();
    while(m_threadState != STOPPED) {
        if(m_threadState == PAUSED) {
            std::unique_lock<std::mutex> lock(m_playbackThreadMutex);
            m_pausedCv.wait(lock);
            startTime = curTime();  // reset the start time to get the proper delta after resuming from a pause
        }
        auto delta = getDelta(startTime);
        startTime = curTime();

        auto action = m_handler.processTick(delta);

        if(m_tickCbFn) {
            m_tickCbFn(m_handler.curTime());
        }

        spdlog::debug("BEGIN Action notes-------");
        spdlog::debug("Chosen octave: {}", action->chosen_octave);
        swapOctaves(action);
        playNotes(action);
        spdlog::debug("END Action notes---------");

        if(action->end_reached) {
            spdlog::info("Reached end of track, exiting...");
            m_threadState = STOPPED;
            continue;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds (10));
    }

    // Reset for the next playback
    m_handler.reset();
    if(m_playEndedCbFn) {
        m_playEndedCbFn();
    }
}

void GwidiPlayback::swapOctaves(gwidi::tick::GwidiAction *action) {
    int delay = octaveSwapDelay(action);
    while(action->chosen_octave != -1 && action->chosen_octave != m_startingOctave) {
        if(action->chosen_octave > m_startingOctave) {
            swapOctaveUp();
        }
        else if(action->chosen_octave < m_startingOctave) {
            swapOctaveDown();
            // Need some delay if we are swapping multiple octaves, seems like the time it takes to swap the skill bar
            // interferes with doing multiple octave swaps quickly
        }
        spdlog::info("waiting octave swap delay: {}", delay);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        delay = octaveSwapDelay(action);    // update to reduce the next swap delay if needed
    }
}

void GwidiPlayback::playNotes(gwidi::tick::GwidiAction *action) {
    if(m_playCbFn) {
        m_playCbFn(action);
    }
    for(auto &n : action->notes) {
        spdlog::debug("note key: {}, start_offset: {}, octave: {}", n.key, n.start_offset, n.octave);

        // for testing, react to the actions
        if(n.octave == action->chosen_octave) {
            // pick our key from the note
            spdlog::info("Sending input key: {}", n.key);
            sendInput(n.key);
        }
    }
}

void GwidiPlayback::setRealInput(bool real) {
    m_realInput = real;
}

}