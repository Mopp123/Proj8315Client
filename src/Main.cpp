#include <iostream>

#include "net/platform/web/WebClient.h"


int main(int argc, const char** argv)
{
    std::cout << "Working!!\n";

    net::web::WebClient* client = new net::web::WebClient("ws://127.0.0.1:51421");

    return 0;
}
