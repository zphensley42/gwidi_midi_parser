#include <spdlog/spdlog.h>
#include <linux/input-event-codes.h>
#include "GwidiServerClient.h"

int main() {
    gwidi::udpsocket::GwidiServerListener listener;
    listener.start();

    bool doSendWatchedKeysTest = true;
    listener.setEventCb([&](gwidi::udpsocket::ServerEventType type, gwidi::udpsocket::ServerEvent event) {
        spdlog::info("Event received, type: {}", type);
        if(type == gwidi::udpsocket::ServerEventType::EVENT_KEY) {
            spdlog::info("Key Event: {}, {}", event.keyEvent.code, event.keyEvent.eventType);

            if(doSendWatchedKeysTest) {
                // For testing, send here -- need to instead build a queue of messages to ensure this reconfig makes it through?
                listener.sendWatchedKeysReconfig({KEY_Q, KEY_W});
                doSendWatchedKeysTest = false;
            }
        }
        else if(type == gwidi::udpsocket::ServerEventType::EVENT_FOCUS) {
            spdlog::info("Focus Event: {}, {}", event.focusEvent.windowName, event.focusEvent.hasFocus);
        }
    });

    while(listener.isAlive()) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    return 0;
}