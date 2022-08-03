#pragma once

#include <emscripten/fetch.h>
#include "../../Request.h"


/*
* 
emscripten_fetch_attr_t attr;

	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "GET");

	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

	attr.onsuccess = downloadSucceeded;
	attr.onerror = downloadFailed;

	emscripten_fetch(&attr, "http://192.168.160.249:51421");

*/

namespace net
{
	namespace web
	{

		class WebRequest : public Request
		{
		private:

			emscripten_fetch_attr_t _fetchAttribs;


		public:

			WebRequest(OnCompletionEvent* onCompletion, PK_byte* reqBody, size_t bodySize);
			WebRequest(ReqType reqType, OnCompletionEvent* onCompletion, const std::vector<ByteBuffer>& reqBody, size_t bodySize);
			~WebRequest();
		
		private:

			friend void download_succeeded(emscripten_fetch_t* fetch);
			friend void download_failed(emscripten_fetch_t* fetch);
		};
	}
}