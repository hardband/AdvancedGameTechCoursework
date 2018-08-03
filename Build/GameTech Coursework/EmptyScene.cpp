#include "EmptyScene.h"

void EmptyScene::spawnSphere(Vector3 pos)
{
	GameObject* sphere = CommonUtils::BuildSphereObject(
		"",					// Optional: Name
		pos,				// Position
		5.0f,			// Half-Dimensions
		true,				// Physics Enabled?
		10.f,				// Physical Mass (must have physics enabled)
		true,				// Physically Collidable (has collision shape)
		true,				// Dragable by user?
		Vector4(1, 1, 1, 1));// Render color

	this->AddGameObject(sphere);
}
