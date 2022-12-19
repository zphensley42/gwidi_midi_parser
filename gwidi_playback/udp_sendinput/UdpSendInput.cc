#include "UdpSendInput.h"
#include <iostream>
#include "GwidiServerClient.h"


SendInput::SendInput() {
}

SendInput::~SendInput() {
}

void SendInput::sendInput(const std::string &key) {
    auto serverListener = gwidi::udpsocket::GwidiServerClientManager::instance().serverListener();
    serverListener->getServerClient()->sendKeyInputEvent(key);
}
