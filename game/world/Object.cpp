
#include "Object.h"


using namespace pk;

namespace world
{
	namespace objects
	{
		VisualObject::VisualObject(pk::Sprite3DRenderable* pSprite) :
			_pSprite(pSprite)
		{}

		VisualObject::VisualObject(const VisualObject& other)
		{
			// Purposefully copying ptrs here and not their content!
			_pSprite = other._pSprite;
		}

		VisualObject::~VisualObject() 
		{}


		// Returns "display direction" of a sprite depending ong the camera's direction
		static int get_display_dir(int objDir, int camDir)
		{
			const int directionCount = 8;
			const int dist = std::abs(objDir - camDir);
			if (objDir >= camDir)
				return dist;
			else
				return directionCount - dist;
		}
		// TODO: Determine which sprite to show, depending on the "tileObject"
		// TODO: Sprite animating
		// TODO: Object speeds and stats
		void VisualObject::show(
			PK_ubyte tileObject, 
			PK_ubyte tileAction, 
			int objDir,
			int camDir,
			const ObjectInfo& staticObjInfo,
			float worldX, 
			float worldY, 
			float worldZ
		)
		{
			_pSprite->setActive(true);
			_pSprite->position.x = worldX;
			_pSprite->position.y = worldY;
			_pSprite->position.z = worldZ;
			_pSprite->texture = (Texture*)staticObjInfo.pTexture;


			// NOTE: JUST TESTING ATM!
			// TODO: ..make it properly
			if (staticObjInfo.rotateableSprite)
			{
				int toDisplayDir = get_display_dir(objDir, camDir);
				vec2 texOffset = _pSprite->textureOffset;
				texOffset.x = 1.0f;
				texOffset.y = (float)toDisplayDir;
				_pSprite->textureOffset = texOffset;
			}
			else
			{
				// This just temp here while testing rotateable sprites..
				_pSprite->textureOffset = vec2(0.0f, 0.0f);
			}

		}

		void VisualObject::hide()
		{
			_pSprite->setActive(false);
		}
	}
}
