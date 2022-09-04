
#include "WebSockTest.h"

#include "../net/Client.h"

#include "../net/Client.h"
#include "../net/NetCommon.h"

#include "../world/Tile.h"


#include <iostream>
#include <chrono>

#include <stdio.h>
#include <string>

using namespace pk;
using namespace pk::web;

using namespace ui;
using namespace net;


/*
static EMSCRIPTEN_WEBSOCKET_T ws;
static EMSCRIPTEN_WEBSOCKET_T connSd;

EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData) {
	Debug::log("onopen");
    	EMSCRIPTEN_RESULT result;
	std::string test = "asasdasdasdasd";
	connSd = websocketEvent->socket;
   	result = emscripten_websocket_send_binary(websocketEvent->socket, (void*)test.data(), (uint32_t)test.size());
   	if (result) {
   	    printf("Failed to emscripten_websocket_send_utf8_text(): %d\n", result);
   	}
   	return EM_TRUE;
}

EM_BOOL onerror(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData) {
	Debug::log("onerror");
	return EM_TRUE;
}

EM_BOOL onclose(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData) {
	Debug::log("onclose");
	return EM_TRUE;
}

EM_BOOL onmessage(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData) {
	Debug::log("onmessage");
	std::string message((char*)websocketEvent->data, (size_t)websocketEvent->numBytes);
	Debug::log("Message: " + message + " bytes: " + std::to_string(websocketEvent->numBytes));
	//std::string data(, 256);
	//Debug::log("message: " + data);
	return EM_TRUE;
}*/

WebSockTest::WebSockTest()
{}

WebSockTest::~WebSockTest()
{
	// JUST FOR TESTING: shutdown server when quit
	Client* client = Client::get_instance();
	size_t bufSize = sizeof(int);
	PK_byte* pSendBuf = new PK_byte[bufSize];
	memset(pSendBuf, 0, bufSize);

	const int32_t messageType = MESSAGE_TYPE__ServerShutdown;
	memcpy(pSendBuf, &messageType, sizeof(int32_t));

	client->send(pSendBuf, bufSize);

	delete[] pSendBuf;
}

void WebSockTest::init()
{}

void WebSockTest::update()
{
	int32_t x = 100;
	int32_t z = 100;
	int32_t radius = 15;

	Client* client = Client::get_instance();
	size_t bufSize = sizeof(int) * 4;
	PK_byte* pSendBuf = new PK_byte[bufSize];
	memset(pSendBuf, 0, bufSize);
	const int32_t messageType = MESSAGE_TYPE__UpdateObserverProperties;
	
	memcpy(pSendBuf, &messageType, sizeof(int32_t));
	memcpy(pSendBuf + sizeof(int32_t), (const void*)&x, sizeof(int32_t));
	memcpy(pSendBuf + sizeof(int32_t) * 2, (const void*)&z, sizeof(int32_t));
	memcpy(pSendBuf + sizeof(int32_t) * 3, (const void*)&radius, sizeof(int32_t));
	
	client->send(pSendBuf, bufSize);

	delete[] pSendBuf;
}
