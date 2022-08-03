
#include "Request.h"
#include "../Client.h"

namespace net
{
	Request::Request(OnCompletionEvent* onCompletion, PK_byte* reqBody, size_t bodySize) :
		_pOnCompletion(onCompletion), _pReqBody(reqBody)
	{
		_pClient = Client::get_instance();
	}

	Request::Request(ReqType type, OnCompletionEvent* onCompletion, const std::vector<ByteBuffer>& reqBody, size_t bodySize) :
		_type(type), _pOnCompletion(onCompletion)
	{
		_pClient = Client::get_instance();

		// cat all ByteBuffers into a single large body
		_pReqBody = new PK_byte[bodySize];
		size_t ptr = 0;
		for (const ByteBuffer& bb : reqBody)
		{
			size_t bufSize = static_cast<size_t>(bb.getSize());
			memcpy(_pReqBody + ptr, bb.getRawData(), bufSize);
			ptr += bufSize;
		}
	}

	Request::~Request()
	{
		delete[] _pReqBody;
		delete _pOnCompletion;
	}
}