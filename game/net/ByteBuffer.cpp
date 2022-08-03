
#include "ByteBuffer.h"
#include <cstring>

namespace net
{

	ByteBuffer::ByteBuffer() :
		ByteBuffer(nullptr, 0)
	{
	}

	ByteBuffer::ByteBuffer(PK_byte byteVal) : 
		_type(DataType::BYTE)
	{
		_size = 1;
		_data = new PK_byte[_size];
		memset(_data, 0, _size);
		memcpy(_data, &byteVal, _size);
	}
	ByteBuffer::ByteBuffer(const char* cStr, int32_t size)
	{
		if (cStr && size > 0)
		{
			_type = DataType::STRING;
			_size = size;
			_data = new PK_byte[_size];
			memset(_data, 0, _size);
			memcpy(_data, cStr, _size);
		}
		else
		{
			_type = DataType::EMPTY;
			_size = 0;
			_data = nullptr;
		}
	}
	ByteBuffer::ByteBuffer(int32_t intVal) : 
		_type(DataType::INT32)
	{
		_size = sizeof(int32_t);

		_data = new PK_byte[_size];
		memset(_data, 0, _size);
		memcpy(_data, &intVal, _size);
	}
	
	ByteBuffer::ByteBuffer(const ByteBuffer& other)
	{
		_type = other._type;
		_size = other._size;

		_data = new PK_byte[_size];
		memcpy(_data, other._data, _size);
	}

	ByteBuffer::~ByteBuffer()
	{
		delete[] _data;
	}

	bool ByteBuffer::isEmpty() const
	{
		return _type == DataType::EMPTY;
	}

	std::string ByteBuffer::getString() const
	{
		return std::string(_data, _size);
	}

}