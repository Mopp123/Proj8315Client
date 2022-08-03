
#include "Commands.h"
#include "../NetCommon.h"


namespace net
{
	using namespace web;

	static PK_byte* alloc_command(const std::string& userID, int32_t function, size_t cmdSize)
	{
		PK_byte* data = new PK_byte[cmdSize];
		// create header
		memcpy(data, userID.data(), userID.size());
		memcpy(data + USER_ID_LEN, &function, sizeof(int32_t));

		return data;
	}

	static void add_to_command(PK_byte* cmdBuf, const PK_byte* toAdd, size_t toAddSize, int* ptr)
	{
		memcpy(cmdBuf + (*ptr), toAdd, toAddSize);
		*ptr += toAddSize;
	}




	void send_command(const std::string& userID, int32_t function, OnCompletionEvent* onCompletion)
	{
		size_t totalCmdSize = CMD_MIN_LEN;
		PK_byte* cmdData = alloc_command(userID, function, totalCmdSize);
		new WebRequest(onCompletion, cmdData, totalCmdSize);
	}

	void send_command(const std::string& userID, int32_t function, const std::string& arg, OnCompletionEvent* onCompletion)
	{
		size_t totalCmdSize = CMD_MIN_LEN + arg.size();
		int ptr = CMD_MIN_LEN;
		PK_byte* cmdData = alloc_command(userID, function, totalCmdSize);
		add_to_command(cmdData, arg.data(), arg.size(), &ptr);
		new WebRequest(onCompletion, cmdData, totalCmdSize);
	}

	void send_command(const std::string& userID, int32_t function, int32_t arg1, int32_t arg2, int32_t arg3, OnCompletionEvent* onCompletion)
	{
		size_t totalCmdSize = CMD_MIN_LEN + sizeof(int32_t) * 3;
		int ptr = CMD_MIN_LEN;
		PK_byte* cmdData = alloc_command(userID, function, totalCmdSize);
		add_to_command(cmdData, (const PK_byte*)(&arg1), sizeof(int32_t), &ptr);
		add_to_command(cmdData, (const PK_byte*)(&arg2), sizeof(int32_t), &ptr);
		add_to_command(cmdData, (const PK_byte*)(&arg3), sizeof(int32_t), &ptr);
		new WebRequest(onCompletion, cmdData, totalCmdSize);
	}
}