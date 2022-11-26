#ifndef GWIDI_MIDI_PARSER_GWIDISERVERCLIENT_H
#define GWIDI_MIDI_PARSER_GWIDISERVERCLIENT_H

#include <memory>
#include <atomic>
#include <thread>
#include <netinet/in.h>
#include <functional>
#include <utility>

// TODO: Make connection to server more resilient (i.e. don't need server started first for this to work, poll until we connect?)

namespace gwidi::udpsocket {

// TODO: Instead of redefining these, pull them from the gwidi_server header
enum ServerEventType {
    EVENT_HELLO = 0,
    EVENT_KEY = 1,
    EVENT_FOCUS = 2,
    EVENT_WATCHEDKEYS_RECONFIGURE = 3
};

struct KeyEvent {
    int code;
    int eventType;  // 0 == release, 1 == pressed
};

struct WindowFocusEvent {
    std::size_t windowNameSize;
    const char* windowName;
    bool hasFocus;
};

union ServerEvent {
    KeyEvent keyEvent;
    WindowFocusEvent focusEvent;
};


// Send things to the server
class GwidiServerClient {
public:
    GwidiServerClient() = delete;
    explicit GwidiServerClient(const sockaddr_in& toAddr);

    void sendHello();
    void sendWatchedKeysReconfig(std::vector<int> watchedKeys);

    inline void markReceived() {
        m_receivedHello = true;
    }
    inline bool isReceived() {
        return m_receivedHello;
    }

private:
    bool m_receivedHello{false};
    int sockfd;
    struct sockaddr_in m_toAddr;
};

// Listen for data on the server's udp socket
class GwidiServerListener {
public:
    using EventCb = std::function<void(ServerEventType, ServerEvent)>;

    GwidiServerListener();
    ~GwidiServerListener();

    void start();
    void stop();

    void sendWatchedKeysReconfig(std::vector<int> watchedKeys);

    void processEvent(char* buffer);

    inline bool isAlive() {
        return m_thAlive.load();
    }

    inline void setEventCb(EventCb cb) {
        m_eventCb = std::move(cb);
    }

private:
    EventCb m_eventCb;

    std::atomic_bool m_thAlive{false};
    std::shared_ptr<std::thread> m_th;
    GwidiServerClient* m_socketClient{nullptr};
};

class GwidiServerClientManager {
public:
    static GwidiServerClientManager& instance();

    std::shared_ptr<GwidiServerListener> serverListener();

private:
    GwidiServerClientManager();

    std::shared_ptr<GwidiServerListener> m_serverListener;
};

}

#endif //GWIDI_MIDI_PARSER_GWIDISERVERCLIENT_H
