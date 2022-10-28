#include <spdlog/spdlog.h>
#include "gwidi_hotkey.h"

#include <linux/input.h>
#include <csignal>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <filesystem>

#define EVT_DEVICE "/dev/input/event7"

namespace gwidi::hotkey {

int GwidiHotkey::m_timeoutMs{5000};

bool supports_key_events(const int &fd) {
    unsigned long evbit = 0;
    ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), &evbit);
    return (evbit & (1 << EV_KEY));
}

void GwidiHotkey::findInputDevices() {
    // Requires root
    auto uid = getuid();
    if(uid != 0) {
        spdlog::warn("User is not root, cross-process hotkeys may not function!");
    }

    char eventPathStart[] = "/dev/input/event";
    m_inputDevices.clear();
    for (auto &i : std::filesystem::directory_iterator("/dev/input")) {
        if(i.is_character_file()) {
            std::string view(i.path());
            if (view.compare(0, sizeof(eventPathStart)-1, eventPathStart) == 0) {
                int evfile = open(view.c_str(), O_RDONLY | O_NONBLOCK);
                if(supports_key_events(evfile)) {
                    spdlog::debug("Adding {}", i.path().c_str());
                    m_inputDevices.push_back({evfile, POLLIN, 0});
                } else {
                    close(evfile);
                }
            }
        }
    }
}

void GwidiHotkey::beginListening() {
    findInputDevices();

    // Requires root
    auto uid = getuid();
    if(uid != 0) {
        spdlog::warn("User is not root, cross-process hotkeys may not function!");
    }


    // from input-event-codes.h
//#define KEY_1			2
//#define KEY_2			3
//#define KEY_3			4
//#define KEY_4			5
//#define KEY_5			6
//#define KEY_6			7
//#define KEY_7			8
//#define KEY_8			9
//#define KEY_9			10
//#define KEY_0			11

    input_event ev{};
    while(true) {
        poll(m_inputDevices.data(), m_inputDevices.size(), m_timeoutMs);
        for (auto &pfd : m_inputDevices) {
            if (pfd.revents & POLLIN) {
                if(read(pfd.fd, &ev, sizeof(ev)) == sizeof(ev)) {
                    // value: or 0 for EV_KEY for release, 1 for keypress and 2 for autorepeat
                    if(ev.type == EV_KEY && ev.code < 0x100) {
                        if(ev.value == 0) {
                            spdlog::info("Key released: {}", ev.code);
                        }
                        else if(ev.value == 1) {
                            spdlog::info("Key pressed: {}", ev.code);
                        }
                        else if(ev.value == 2) {
                            spdlog::info("Key autorepeat: {}", ev.code);
                        }
                    }
                }
            }
        }
    }

    for(auto &pfd : m_inputDevices) {
        close(pfd.fd);
    }
}

}