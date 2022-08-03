#pragma once

#include "../../../pk/ecs/systems/System.h"
#include "../../../pk/ecs/components/renderable/Sprite3DRenderable.h"
#include "../../../pk/ecs/systems/animations/SpriteAnimations.h"
#include "../../../pk/Common.h"

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