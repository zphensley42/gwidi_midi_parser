#include <spdlog/spdlog.h>
#include "GwidiPlayback.h"

#if defined(WIN32) || defined(WIN64)
#include "WindowsSendInput.h"
#define TEST_FILE R"(E:\Tools\repos\gwidi_midi_parser\assets\super_mario.mid)"
#elif defined(__linux__)
#include "LinuxSendInput.h"

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
    m_input.sendInput(key);
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
    std::unique_lock<std::mutex> lock(m_playbackThreadMutex);
    switch(m_threadState) {
        case STOPPED: {
            return;
        }
        case STARTED:
        case PAUSED: {
            m_threadState = STOPPED;
            lock.unlock();
            m_playbackThread.join();
            return;
        }
    }
}

void GwidiPlayback::thread_cb() {
    if(!m_handler.hasData()) {
        m_threadState = STOPPED;
        return;
    }

    auto startTime = curTime();
    while(m_threadState != STOPPED) {
        auto delta = getDelta(startTime);
        startTime = curTime();

        auto action = m_handler.processTick(delta);
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
    for(auto &n : action->notes) {
        spdlog::debug("note key: {}, start_offset: {}, duration: {}", n->key, n->start_offset, n->duration);

        // for testing, react to the actions
        if(n->octave == action->chosen_octave) {
            // pick our key from the note
            spdlog::info("Sending input key: {}", n->key);
            sendInput(n->key);
        }
    }
}

}