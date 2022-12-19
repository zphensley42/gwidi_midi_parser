#include <cstring>
#include "GwidiServerClient.h"
#include <arpa/inet.h>
#include <string>
#include <utility>
#include <spdlog/spdlog.h>

namespace gwidi::udpsocket {

std::string ipForSin(const sockaddr_in& sin) {
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(sin.sin_addr), str, INET_ADDRSTRLEN);
    return {str};
}

GwidiServerClient::GwidiServerClient(const sockaddr_in &toAddr) {
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&m_toAddr, '\0', sizeof(m_toAddr));
    m_toAddr.sin_family = AF_INET;
    m_toAddr.sin_port = htons(5577);
    m_toAddr.sin_addr.s_addr = toAddr.sin_addr.s_addr;
}

void GwidiServerClient::sendHello() {
    auto toIp = ipForSin(m_toAddr);

    char* buffer = new char[1024];
    memset(buffer, '\0', sizeof(char) * 1024);

    std::size_t bufferOffset = 0;

    std::string msg = "msg_hello";
    std::size_t msg_size = msg.size();
    int msg_type = static_cast<int>(ServerEventType::EVENT_HELLO);

    memcpy(buffer + bufferOffset, &(msg_type), sizeof(int));
    bufferOffset += sizeof(int);

    memcpy(buffer + bufferOffset, &(msg_size), sizeof(msg_size));
    bufferOffset += sizeof(msg_size);

    memcpy(buffer + bufferOffset, &(msg[0]), sizeof(char) * msg_size);
    bufferOffset += sizeof(char) * msg_size;

    auto bytesSent = sendto(sockfd, buffer, 1024, 0, (struct sockaddr*)&m_toAddr, sizeof(m_toAddr));
    spdlog::info("Sent {} bytes of data", bytesSent);
}

void GwidiServerClient::sendWatchedKeysReconfig(const std::vector<int>& watchedKeys) {
    auto toIp = ipForSin(m_toAddr);

    char* buffer = new char[1024];
    memset(buffer, '\0', sizeof(char) * 1024);

    std::size_t bufferOffset = 0;

    int msg_type = static_cast<int>(ServerEventType::EVENT_WATCHEDKEYS_RECONFIGURE);
    memcpy(buffer + bufferOffset, &msg_type, sizeof(int));
    bufferOffset += sizeof(int);

    std::size_t listSize = watchedKeys.size();
    memcpy(buffer + bufferOffset, &listSize, sizeof(std::size_t));
    bufferOffset += sizeof(std::size_t);

    for(int k : watchedKeys) {
        memcpy(buffer + bufferOffset, &k, sizeof(int));
        bufferOffset += sizeof(int);
    }

    auto bytesSent = sendto(sockfd, buffer, 1024, 0, (struct sockaddr*)&m_toAddr, sizeof(m_toAddr));
    spdlog::info("Sent {} bytes of data", bytesSent);
}

void GwidiServerClient::sendKeyInputEvent(const std::string &key) {
    auto toIp = ipForSin(m_toAddr);

    char* buffer = new char[1024];
    memset(buffer, '\0', sizeof(char) * 1024);

    std::size_t bufferOffset = 0;

    int msg_type = static_cast<int>(ServerEventType::EVENT_SENDINPUT);
    memcpy(buffer + bufferOffset, &msg_type, sizeof(int));
    bufferOffset += sizeof(int);

    std::size_t keySize = key.size();
    memcpy(buffer + bufferOffset, &keySize, sizeof(std::size_t));
    bufferOffset += sizeof(std::size_t);

    memcpy(buffer + bufferOffset, &(key[0]), sizeof(char) * keySize);
    bufferOffset += sizeof(char) * keySize;

    auto bytesSent = sendto(sockfd, buffer, 1024, 0, (struct sockaddr*)&m_toAddr, sizeof(m_toAddr));
    spdlog::info("Sent {} bytes of data", bytesSent);
}

void GwidiServerClient::markReceived() {
    m_receivedHello = true;
    while(!m_watchedKeysReconfigQueue.empty()) {
        auto &cfg = m_watchedKeysReconfigQueue.front();
        sendWatchedKeysReconfig(cfg);
        m_watchedKeysReconfigQueue.pop();
    }
}

GwidiServerClientManager &GwidiServerClientManager::instance() {
    static GwidiServerClientManager instance;
    return instance;
}

GwidiServerClientManager::GwidiServerClientManager() {
    m_serverListener = std::make_shared<GwidiServerListener>();
}

std::shared_ptr<GwidiServerListener> GwidiServerClientManager::serverListener() {
    return m_serverListener;
}

GwidiServerListener::GwidiServerListener() = default;

std::vector<int> GwidiServerListener::s_helloTimeoutS{1, 5, 10, 20};

void GwidiServerListener::start() {
    if(m_thAlive.load()) {
        return;
    }

    m_thAlive.store(true);

    struct sockaddr_in socketIn_serverClient{};
    memset(&socketIn_serverClient, '\0', sizeof(socketIn_serverClient));
    socketIn_serverClient.sin_family = AF_INET;
    socketIn_serverClient.sin_port = htons(5577);
    socketIn_serverClient.sin_addr.s_addr = inet_addr("127.0.0.1");   // local listening server
    m_socketClient = new GwidiServerClient(socketIn_serverClient);

    m_th = std::make_shared<std::thread>([this] {
        // For now, port is here
        int port = 5578;
        int sockfd;
        struct sockaddr_in socketIn_server{}, socketIn_client{};
        char buffer[1024];  // probably too big for our needs
        socklen_t addr_size;

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);

        memset(&socketIn_server, '\0', sizeof(socketIn_server));
        socketIn_server.sin_family = AF_INET;
        socketIn_server.sin_port = htons(port);
        socketIn_server.sin_addr.s_addr = inet_addr("127.0.0.1");   // local listening server

        auto bindStatus = bind(sockfd, (struct sockaddr*)&socketIn_server, sizeof(socketIn_server));
        if(bindStatus != 0) {
            spdlog::warn("Failed to bind listening socket!");
            m_thAlive.store(false);
            return;
        }

        addr_size = sizeof(socketIn_client);
        while(m_thAlive.load()) {
            memset(buffer, '\0', sizeof(buffer));

            auto toWaitIndex = std::min(s_helloTimeoutS.size() - 1, m_helloRetryCount);
            auto toWait = s_helloTimeoutS[toWaitIndex];
            struct timeval tv{};
            tv.tv_sec = toWait;
            tv.tv_usec = 0;
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

            auto read = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)&socketIn_client, &addr_size);  // Should loop this to continue receiving messages
            if(read == -1) {
                // Error
                if(m_socketClient) {
                    if(!m_socketClient->isReceived()) { // TODO: Need to handle when the server restarts and we have marked as received, we will need to send hello again
                        // TODO: Possibly handle the above with a recurrent 'ping' to the server for connection status?
                        m_helloRetryCount++;
                        spdlog::error("Socket recvfrom error: {}", strerror(errno));

                        // Try again
                        m_socketClient->sendHello();
                    }
                }
            }
            else {
                m_helloRetryCount = 0;

                spdlog::info("Received message from client: {}, message: {}", ipForSin(socketIn_client), buffer);
                // TODO: Probably do some type of shared secret thing where we only accept clients we trust
                // TODO: For now, just look for a header message first to determine this is our client (assign only a single client at a time)

                processEvent(buffer);
            }
        }
        if(m_socketClient != nullptr) {
            delete m_socketClient;
            m_socketClient = nullptr;
        }

        m_thAlive.store(false);
    });
    m_th->detach();

    // After we create the server and bind/recv on the port, message the server to tell it hello (we then wait for helloback)
    // If we don't receive a helloback within a set period, retry the hello
    m_socketClient->sendHello();

}

void GwidiServerListener::stop() {
    m_thAlive.store(false);
}

void GwidiServerListener::sendWatchedKeysReconfig(const std::vector<int>& watchedKeys) {
    if(m_socketClient) {
        if(m_socketClient->isReceived()) {
            m_socketClient->sendWatchedKeysReconfig(watchedKeys);
        }
        else {
            m_socketClient->queueWatchedKeysReconfig(watchedKeys);
        }
    }
}

void GwidiServerListener::sendKeyInputEvent(const std::string& key) {
    if(m_socketClient) {
        if (m_socketClient->isReceived()) {
            m_socketClient->sendKeyInputEvent(key);
        }
    }
}

GwidiServerListener::~GwidiServerListener() {
    if(m_thAlive.load() && m_th->joinable()) {
        m_thAlive.store(false);
        m_th->join();
    }
    else {
        m_thAlive.store(false);
    }
}

void GwidiServerListener::processEvent(char *buffer) {
    // The message we expect to receive is of the format: [{msg_type}{size}{message}]
    std::size_t bufferOffset = 0;
    int msg_type;
    memcpy(&msg_type, buffer, sizeof(int));
    bufferOffset += sizeof(int);

    switch(static_cast<ServerEventType>(msg_type)) {
        case ServerEventType::EVENT_HELLO: {
            std::size_t msgSize;
            memcpy(&msgSize, buffer + bufferOffset, sizeof(std::size_t));
            bufferOffset += sizeof(std::size_t);

            char* msg = nullptr;
            if(msgSize > 0 && msgSize <= 1024) {
                msg = new char[msgSize];
                memcpy(&msg[0], buffer + bufferOffset, msgSize);
                bufferOffset += msgSize;
            }

            // verify our hello string
            auto helloMsgPre = "msg_helloback";
            auto selectionMessageMatched = strncmp(helloMsgPre, msg, strlen(helloMsgPre)) == 0;

            if(m_socketClient && selectionMessageMatched) {
                m_socketClient->markReceived();
            }
            break;
        }
        case ServerEventType::EVENT_KEY: {
            // Only process if we are already marked
            if(m_socketClient && m_socketClient->isReceived()) {
                spdlog::info("Key Message to pass to client: {}", buffer);

                int code;
                int type;

                memcpy(&code, buffer + bufferOffset, sizeof(int));
                bufferOffset += sizeof(int);

                memcpy(&type, buffer + bufferOffset, sizeof(int));
                bufferOffset += sizeof(int);

                iterateEvents(ServerEventType::EVENT_KEY, ServerEvent{.keyEvent{code, type}});
            }
            break;
        }
        case ServerEventType::EVENT_FOCUS: {
            // Only process if we are already marked
            if(m_socketClient && m_socketClient->isReceived()) {
                spdlog::info("Focus Message to pass to client: {}", buffer);

                std::size_t windowNameSize;
                memcpy(&windowNameSize, buffer + bufferOffset, sizeof(std::size_t));
                bufferOffset += sizeof(std::size_t);

                char* windowName = new char[windowNameSize];
                memcpy(&windowName[0], buffer + bufferOffset, windowNameSize);
                bufferOffset += windowNameSize;

                bool hasFocus;
                memcpy(&hasFocus, buffer + bufferOffset, sizeof(bool));
                bufferOffset += sizeof(bool);

                iterateEvents(ServerEventType::EVENT_FOCUS, ServerEvent{.focusEvent{windowNameSize, windowName, hasFocus}});
            }
            break;
        }
        default: {
            spdlog::warn("Message type not supported, message: {}", buffer);
            break;
        }
    }
}

void GwidiServerListener::addEventCb(const std::string &identifier, const GwidiServerListener::EventCb &cb) {

    auto action = [identifier, cb, this](){
        auto it = m_eventCbs.find(identifier);
        if(it == m_eventCbs.end()) {
            m_eventCbs[identifier] = cb;
        }
        else {
            // Lock in case we are modifying an existing callback reference
            m_eventCbsLock.lock();
            m_eventCbs[identifier] = cb;
            m_eventCbsLock.unlock();
        }
    };

    if(m_eventCbsLock.owns_lock()) {
        m_deferredActions.emplace_back(action);
    }
    else {
        action();
    }
}

void GwidiServerListener::removeEventCb(const std::string &identifier) {
    auto action = [this, identifier](){
        auto it = m_eventCbs.find(identifier);
        if(it != m_eventCbs.end()) {
            m_eventCbsLock.lock();
            m_eventCbs.erase(it);
            m_eventCbsLock.unlock();
        }
    };

    if(m_eventCbsLock.owns_lock()) {
        m_deferredActions.emplace_back(action);
    }
    else {
        action();
    }
}

void GwidiServerListener::iterateEvents(ServerEventType type, ServerEvent event) {
    // First, deal with any pending actions
    for(auto &act : m_deferredActions) {
        act();
    }
    m_deferredActions.clear();

    m_eventCbsLock.lock();
    for(auto &cb : m_eventCbs) {
        cb.second(type, event);
    }
    m_eventCbsLock.unlock();
}

}
