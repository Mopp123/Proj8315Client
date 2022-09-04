#pragma once

#include <vector>

#include "../../PortablePesukarhu/ppk.h"

#include "../world/World.h"


class WebSockTest : public pk::Scene
{
private:

public:

	WebSockTest();
	~WebSockTest();

	virtual void init();
	virtual void update();

};
