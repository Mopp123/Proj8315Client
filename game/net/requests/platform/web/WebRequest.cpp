
#include "WebRequest.h"
#include "../../../Client.h"
#include "../../../../../pk/core/Debug.h"

#include <iostream>

using namespace pk;

namespace net
{
	namespace web
	{


		void download_succeeded(emscripten_fetch_t* fetch)
		{
			// The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
			
			WebRequest* req = (WebRequest*)fetch->userData;

			if (req->_pOnCompletion)
				req->_pOnCompletion->func(fetch->data, fetch->numBytes);

			emscripten_fetch_close(fetch); // Free data associated with the fetch.

			PK_COMMIT_SUICIDE(req);
		}

		void download_failed(emscripten_fetch_t* fetch) 
		{
			Debug::log("Request failed! Status code was: " + std::to_string(fetch->status), Debug::MessageType::PK_ERROR);

			printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
			emscripten_fetch_close(fetch); // Also free data on failure.

			WebRequest* req = (WebRequest*)fetch->userData;
			PK_COMMIT_SUICIDE(req);
		}

		WebRequest::WebRequest(OnCompletionEvent* onCompletion, PK_byte* reqBody, size_t bodySize) :
			Request(onCompletion, reqBody, bodySize)
		{
			emscripten_fetch_attr_init(&_fetchAttribs);

			_fetchAttribs.userData = this;
			strcpy(_fetchAttribs.requestMethod, "POST");

			_fetchAttribs.requestDataSize = bodySize;
			_fetchAttribs.requestData = _pReqBody;

			_fetchAttribs.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
			_fetchAttribs.onsuccess = download_succeeded;
			_fetchAttribs.onerror = download_failed;
			
			emscripten_fetch(&_fetchAttribs, Client::get_instance()->getHostname().c_str());
		}



		WebRequest::WebRequest(ReqType reqType, OnCompletionEvent* onCompletion, const std::vector<ByteBuffer>& reqBody, size_t bodySize) :
			Request(reqType, onCompletion, reqBody, bodySize)
		{
			
			emscripten_fetch_attr_init(&_fetchAttribs);

			_fetchAttribs.userData = this;

			if(_type == ReqType::GET)
				strcpy(_fetchAttribs.requestMethod, "GET");
			else if (_type == ReqType::POST)
				strcpy(_fetchAttribs.requestMethod, "POST");


			_fetchAttribs.requestDataSize = bodySize;
			_fetchAttribs.requestData = _pReqBody;

			_fetchAttribs.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
			_fetchAttribs.onsuccess =	download_succeeded;
			_fetchAttribs.onerror =		download_failed;
			
			emscripten_fetch(&_fetchAttribs, Client::get_instance()->getHostname().c_str());
		}

		WebRequest::~WebRequest()
		{
			Debug::log("Req was completed. Committing suicide...");
		}
	}
}