#include "WebClient.h"
#include <string>

#include "../../../../PortablePesukarhu/ppk.h"
#include "../../NetCommon.h"

using namespace pk;

namespace net
{
	namespace web
	{
		EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData) {
		    	WebClient* client = (WebClient*)(Client::get_instance());
			client->_connSD = websocketEvent->socket;
			Debug::log("Websocket connection established");
			client->_connected = true;
		   	return EM_TRUE;
		}
		

		EM_BOOL onerror(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData) {
			Debug::log("Emscripten websocket error!", Debug::MessageType::PK_ERROR);
			return EM_TRUE;
		}

		
		EM_BOOL onclose(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData) {
			Debug::log("Emscripten websocket closed");
			return EM_TRUE;
		
		}


		EM_BOOL onmessage(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData) {
			//std::string message((char*)websocketEvent->data, (size_t)websocketEvent->numBytes);
			//Debug::log("Message: " + message + " bytes: " + std::to_string(websocketEvent->numBytes));
			
			PK_byte* messageData = (PK_byte*)websocketEvent->data;
			const size_t messageSize = (size_t)websocketEvent->numBytes;

		    WebClient* client = (WebClient*)(Client::get_instance());
			if (messageSize >= MESSAGE_MIN_DATA_SIZE)
			{
				// Attempt to parse "header"(first 32 bits) to find the "MessageType"
				int32_t messageType = -1;
				memcpy(&messageType, (const void*)messageData, sizeof(int32_t));
				//Debug::log("___TEST___Received msg of size: " + std::to_string(messageSize) + " parsed type: " + std::to_string(messageType));
				auto event = client->_onMessageEvents.find(messageType);
				if (event != client->_onMessageEvents.end())
					(*event).second->onMessage(messageData + sizeof(int32_t), messageSize - sizeof(int32_t));
			
			//for (std::pair<int32_t, OnMessageEvent*>& event : client->_onMessageEvents)
			//	event->onMessage((PK_byte*)websocketEvent->data(), (size_t)websocketEvent->numBytes);
			}
			return EM_TRUE;
		}


		WebClient::WebClient(const std::string& hostname) :
			Client(hostname)
		{
			if (!emscripten_websocket_is_supported()) {
				Debug::log("Websocket was not supported!", Debug::MessageType::PK_FATAL_ERROR);
			}
			// "ws://127.0.0.1:51421",
			EmscriptenWebSocketCreateAttributes ws_attrs = {
				_hostname.c_str(),
				NULL,
				EM_FALSE
			};
			
			_ws = emscripten_websocket_new(&ws_attrs);
			if (_ws == 0)
				Debug::log("Websocket was not supporte", Debug::MessageType::PK_ERROR);
			else if (_ws < 0)
				Debug::log("Failed to connect to: " + _hostname, Debug::MessageType::PK_ERROR);

			emscripten_websocket_set_onopen_callback(_ws, NULL, onopen);
			
			emscripten_websocket_set_onerror_callback(_ws, NULL, onerror);
			emscripten_websocket_set_onclose_callback(_ws, NULL, onclose);
			emscripten_websocket_set_onmessage_callback(_ws, NULL, onmessage);

			Client::s_pInstance = this;
		}

		WebClient::~WebClient()
		{
			EMSCRIPTEN_RESULT closeConnResult = emscripten_websocket_close(_connSD, 1000, "no reason");
			EMSCRIPTEN_RESULT closeWsResult = emscripten_websocket_close(_ws, 1000, "no reason");
			
			if (closeConnResult < 0)
				Debug::log("Failed to successfully close connection to the host");
			if (closeWsResult < 0)
				Debug::log("Failed to successfully close websocket");

			if (closeConnResult >= 0 && closeWsResult > 0)
				Debug::log("Websockets were closed successfully");
		}

		int WebClient::send(PK_byte* data, size_t dataSize)
		{
			if (!_connected)
			{
				//Debug::log("___WEBSOCKET_ERROR___");
				return 0;
			}

   			EMSCRIPTEN_RESULT result = emscripten_websocket_send_binary(_connSD, (void*)data, (uint32_t)dataSize);
			if (result < 0)
			{
				Debug::log("Failed WebClient::send(byte*, size_t) EMSCRIPTEN_ERROR = " + std::to_string(result));
				return 0;
			}
			else
			{
				return 1;
			}
		}
	}
}
