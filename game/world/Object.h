#pragma once

#include <vector>
#include <unordered_map>

#include "../../PortablePesukarhu/ppk.h"


#define OBJECT_DATA_MAX_STRLEN 32


namespace world
{
	namespace objects
	{
		struct ObjectInfo
                {
			// misc. stuff
			char name[OBJECT_DATA_MAX_STRLEN];
			char description[OBJECT_DATA_MAX_STRLEN];
			
			// stats
			PK_ubyte speed;

			// Client exclusive properties:
			pk::web::WebTexture* pTexture = nullptr; // *NOTE! ObjectInfo doesn't own its' texture
			bool rotateableSprite = false;

			ObjectInfo(
			        const char* objName,
			        const char* objDescription,
			        PK_ubyte speedVal,
				pk::web::WebTexture* pSpriteTexture,
				bool visuallyRotateable
			):
			        speed(speedVal),
			        pTexture(pSpriteTexture),
				rotateableSprite(visuallyRotateable)
			{
			        memcpy(name, objName, OBJECT_DATA_MAX_STRLEN);
			        memcpy(description, objDescription, OBJECT_DATA_MAX_STRLEN);
			}
			
			ObjectInfo(const ObjectInfo& other):
			        speed(other.speed),
			        pTexture(other.pTexture),
				rotateableSprite(other.rotateableSprite)
			{
			        memcpy(name, other.name, OBJECT_DATA_MAX_STRLEN);
			        memcpy(description, other.description, OBJECT_DATA_MAX_STRLEN);
			}
                };


		// NOTE: Visual object doesnt own any of mem of these ptrs. 
		// Its just collection of ptrs to elsewhere managed mem
		// (atm like like World's "VisualTile")
		class VisualObject
		{
		private:
			pk::Sprite3DRenderable* _pSprite = nullptr;
			
			// *NOTE! Below was bad idea, especially when object rotates in middle of anim..
			// Outermost vector[8] is for each different direction(N,NE,E...) of the sprite
			// Inner vector[varying] is for each animation of a single direction (as many anims we want)
			//std::vector<std::vector<pk::SpriteAnimator*>> _anims;
			
			// Contains each possible object sprite texture
			// NOTE: VisualObject does NOT own these textures!

			
		public:
			VisualObject(pk::Sprite3DRenderable* pSprite);
			VisualObject(const VisualObject& other);
			~VisualObject();
			
			void show(
				PK_ubyte tileObject, 
				PK_ubyte tileAction, 
				int objDir,
				int camDir,
				const ObjectInfo& staticObjInfo,
				float worldX, 
				float worldY,
			       	float worldZ
			);
			void hide();
		};
	}
}
