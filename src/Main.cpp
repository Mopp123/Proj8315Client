#include "../PortablePesukarhu/ppk.h"

#include "scenes/MainMenu.h"
#include "scenes/InGame.h"
#include "scenes/InGameLocal.h"

#include "net/Client.h"
#include "net/platform/web/WebClient.h"

#include <iostream>

using namespace pk;
using namespace pk::web;
using namespace ui;


int main(int argc, const char** argv)
{
    std::cout << "TESTING123!\n";

    // NOTE: ISSUES!
    // * need to create window and input manager to heap using
    // some "create func" like the other api/platform agnostic stuff..

    // TODO: platform agnostic window creation
    WebWindow window;
    Context* pGraphicsContext = Context::create(GRAPHICS_API_WEBGL);
    WebInputManager inputManager;

    Application application(
        PK_PLATFORM_ID_WEB,
        "IndexApp Emscripten testing",
        &window,
        pGraphicsContext,
        &inputManager
    );

    //net::Client* pClient = (net::Client*)(new net::web::WebClient("ws://127.0.0.1:51421"));
    application.switchScene((Scene*)(new InGameLocal));
    application.run();

    return 0;
}
