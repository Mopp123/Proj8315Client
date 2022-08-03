#pragma once


#include "../../pk/core/Scene.h"

#include "../../pk/ecs/systems/ui/combinedFunctional/Button.h"
#include "../../pk/ecs/systems/ui/combinedFunctional/InputField.h"

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