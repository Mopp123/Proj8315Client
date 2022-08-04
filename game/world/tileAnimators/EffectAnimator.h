#pragma once

#include "../../../PortablePesukarhu/ppk.h"

#include <unordered_map>

namespace world
{

	class EffectAnimator : public pk::Updateable
	{
	private:

		std::unordered_map<PK_ubyte, pk::SpriteAnimator*> _animMapping;

	public:
	};
}
