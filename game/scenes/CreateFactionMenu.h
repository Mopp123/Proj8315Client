#pragma once


#include "../../pk/core/Scene.h"

#include "../../pk/ecs/systems/ui/combinedFunctional/Button.h"
#include "../../pk/ecs/systems/ui/combinedFunctional/InputField.h"

class CreateFactionMenu : public pk::Scene
{
private:

	pk::ui::InputField* _inputField_username = nullptr;
	pk::ui::InputField* _inputField_password = nullptr;
	pk::ui::InputField* _inputField_passwordRepeat = nullptr;

	pk::ui::Button* _button_create = nullptr;
	pk::ui::Button* _button_cancel = nullptr;

public:

	CreateFactionMenu();
	~CreateFactionMenu();

	virtual void init();
	virtual void update();

};