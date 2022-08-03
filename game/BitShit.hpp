#pragma once

#include <iostream>

/*

   Little helper functions for bit level manipulations thing things...

   Use at your own risk:D

*/

namespace bitshit
{
	// Sets "isSet" is a bit in the "val" at "pos" set, or not
	template<typename T>
	void is_set(const T& val, int pos, bool& isSet)
	{
		isSet = (val >> pos) & 0x1;
	}

	// Sets bit in "target" at "pos" to "set" or "not set"
	template<typename T>
	void set_bit(T& target, int pos, bool bit)
	{
		T mask = (T)1 << pos;
		T bitT = (T)bit;
		target = ((target & ~mask) | bitT << pos);
	}


	// Sets area in "target" to match bits in "input" starting from "startPos"
	// *Currently bit count has to match that "input's" bitcount.. (has to be <= to target's bitcount)
	template<typename A, typename B>
	void set_area(A& target, B input, int startPos, int bitCount)
	{
		for (int j = 0; j < bitCount; ++j)
		{
			int i = j + startPos;

			bool bit = false;
			is_set<B>(input, j, bit);

			A bitA = (A)bit;
			A maskA = (A)1 << (A)i;
			target = ((target & ~maskA) | bitA << i);
		}
	}


	// Converts area of input value into "output" starting from "pos" by amount of "bitcount"
	template<typename A, typename B>
	void convert_area(A input, B& output, int pos, int bitCount)
	{
		for (int i = 0; i < bitCount; ++i)
		{
			int j = pos + i;

			// Is bit at pos "i" set in "input"?
			A bit = (input >> j) & 0x1;
			A mask = (A)1 << i;

			output = ((output & ~mask) | bit << i);
		}
	}


	// Prints bits in input
	template<typename T>
	void print_bits(T input, int bitCount)
	{
		for (int i = bitCount - 1; i >= 0; --i)
		{
			bool bit = false;
			bitshit::is_set<T>(input, i, bit);
			std::cout << std::to_string((char)bit);
		}
		std::cout << "\n";
	}


}