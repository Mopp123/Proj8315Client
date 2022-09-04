#pragma once

#include <emscripten/websocket.h>

#include "../../Client.h"


namespace net
{
	namespace web
	{
		class WebClient : public Client
		{
		private:
			EMSCRIPTEN_WEBSOCKET_T _ws;
			EMSCRIPTEN_WEBSOCKET_T _connSD;

		public:
			WebClient(const std::string& hostname);
			~WebClient();

			virtual void send(PK_byte* data, size_t dataSize) override;

		private:
			friend EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData);
			friend EM_BOOL onerror(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData);
			friend EM_BOOL onclose(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData);
			friend EM_BOOL onmessage(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData);
		};
	}
}
