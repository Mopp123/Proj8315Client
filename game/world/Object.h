#pragma once

#include <vector>
#include <unordered_map>

#include "../../PortablePesukarhu/ppk.h"
#include "Tile.h"


#define OBJECT_DATA_STRLEN_NAME 32
#define OBJECT_DATA_STRLEN_DESCRIPTION 32
#define OBJECT_DATA_STRLEN_ACTION_NAME 16
                                                                                                       
#define OBJECT_DATA_COUNT_STATS 1
#define OBJECT_DATA_COUNT_TOTAL ((1 + 1) + (TILE_STATE_MAX_action + 1) + OBJECT_DATA_COUNT_STATS)


namespace world
{
	class VisualWorld;

	namespace objects
	{
		struct ObjectInfo
                {
			// misc. stuff
			char name[OBJECT_DATA_STRLEN_NAME];
			char description[OBJECT_DATA_STRLEN_DESCRIPTION];
			
			char actionSlot[TILE_STATE_MAX_action + 1][OBJECT_DATA_STRLEN_ACTION_NAME];

			// stats
			PK_ubyte speed;

			// Client exclusive properties:
			pk::web::WebTexture* pTexture = nullptr; // *NOTE! ObjectInfo doesn't own its' texture
			bool rotateableSprite = false;

			ObjectInfo(
				std::string objName,
				std::string objDescription,
				std::vector<std::string> actionSlots,
				PK_ubyte speedVal
			):
				speed(speedVal)
			{

				memset(name, 0, sizeof(char) * OBJECT_DATA_STRLEN_NAME);
				memset(description, 0, sizeof(char) * OBJECT_DATA_STRLEN_DESCRIPTION);
				
				size_t nameLen = objName.size();
				size_t descriptionLen = objDescription.size();

				if (nameLen > OBJECT_DATA_STRLEN_NAME || nameLen == 0)
					nameLen = OBJECT_DATA_STRLEN_NAME;
				if (descriptionLen > OBJECT_DATA_STRLEN_DESCRIPTION || nameLen == 0)
					descriptionLen = OBJECT_DATA_STRLEN_DESCRIPTION;

				memcpy(name, objName.c_str(), sizeof(char) * nameLen);
				memcpy(description, objDescription.c_str(), sizeof(char) * descriptionLen);
				
				for (size_t i = 0; i < TILE_STATE_MAX_action + 1; ++i)
				{
					memset(actionSlot[i], 0, sizeof(char) * OBJECT_DATA_STRLEN_ACTION_NAME);
					if (i < actionSlots.size())
					{
						const std::string& slotName = actionSlots[i];
						size_t slotLen = slotName.size();
						if (slotLen > OBJECT_DATA_STRLEN_ACTION_NAME)
							slotLen = OBJECT_DATA_STRLEN_ACTION_NAME;

						memcpy(actionSlot[i], slotName.c_str(), sizeof(char) * slotLen);
					}
				}
			}
			
			ObjectInfo(const ObjectInfo& other):
				speed(other.speed),
				pTexture(other.pTexture),
				rotateableSprite(other.rotateableSprite)
			{
				memcpy(this->name, other.name, OBJECT_DATA_STRLEN_NAME);
				memcpy(this->description, other.description, OBJECT_DATA_STRLEN_DESCRIPTION);
				for (int i = 0; i < TILE_STATE_MAX_action + 1; ++i)
					memcpy(actionSlot[i], other.actionSlot[i], OBJECT_DATA_STRLEN_ACTION_NAME);
			}
                };
		

		// Returns the size of data moving accross netw (excludes server and client specific data)
		size_t get_netw_objinfo_size();


		// NOTE: Visual object doesnt own any of mem of these ptrs. 
		// Its just collection of ptrs to elsewhere managed mem
		// (atm like like World's "VisualTile")
		class VisualObject
		{
		private:
			VisualWorld& _worldRef;
			pk::Sprite3DRenderable* _pSprite = nullptr;
			float _verticalOffset = 0.0f;
			
			// *NOTE! Below was bad idea, especially when object rotates in middle of anim..
			// Outermost vector[8] is for each different direction(N,NE,E...) of the sprite
			// Inner vector[varying] is for each animation of a single direction (as many anims we want)
			//std::vector<std::vector<pk::SpriteAnimator*>> _anims;
		public:
			VisualObject(pk::Scene& scene, VisualWorld& worldRef, pk::Sprite3DRenderable* pSprite);
			VisualObject(const VisualObject& other);
			~VisualObject();

			void assignAnimFrames(PK_ubyte tileObject, PK_ubyte tileAction, pk::Animation* anim);
			
			void show(
				PK_ubyte tileObject, 
				PK_ubyte tileAction, 
				int objDir,
				int camDir,
				const ObjectInfo& staticObjInfo,
				float worldX, 
			       	float worldZ,
				pk::Animation* animation,
				pk::vec3& tileMovement
			);
			void hide();

		private:
			void move(int dir, float speed, pk::vec3& tileMovement);
			void moveVertical(int dir, float speed, pk::vec3& tileMovement);
		};
	}
}
