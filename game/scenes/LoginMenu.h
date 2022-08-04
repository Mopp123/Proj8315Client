#pragma once


#include "../../PortablePesukarhu/ppk.h"

class LoginMenu : public pk::Scene
{
private:

	pk::ui::InputField* _inputField_username = nullptr;
	pk::ui::InputField* _inputField_password = nullptr;

	pk::ui::Button* _button_login = nullptr;

public:

	LoginMenu();
	~LoginMenu();

	virtual void init();
	virtual void update();

};
