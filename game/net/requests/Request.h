#pragma once

#include "../ByteBuffer.h"
#include <vector>

namespace net
{

	class Client;

	class OnCompletionEvent
	{
	public:

		virtual void func(const PK_byte* data, size_t dataSize) = 0;
		virtual ~OnCompletionEvent() {}
	};

	class Request
	{
	public:
		enum ReqType
		{
			POST,
			GET
		};

	protected:


		Client* _pClient = nullptr;

		OnCompletionEvent* _pOnCompletion = nullptr;
		ReqType _type;

		// body as "raw data"
		PK_byte* _pReqBody;

	public:

		// *NOTE! reqBody's ownership gets transferred to THIS Request
		Request(OnCompletionEvent* onCompletion, PK_byte* reqBody, size_t bodySize);

		Request(ReqType type, OnCompletionEvent* onCompletion, const std::vector<ByteBuffer>& reqBody, size_t bodySize);
		virtual ~Request();

	};
}