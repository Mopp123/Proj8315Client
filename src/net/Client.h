#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

#include "../../PortablePesukarhu/ppk.h"
#include "../../Proj8315Common/src/Common.h"


namespace net
{
    typedef struct MsgDataPart
    {
        PK_byte* data = nullptr;
        size_t usedSize = 0;
        size_t maxSize = 0;
    } MsgDataPart;


    class OnMessageEvent
    {
    public:
        OnMessageEvent() {}
        virtual ~OnMessageEvent() {}
        virtual void onMessage(const GC_byte* data, size_t dataSize) = 0;
    };


    class Client
    {
    protected:
        // The complete url to server...
        std::string _hostname;

        static Client* s_pInstance;
        bool _connected = false;

        std::unordered_map<int32_t, OnMessageEvent*> _onMessageEvents;

    public:
        gamecommon::User user;

        Client(const std::string& hostname) : _hostname(hostname) {};
        virtual ~Client() {};
        Client(const Client& other) = delete;

        // Returns status code
        // 	0 = fail
        // 	1 = success
        virtual int send_raw(
            GC_byte* data,
            size_t dataSize
        )
        {
            return 0;
        }

        virtual int send(
            int32_t messageType,
            std::vector<MsgDataPart> data)
        {
            return 0;
        }

        void addOnMessageEvent(int32_t messageType, OnMessageEvent* event);
        void clearOnMessageEvents();

        inline const std::string& getHostname() const { return _hostname; }
        // *Safe to use even if the instance is nullptr -> if no instance obsiously not connected
        bool isConnected() const;

        static Client* get_instance();
    };
}
