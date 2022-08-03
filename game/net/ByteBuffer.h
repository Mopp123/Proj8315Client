#pragma once

#include "../../pk/Common.h"
#include <string>
#include <cstring>
#include <cstdint>


#define NULL_BYTEBUFFER (net::ByteBuffer(nullptr, 0))


namespace net
{
	
	class ByteBuffer
	{
	private:

		enum DataType
		{
			EMPTY = 0,
			BYTE,
			INT32,
			STRING
		};

		DataType _type;
		PK_byte* _data = nullptr;
		int32_t _size = 0;
		
	public:

		ByteBuffer();
		ByteBuffer(PK_byte byteVal);
		ByteBuffer(const char* cStr, int32_t size);
		ByteBuffer(int32_t intVal);

		ByteBuffer(const ByteBuffer& other);

		~ByteBuffer();

		bool isEmpty() const;

		std::string getString() const;

		inline const PK_byte * const getRawData() const { return _data; }

		template<typename T>
		T getVal() const
		{
			if (_type == STRING)
				return 0;

			T val;
			memcpy(&val, _data, _size);
			return val;
		}

		inline DataType getDataType() const { return _type; }
		inline int32_t getSize() const { return _size; }
		
		
		template<typename T>
		static T get_val(PK_byte* buffer, int pos = 0)
		{
			T val;
			memcpy(&val, buffer + pos, sizeof(T));
			return val;
		}

	};
}