#include "../PortablePesukarhu/ppk.h"

#include "scenes/MainMenu.h"
#include "scenes/InGame.h"
#include "scenes/InGameLocal.h"

#include "net/Client.h"
#include "net/platform/web/WebClient.h"

#include "../PortablePesukarhu/utils/FileUtils.h"
#include <iostream>

using namespace pk;
using namespace pk::web;
using namespace ui;


int main(int argc, const char** argv)
{

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

    const std::string configPath = "assets/config.conf";
    std::string configStr = load_text_file(configPath);
    if (configStr.empty())
    {
        Debug::log(
            "@main "
            "Invalid config file: " + configPath + " "
            "returned config data was empty",
            Debug::MessageType::PK_FATAL_ERROR
        );
        return 0;
    }
    const std::string fullHost = "ws://" + configStr;
    net::Client* pClient = (net::Client*)(new net::web::WebClient(fullHost));
    application.switchScene((Scene*)(new MainMenu));
    application.run();

    return 0;
}
