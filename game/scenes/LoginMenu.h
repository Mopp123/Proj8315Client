#pragma once

#include "../../PortablePesukarhu/ppk.h"

class LoginMenu : public pk::Scene
{
private:

	pk::ui::InputField* _inputFieldUsername = nullptr;
	pk::ui::InputField* _inputFieldPassword = nullptr;

	pk::ui::Button* _loginButton = nullptr;

public:

	LoginMenu();
	~LoginMenu();

	virtual void init();
	virtual void update();

};
