#include "BallScene.h"

#define WALL_HEIGHT 10

BallScene::~BallScene()
{
}

void BallScene::OnInitializeScene()
{
	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"Ground",
		Vector3(0.0f, -WALL_HEIGHT, 0.0f),//Pos
		Vector3(20.0f, 1.0f, 20.0f),//Scale
		true,//Physics
		0.0f,//Mass
		true,//Collideable
		false,//Draggable
		Vector4(0.2f, 0.5f, 1.0f, 1.0f)));//Colour

	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"",
		Vector3(0.0f, 0.0f, 20.0f),//Pos
		Vector3(20.0f, WALL_HEIGHT, 1.0f),//Scale
		true,//Physics
		0.0f,//Mass
		true,//Collideable
		false,//Draggable
		Vector4(0.2f, 0.5f, 1.0f, 1.0f)));//Colour

	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"",
		Vector3(0.0f, 0.0f, -20.0f),//Pos
		Vector3(20.0f, WALL_HEIGHT, 1.0f),//Scale
		true,//Physics
		0.0f,//Mass
		true,//Collideable
		false,//Draggable
		Vector4(0.2f, 0.5f, 1.0f, 1.0f)));//Colour

	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"",
		Vector3(20.0f, 0.0f, 0.0f),//Pos
		Vector3(1.0f, WALL_HEIGHT, 20.0f),//Scale
		true,//Physics
		0.0f,//Mass
		true,//Collideable
		false,//Draggable
		Vector4(0.2f, 0.5f, 1.0f, 1.0f)));//Colour

	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"",
		Vector3(-20.0f, 0.0f, 0.0f),//Pos
		Vector3(1.0f, WALL_HEIGHT, 20.0f),//Scale
		true,//Physics
		0.0f,//Mass
		true,//Collideable
		false,//Draggable
		Vector4(0.2f, 0.5f, 1.0f, 1.0f)));//Colour

	for (int x = 0; x < 5; x++) {
		for (int y = 0; y < 5; y++) {
			for (int z = 0; z < 5; z++) {

				Vector3 pos;

				if (y % 2 == 0) {
					pos = Vector3(x - 2.5, y + 2.5, z - 2.5);
				}
				else {
					pos = Vector3(x - 3, y + 2.5 , z - 2.5);
				}

				pos = pos * 6;

				this->AddGameObject(CommonUtils::BuildSphereObject(
					"",
					pos,//Pos
					2.0f,//Scale
					true,//Physics
					2.0f,//Mass
					true,//Collideable
					false,//Draggable
					Vector4(0.2f, 0.5f, 1.0f, 1.0f)));//Colour
			}
		}
	}

}

void BallScene::spawnSphere(Vector3 pos)
{
}
