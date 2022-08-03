
#include "LoginMenu.h"

using namespace pk;
using namespace ui;


LoginMenu::LoginMenu()
{}


LoginMenu::~LoginMenu()
{
	delete _inputField_username;
	delete _inputField_password;
	delete _button_login;
}



void LoginMenu::init()
{
	const float textSize = 32;
	const float rowPadding = 5;

	const float buttonSize = 24;

	const float panelX = 128;
	const float panelY = 128;

	_inputField_username = new InputField(
		"Username",
		{
			{ConstraintType::PIXEL_LEFT, panelX},
			{ConstraintType::PIXEL_TOP, panelY}
		},
		200,
		nullptr
	);

	_inputField_password = new InputField(
		"Password",
		{
			{ConstraintType::PIXEL_LEFT, panelX},
			{ConstraintType::PIXEL_TOP, panelY + textSize}
		},
		200,
		nullptr
	);

	_button_login = new Button(
		"Login",
		{
			{ConstraintType::PIXEL_LEFT, panelX + 105},
			{ConstraintType::PIXEL_TOP, panelY + textSize * 2}
		},
		100,
		buttonSize,
		nullptr
	);
}

void LoginMenu::update()
{
}