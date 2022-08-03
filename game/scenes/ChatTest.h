#pragma once


#include "../../pk/core/Scene.h"

#include "../../pk/ecs/systems/ui/combinedFunctional/Button.h"
#include "../../pk/ecs/systems/ui/combinedFunctional/InputField.h"
#include "../../pk/ecs/systems/ui/Text.h"

class ChatTest : public pk::Scene
{
private:

	pk::ui::InputField* _inputField_message = nullptr;
	pk::ui::Button* _button_send = nullptr;

	const int _maxMessageCount = 16;
	int _lastMsgIndex = 0;
	std::vector<pk::ui::Text*> _messages;

public:

	ChatTest();
	~ChatTest();

	virtual void init();
	virtual void update();


	void addMessage(std::string msg);
	inline const std::string& getPrevMessage() const { return _messages[_lastMsgIndex]->getRenderable()->getStr(); }

	inline void clearInputField() { _inputField_message->clear(); }
	inline std::string getInputFieldContent() const { return _inputField_message->getContent(); }

private:

	void scrollMessageBoard();
};