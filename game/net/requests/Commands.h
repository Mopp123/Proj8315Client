#pragma once

#include <string>
#include "platform/web/WebRequest.h"

namespace net
{

	void send_command(const std::string& userID, int32_t function, OnCompletionEvent* = nullptr);
	void send_command(const std::string& userID, int32_t function, const std::string& arg, OnCompletionEvent* = nullptr);
	
	void send_command(const std::string& userID, int32_t function, int32_t, int32_t, int32_t, OnCompletionEvent* = nullptr);
	

}