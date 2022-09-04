#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "../../PortablePesukarhu/ppk.h"


namespace net
{
	class OnMessageEvent
	{
	public:
		OnMessageEvent() {}
		virtual ~OnMessageEvent() {}
		virtual void onMessage(const PK_byte* data, size_t dataSize) = 0;
	};

	class Client
	{
	protected:
		// actually its the complete url to server...
		std::string _hostname;
		std::string _userID;

		static Client* s_pInstance;

		std::unordered_map<int32_t, OnMessageEvent*> _onMessageEvents;

	public:
		Client(const std::string& hostname) : _hostname(hostname) {};
		virtual ~Client() {};
		Client(const Client& other) = delete;

		virtual void send(PK_byte* data, size_t dataSize) {};
		void addOnMessageEvent(int32_t messageType, OnMessageEvent* event);
		void clearOnMessageEvents();

		inline void setUserID(const std::string& id) { _userID = id; }

		inline const std::string& getHostname() const { return _hostname; }
		inline const std::string& getUserID() const { return _userID; }

		static Client* get_instance();
	};
}
