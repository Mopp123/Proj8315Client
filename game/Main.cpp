

#include "../PortablePesukarhu/ppk.h"

#include "scenes/LoginMenu.h"
#include "scenes/CreateFactionMenu.h"
#include "scenes/InGame.h"

#include "net/ByteBuffer.h"
#include "net/Client.h"
#include "net/requests/platform/web/WebRequest.h"
#include "net/NetCommon.h"

#include <unordered_set>

using namespace pk;
using namespace pk::web;
using namespace ui;
using namespace net::web;
using namespace net;


int main(int argc, const char** argv)
{

	std::unordered_set<int> test;

	for (auto w : test)
	{
	}

	bool initSuccess = true;

	WebWindow window;
	WebContext graphicsContext;
	WebInputManager inputManager;

	WebMasterRenderer masterRenderer;
	Renderer* pTerrainRenderer =	(Renderer*)(new WebTerrainRenderer);
	Renderer* pSpriteRenderer =		(Renderer*)(new WebSpriteRenderer);
	Renderer* pGuiRenderer =		(Renderer*)(new WebGUIRenderer);
	Renderer* pFontRenderer =		(Renderer*)(new WebFontRenderer);

	Application application(
		"Emscripten testing..", 
		&window, &graphicsContext, &inputManager, 
		&masterRenderer, 
		{ 
			{ ComponentType::PK_RENDERABLE_TERRAINTILE,	pTerrainRenderer },
			{ ComponentType::PK_RENDERABLE_SPRITE3D,	pSpriteRenderer	 },
			{ ComponentType::PK_RENDERABLE_GUI,			pGuiRenderer	 },
			{ ComponentType::PK_RENDERABLE_TEXT,		pFontRenderer	 }
		});
	
	Client::get_instance()->init("http://192.168.238.230:51421");

	application.switchScene(new InGame);

	// TESTING NET

	
	/*PK_byte messageType = 1;
	ByteBuffer bbHeader(messageType);

	std::string message = "Hello server!";
	ByteBuffer bbMessage(message.data(), message.size());
	
	WebRequest* req = new WebRequest(Request::ReqType::POST, new CustomOnCompletionEvent, { bbHeader, bbMessage }, bbHeader.getSize() + bbMessage.getSize());
	*/

	application.run();

	// set "main loop"
	//emscripten_set_main_loop(update, 0, 0);

	return 0;
}

// Testing emsc networking capabilities...
/*
#include "../pk/core/Debug.h"
#include "net/ByteBuffer.h"
#include "net/Client.h"
#include "net/requests/platform/web/WebRequest.h"

#include <stdio.h>
#include <string.h>
#include <emscripten.h>

#include <iostream>

using namespace pk;
using namespace net;
using namespace web;


void downloadSucceeded(emscripten_fetch_t* fetch) {
	//printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
	
	char* data = new char[fetch->numBytes];
	memcpy(data, fetch->data, fetch->numBytes);

	std::cout << "fetched data: " << data << std::endl;
	delete[] data;
	//printf("Data was: %s", data);
	// The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
	emscripten_fetch_close(fetch); // Free data associated with the fetch.

}

void downloadFailed(emscripten_fetch_t* fetch) {
	printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
	emscripten_fetch_close(fetch); // Also free data on failure.
}*/

/*
void loop()
{
	
}

class CustomOnCompletion : public OnCompletionEvent
{
public:

	virtual void func(const std::vector<ByteBuffer>& data)
	{
		std::string dat_str = data[0].getString();
		Debug::log("Custom onCompletion: data was: " + dat_str);

		std::string message = "Well hello hello!!";
		ByteBuffer body(message.data(), message.size());
		WebRequest* req = new WebRequest(Request::ReqType::POST, nullptr, body);
	}
};

int main() 
{

	Client* c = Client::get_instance();
	c->init("http://192.168.160.249:51421");

	std::string message = "Testing";
	ByteBuffer body(message.data(), message.size());
	WebRequest* req = new WebRequest(Request::ReqType::POST, new CustomOnCompletion, body);

	emscripten_set_main_loop(loop, 0, 1);

	return 0;
}*/
