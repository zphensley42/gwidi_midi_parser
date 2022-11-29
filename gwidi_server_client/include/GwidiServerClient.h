#ifndef GWIDI_MIDI_PARSER_GWIDISERVERCLIENT_H
#define GWIDI_MIDI_PARSER_GWIDISERVERCLIENT_H

#include <memory>
#include <atomic>
#include <thread>
#include <netinet/in.h>
#include <functional>
#include <utility>
#include <list>
#include <map>
#include <string>
#include <queue>
#include <mutex>
#include <shared_mutex>

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
    using WatchedKeysConfig = std::vector<int>;

    GwidiServerClient() = delete;
    explicit GwidiServerClient(const sockaddr_in& toAddr);

    void sendHello();
    void sendWatchedKeysReconfig(const std::vector<int>& watchedKeys);

    inline void markReceived();
    inline bool isReceived() const {
        return m_receivedHello;
    }
    inline void queueWatchedKeysReconfig(const std::vector<int>& watchedKeys) {
        m_watchedKeysReconfigQueue.emplace(watchedKeys);
    }

private:
    std::queue<WatchedKeysConfig> m_watchedKeysReconfigQueue;

    bool m_receivedHello{false};
    int sockfd;
    struct sockaddr_in m_toAddr{};
};

// Listen for data on the server's udp socket
class GwidiServerListener {
public:
    using EventCb = std::function<void(ServerEventType, ServerEvent)>;

    GwidiServerListener();
    ~GwidiServerListener();

    void start();
    void stop();

    void sendWatchedKeysReconfig(const std::vector<int>& watchedKeys);

    void processEvent(char* buffer);

    inline bool isAlive() {
        return m_thAlive.load();
    }

    void addEventCb(const std::string &identifier, const EventCb& cb);

    void removeEventCb(const std::string &identifier);

    void iterateEvents(ServerEventType type, ServerEvent event);

private:
    std::list<std::function<void()>> m_deferredActions;

    mutable std::mutex m_eventCbsMutex;
    std::unique_lock<std::mutex> m_eventCbsLock{m_eventCbsMutex, std::defer_lock};
    std::map<std::string, EventCb> m_eventCbs;

    std::atomic_bool m_thAlive{false};
    std::shared_ptr<std::thread> m_th;
    GwidiServerClient* m_socketClient{nullptr};

    static std::vector<int> s_helloTimeoutS;
    std::size_t m_helloRetryCount{0};
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
