
#include "CreateFactionMenu.h"

#include "../net/Client.h"

#include "../net/Client.h"
#include "../net/requests/Commands.h"
#include "../net/NetCommon.h"

#include "../../pk/core/Debug.h"
#include <iostream>

using namespace pk;
using namespace ui;
using namespace net;
using namespace net::web;


CreateFactionMenu::CreateFactionMenu()
{}


CreateFactionMenu::~CreateFactionMenu()
{
	delete _inputField_username;
	delete _inputField_password;
	delete _button_create;
	delete _button_cancel;
}


class OnCompletion_test : public OnCompletionEvent
{
public:

	virtual void func(const uint64_t* data, size_t dataSize)
	{
		Debug::log("Req completed! A");
		/*
		if (data.size() > 0)
		{
			std::string response(data[0].getRawData(), data[0].getSize());
			Debug::log("Server response: " + response);
		}*/
	}

};


static std::string s_TEST_worldstate;
static Text* s_TEST_text = nullptr;

/*
class OnCompletion_fetchWorldState : public OnCompletionEvent
{
public:

	virtual void func(const std::vector<ByteBuffer>& data)
	{
		Debug::log("Req completed! B");

		const int dataWidth = (5 * 2) + 1;
		const size_t expectedDataSize = (dataWidth * dataWidth) * sizeof(uint64_t);
		
		Debug::log("size was: " + std::to_string(data[0].getSize()) + " Expected: " + std::to_string(expectedDataSize));

		// Attempt to print fetched area
		if (data.size() > 0)
		{
			if (data[0].getSize() >= expectedDataSize)
			{
				const uint64_t* dataBuf = (const uint64_t*)data[0].getRawData();
				
				int x = 0;
				for (size_t i = 0; i < dataWidth * dataWidth; ++i)
				{
					uint32_t tileStateUID = 0;
					memcpy((void*)(&tileStateUID), (void*)(dataBuf + i), sizeof(uint32_t));

					uint32_t s = tileStateUID > 0 ? 1 : 0;
					s_TEST_worldstate += std::to_string(s) + " ";
					x++;
					if (x >= dataWidth)
					{
						s_TEST_worldstate += "\n";
						x = 0;
					}
				}
			}
			//std::string response(data[0].getRawData(), data[0].getSize());
			//Debug::log("Server response: " + response);
		}
	}
};


static int s_TEST_xPos = 0;
static int s_TEST_zPos = 0;


class KeyEvent_move : public KeyEvent
{
public:

	virtual void func(InputKeyName key, int scancode, InputAction action, int mods)
	{
		if (action != InputAction::PK_INPUT_RELEASE)
		{
			s_TEST_worldstate.clear();

			if (key == PK_INPUT_KEY_W)
				s_TEST_zPos -= 1;
			else if (key == PK_INPUT_KEY_S)
				s_TEST_zPos += 1;

			if (key == PK_INPUT_KEY_A)
				s_TEST_xPos -= 1;
			else if (key == PK_INPUT_KEY_D)
				s_TEST_xPos += 1;


			std::string userID = "Persekorva123";
			// Fetch world pos at these coords
			send_command(userID, CMD_FetchWorldState, s_TEST_xPos, s_TEST_zPos, new OnCompletion_fetchWorldState);

		}
	}
};*/

void CreateFactionMenu::init()
{
	std::string userID = "Persekorva123";
	
	// Test loading world state
	//send_command(userID, CMD_FetchWorldState, 0, 0, new OnCompletion_fetchWorldState);

	//send_command(userID, CMD_FetchServerMessage, new OnCompletion_test);
	//send_command(userID, CMD_CreateFaction, "AmazingPpl", new OnCompletion_test);
	

	const float textSize = 16;
	const float rowPadding = 5;

	const float buttonSize = 24;

	const float panelX = 128;
	const float panelY = 128;

	_inputField_username = new InputField(
		" Enter username",
		{
			{ConstraintType::PIXEL_LEFT, panelX},
			{ConstraintType::PIXEL_TOP, panelY}
		},
		200,
		nullptr
	);

	_inputField_password = new InputField(
		" Enter password",
		{
			{ConstraintType::PIXEL_LEFT, panelX},
			{ConstraintType::PIXEL_TOP, panelY + textSize + rowPadding}
		},
		200,
		nullptr
	);

	_inputField_passwordRepeat = new InputField(
		" Repeat password",
		{
			{ConstraintType::PIXEL_LEFT, panelX},
			{ConstraintType::PIXEL_TOP, panelY + (textSize + rowPadding) * 2}
		},
		200,
		nullptr
	);

	_button_create = new Button(
		"Create user",
		{
			{ConstraintType::PIXEL_LEFT, panelX + 110},
			{ConstraintType::PIXEL_TOP, panelY + (textSize + rowPadding) * 3}
		},
		120,
		buttonSize,
		nullptr
	);

	_button_cancel = new Button(
		"Cancel",
		{
			{ConstraintType::PIXEL_LEFT, panelX},
			{ConstraintType::PIXEL_TOP, panelY + (textSize + rowPadding) * 3}
		},
		120,
		buttonSize,
		nullptr
	);



	//Application::get()->accessInputManager()->addKeyEvent(new KeyEvent_move);

	s_TEST_text = new Text(
		"",
		{
			{ConstraintType::PIXEL_LEFT, panelX},
			{ConstraintType::PIXEL_TOP, panelY + (textSize + rowPadding) * 4}
		}
	);

}

void CreateFactionMenu::update()
{

	s_TEST_text->accessRenderable()->accessStr() = s_TEST_worldstate;
}