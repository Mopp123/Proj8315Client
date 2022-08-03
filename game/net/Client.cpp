
#include "Client.h"


namespace net
{
	Client* Client::s_pInstance = nullptr;

	void Client::init(const std::string& host)
	{
		_hostname = host;
	}

	Client::Client()
	{}

	Client::~Client()
	{}


	void Client::sendRequest(Request* req)
	{

	}

	Client* Client::get_instance()
	{
		if (!s_pInstance)
			s_pInstance = new Client();
		
		return s_pInstance;
	}

}