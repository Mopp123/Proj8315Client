#include "Client.h"


namespace net
{
    Client* Client::s_pInstance = nullptr;

    void Client::addOnMessageEvent(int32_t messageType, OnMessageEvent* event)
    {
        if (_onMessageEvents.find(messageType) == _onMessageEvents.end())
            _onMessageEvents.insert({ messageType, event });
        else
            pk::Debug::log(
                "Attempted to add OnMessageEvent but event with same type already exists", 
                pk::Debug::MessageType::PK_FATAL_ERROR
            );
    }

    void Client::clearOnMessageEvents()
    {
        for (auto event : _onMessageEvents)
            delete event.second;
        _onMessageEvents.clear();

        pk::Debug::log("OnMessageEvents cleared");
    }

    Client* Client::get_instance()
    {
        if (!s_pInstance)
            pk::Debug::log("Attempted to access client instance, but the client didn't exist");
        else
            return s_pInstance;
    }
}
