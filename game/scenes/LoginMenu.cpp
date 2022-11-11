#include "LoginMenu.h"
#include "../net/Client.h"
#include "../net/NetCommon.h"


using namespace pk;
using namespace ui;
using namespace net;


class OnMessageLoginRequest : public net::OnMessageEvent
{
public:
	LoginMenu& sceneRef;

	OnMessageLoginRequest(LoginMenu& scene) : 
		sceneRef(scene)
	{}

	virtual void onMessage(const PK_byte* data, size_t dataSize)
    {
		Debug::log("___TEST___RECV ON LOGIN: packet size = " + std::to_string(dataSize));
        const size_t expectedDataSize = sizeof(int32_t) + 1;
        if (dataSize >= expectedDataSize)
        {
            Client* client = Client::get_instance();
			bool loginStatus = *((bool*)data);
			Debug::log("___TEST___login status: " + std::to_string(loginStatus));
        }
    }
};


class OnClickLogin : public OnClickEvent
{
private:
	InputField* _pUsername = nullptr;
	InputField* _pPassword = nullptr;

public:
	OnClickLogin(InputField* pUsername, InputField* pPasswd)
	{
		_pUsername = pUsername;
		_pPassword = pPasswd;
	}

	virtual void onClick(InputMouseButtonName button)
	{
		Debug::log("___TEST___ONCLICK LOGIN");
		if (button == InputMouseButtonName::PK_INPUT_MOUSE_LEFT)
		{
			Debug::log("___TEST___ONCLICK LOGIN CORRECT BUTTON!");
			const std::string username = _pUsername->getContent();
			const std::string passwd = _pUsername->getContent();
			const size_t usernameLen = username.length();
			const size_t passwdLen = passwd.length();
			
			if (usernameLen <= 0 || usernameLen > USER_NAME_LEN)
			{
				Debug::log("Invalid username length", Debug::MessageType::PK_ERROR);
				return;
			}
			if (passwdLen <= 0 || passwdLen > USER_NAME_LEN)
			{
				Debug::log("Invalid password length", Debug::MessageType::PK_ERROR);
				return;
			}
			Debug::log("___TEST___CREATING SENDBUF...");

			size_t bufSize = sizeof(int32_t) + USER_NAME_LEN + USER_PASSWD_LEN;
			PK_byte* pSendBuf = new PK_byte[bufSize];
			memset(pSendBuf, 0, bufSize);
			
			const int32_t messageType = MESSAGE_TYPE__Login;
			memcpy(pSendBuf, &messageType, sizeof(int32_t));
			memcpy(pSendBuf + sizeof(int32_t), username.c_str(), usernameLen);
			memcpy(pSendBuf + sizeof(int32_t) + USER_NAME_LEN, passwd.c_str(), passwdLen);

		   	Client* client = Client::get_instance();
			client->send(pSendBuf, bufSize);

			delete[] pSendBuf;
		}
	}
};


LoginMenu::LoginMenu()
{}


LoginMenu::~LoginMenu()
{
	delete _inputFieldUsername;
	delete _inputFieldPassword;
	delete _loginButton;
}

void LoginMenu::init()
{
	activeCamera = create_camera({0, 0, 0});

	const float textSize = 32;
	const float rowPadding = 5;

	const float buttonSize = 24;

	const float panelX = 128;
	const float panelY = 128;

	const float panelOffsetX = -200;
	const float panelOffsetY = 32;

	_inputFieldUsername = new InputField(
		"Username",
		{
			{ConstraintType::PIXEL_CENTER_HORIZONTAL, panelOffsetX},
			{ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY}
		},
		200,
		nullptr
	);

	_inputFieldPassword = new InputField(
		"Password",
		{
			{ConstraintType::PIXEL_CENTER_HORIZONTAL, panelOffsetX},
			{ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY - textSize}
		},
		200,
		nullptr
	);

	_loginButton = new Button(
		"Login",
		{
			{ConstraintType::PIXEL_CENTER_HORIZONTAL, -panelOffsetX},
			{ConstraintType::PIXEL_CENTER_VERTICAL, panelOffsetY - textSize * 2.0f}
		},
		100,
		buttonSize,
		new OnClickLogin(_inputFieldUsername, _inputFieldPassword)
	);
    Client* client = Client::get_instance();
	client->addOnMessageEvent(MESSAGE_TYPE__Login, (OnMessageEvent*)(new OnMessageLoginRequest(*this)));
}

void LoginMenu::update()
{
}
