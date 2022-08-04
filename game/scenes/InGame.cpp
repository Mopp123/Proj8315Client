
#include "InGame.h"

#include "../net/Client.h"

#include "../net/Client.h"
#include "../net/requests/Commands.h"
#include "../net/NetCommon.h"

#include "../world/Tile.h"


#include <iostream>
#include <chrono>

using namespace pk;
using namespace pk::web;

using namespace ui;
using namespace net;
using namespace net::web;


class OnSubmit_position : public InputFieldOnSubmitEvent
{
public:
	RTSCamController* camController = nullptr;
	OnSubmit_position(RTSCamController* camcontrol)
	{
		camController = camcontrol;
	}

	void onSubmit(std::string inputFieldText)
	{
		std::string total = inputFieldText;

		size_t delimPos = total.find(',');
		std::string x_str = total.substr(0, delimPos);
		total.erase(0, delimPos + 1);
		std::string z_str = total;

		Debug::log("parse result: x=" + x_str + " z=" + z_str);

		const int tileSize = 2;

		float worldX = (float)(std::stoi(x_str) * tileSize);
		float worldZ = (float)(std::stoi(z_str) * tileSize);

		camController->setPivotPoint({ worldX, 0, worldZ });

	};

};

InGame::InGame()
{}


InGame::~InGame()
{
	delete _pCamController;

	delete _visualWorld;

	delete _pText_debug_delta;

	delete _terrainTexture0;
	delete _terrainTexture1;
	delete _terrainTexture2;
	delete _terrainTexture3;
	delete _terrainTexture4;
}




static std::string s_TEST_worldstate;
static Text* s_TEST_text = nullptr;

void InGame::init()
{
	// create default camera
	activeCamera = create_camera({ 0,0,0 });
	// create dir light
	uint32_t lightEntity = createEntity();
	vec3 lightDir(0.0f, -0.5f, -0.5f);
	lightDir.normalize();
	addComponent(lightEntity, new DirectionalLight({ 1,1,1 }, lightDir));



	const float textSize = 16;
	const float rowPadding = 5;

	const float buttonSize = 24;

	const float panelX = 128;
	const float panelY = 128;

	// For debugging
	_pText_debug_delta = new Text(
		"",
		{
			{ ConstraintType::PIXEL_TOP, 5 },
			{ ConstraintType::PIXEL_LEFT, 5 }
		}
	);


	std::chrono::time_point<std::chrono::steady_clock> startTime = std::chrono::high_resolution_clock::now();
	
	Client::get_instance()->setUserID("Persekorva666");

	// Load channel textures for terrain's tiles
	TextureSampler terrainTileTexSampler =
	{
		TextureSamplerFilterMode::PK_SAMPLER_FILTER_MODE_LINEAR,
		TextureSamplerAddressMode::PK_SAMPLER_ADDRESS_MODE_REPEAT,
		2
	};
	_terrainTexture0 = new WebTexture("assets/deadland.png", terrainTileTexSampler);
	_terrainTexture1 = new WebTexture("assets/water.png", terrainTileTexSampler);
	_terrainTexture2 = new WebTexture("assets/snow.png", terrainTileTexSampler);
	_terrainTexture3 = new WebTexture("assets/rock.png", terrainTileTexSampler);
	_terrainTexture4 = new WebTexture("assets/grass.png", terrainTileTexSampler);
	
	TerrainTileRenderable::s_channelTexture0 = _terrainTexture0;
	TerrainTileRenderable::s_channelTexture1 = _terrainTexture1;
	TerrainTileRenderable::s_channelTexture2 = _terrainTexture2;
	TerrainTileRenderable::s_channelTexture3 = _terrainTexture3;
	TerrainTileRenderable::s_channelTexture4 = _terrainTexture4;

	_visualWorld = new world::VisualWorld(*this, 15);



	float delta = (std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - startTime)).count();
	
	Debug::log("building took: " + std::to_string(delta));

	Transform* camTransformComponent = (Transform*)getComponent(activeCamera->getEntity(), ComponentType::PK_TRANSFORM);
	_pCamTransform = &(camTransformComponent)->accessTransformationMatrix();
	_pCamTransform->setIdentity();
	mat4& camTransform = *_pCamTransform;

	camTransform[0 + 3 * 4] = 10.0f;
	camTransform[1 + 3 * 4] = 10.0f;
	camTransform[2 + 3 * 4] = 25.0f;

	float angle = -0.7f;

	camTransform[1 + 1 * 4] = cos(angle);
	camTransform[1 + 2 * 4] = -sin(angle);
	
	camTransform[2 + 1 * 4] = sin(angle);
	camTransform[2 + 2 * 4] = cos(angle);


	_pCamController = new RTSCamController(*activeCamera, this);


	// TESTING 3D sprites..
	uint32_t spriteEntity = createEntity();
	_testSprite = new Sprite3DRenderable({ 0,0,0 }, { 2,2 });
	addComponent(spriteEntity, _testSprite);

	

	// TESTING tile anims
	_animator_water = new SpriteAnimator({ {0,1},{1,1},{2,1} }, 3.0f);
	_animator_water->enableLooping(true);
	_animator_water->play();
	addSystem(_animator_water);

	_animator_grass = new SpriteAnimator({ {2,0},{3,0},{4,0},{3,0} }, 4.0f);
	_animator_grass->enableLooping(true);
	_animator_grass->play();
	addSystem(_animator_grass);




	// JUST FOR DEBUGGING
	_inputField_position = new InputField(
		" Enter message",
		{
			{ConstraintType::PIXEL_LEFT, 32},
			{ConstraintType::PIXEL_BOTTOM, 32}
		},
		300,
		new OnSubmit_position(_pCamController),
		true
	);
}

void InGame::update()
{
	


	vec3 camPivotPoint = _pCamController->getPivotPoint();
	
	// attempt to glue cam's height to terrain's height
	camPivotPoint.y = _visualWorld->getTileVisualHeightAt(camPivotPoint.x, camPivotPoint.z);
	_pCamController->setPivotPoint(camPivotPoint);
	
	_visualWorld->update(camPivotPoint.x, camPivotPoint.z);
	
	

	_pText_debug_delta->accessRenderable()->accessStr() = "Delta: " + std::to_string(Timing::get_delta_time());


	// debug mouse picking testing..
	mat4 viewMatrix = *_pCamTransform;
	viewMatrix.inverse();
	
	_testSprite->position = _visualWorld->getMousePickCoords(activeCamera->getProjMat3D(), viewMatrix);
}
