
#include "ChatTest.h"
#include "../net/Client.h"

#include "../net/ByteBuffer.h"
#include "../net/Client.h"
#include "../net/requests/platform/web/WebRequest.h"
#include "../net/NetCommon.h"


using namespace pk;
using namespace ui;
using namespace net;
using namespace net::web;

ChatTest::ChatTest()
{}


ChatTest::~ChatTest()
{
	
	delete _inputField_message;
	delete _button_send;
}


class OnCompletion_QueryMessage : public OnCompletionEvent
{
public:
	ChatTest& chatRef;

	OnCompletion_QueryMessage(ChatTest& chat) : 
		chatRef(chat)
	{}

	virtual void func(const uint64_t* data, size_t dataSize)
	{
		/*
		if (data.size() > 0)
		{
			std::string msg = data[0].getString();
			const std::string prev = chatRef.getPrevMessage();
			if (prev.compare(msg) != 0)
			{
				chatRef.addMessage(msg);

			}
		}*/
	}

};

class OnClick_send : public OnClickEvent
{
public:

	ChatTest& chatRef;

	OnClick_send(ChatTest& chat) : chatRef(chat)
	{}

	virtual void onClick(InputMouseButtonName button)
	{
		std::string messageToSend = chatRef.getInputFieldContent();
		if(messageToSend.size() > 0)
		{
			ByteBuffer buf_mType((PK_byte)2);
			ByteBuffer buf_message(messageToSend.data(), messageToSend.size());
			new WebRequest(Request::ReqType::POST, nullptr, { buf_mType, buf_message }, buf_mType.getSize() + buf_message.getSize());
			chatRef.clearInputField();
		}
	}
};

class OnSubmit_send : public InputFieldOnSubmitEvent
{
public:

	virtual void onSubmit(std::string inputFieldText)
	{
		if (inputFieldText.size() > 0)
		{
			ByteBuffer buf_mType((PK_byte)2);
			ByteBuffer buf_message(inputFieldText.data(), inputFieldText.size());
			new WebRequest(Request::ReqType::POST, nullptr, { buf_mType, buf_message }, buf_mType.getSize() + buf_message.getSize());
		}
	}
};

void ChatTest::init()
{
	const float textSize = 32;
	const float rowPadding = 5;

	const float buttonSize = 24;

	const float panelX = 16;
	const float panelY = 32;

	

	_inputField_message = new InputField(
		" Enter message",
		{
			{ConstraintType::PIXEL_LEFT, panelX},
			{ConstraintType::PIXEL_TOP, panelY}
		},
		300,
		new OnSubmit_send,
		true
	);

	_button_send = new Button(
		"Send",
		{
			{ConstraintType::PIXEL_LEFT, panelX + 256},
			{ConstraintType::PIXEL_TOP, panelY + textSize}
		},
		100,
		buttonSize,
		new OnClick_send(*this)
	);

	for (int i = 0; i < _maxMessageCount; ++i)
	{
		Text* t = new Text(
			"",
			{
				{ConstraintType::PIXEL_LEFT, panelX + 530},
				{ConstraintType::PIXEL_TOP, panelY +  i * textSize}
			}
		);
		
		_messages.push_back(t);
	}
}


static float s_TEST_cooldown_getMessages = 0.0f;
static float s_TEST_maxcooldown_getMessages = 1.0f;

void ChatTest::update()
{
	/*
	if (s_TEST_cooldown_getMessages <= 0.0f)
	{
		ByteBuffer buf_mType((PK_byte)1);
		new WebRequest(Request::ReqType::POST, new OnCompletion_QueryMessage(*this), { buf_mType }, buf_mType.getSize());
		s_TEST_cooldown_getMessages = s_TEST_maxcooldown_getMessages;
	}*/


	s_TEST_cooldown_getMessages -= 1.0f * Timing::get_delta_time();
}

void ChatTest::addMessage(std::string msg)
{
	if(_lastMsgIndex + 1 < _maxMessageCount)
	{
		_lastMsgIndex++;
	}
	else
	{
		scrollMessageBoard();
	}

	_messages[_lastMsgIndex]->accessRenderable()->accessStr() = msg;
}


void ChatTest::scrollMessageBoard()
{
	for (int i = 1; i < _maxMessageCount; ++i)
	{
		std::string content = _messages[i]->getRenderable()->getStr();
		_messages[i - 1]->accessRenderable()->accessStr() = content;
	}
}