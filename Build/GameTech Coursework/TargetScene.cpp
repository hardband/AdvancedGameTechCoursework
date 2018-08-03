#include "TargetScene.h"



TargetScene::TargetScene(const std::string& friendly_name) : Scene(friendly_name)
{
}


TargetScene::~TargetScene()
{
}

void TargetScene::OnInitializeScene()
{

	score = 0;

	buildTarget(Vector3(-5.0f, 0, 5.0f));
	buildTarget(Vector3(5.0f, 0, 5.0f), Vector3(5,5,0.5),false);

}

void TargetScene::OnCleanupScene()
{
}

void TargetScene::OnUpdateScene(float dt)
{
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f),"Score: " + std::to_string(score));
}

void TargetScene::buildTarget(Vector3 pos,Vector3 size, bool good)
{
	GameObject* target;

	if (good) {
		target = CommonUtils::BuildCuboidObject(
			"goodTarget",					// Optional: Name
			pos,	// Position
			size,  // Half-Dimensions
			true,						// Physics Enabled?
			0.0f,						// Physical Mass (must have physics enabled)
			true,						// Physically Collidable (has collision shape)
			false,						// Dragable by user?
			Vector4(0.1f, 0.9f, 0.1f, 1.0f)); // Render color
	}
	else {
		target = CommonUtils::BuildCuboidObject(
			"badTarget",					// Optional: Name
			pos,	// Position
			size,  // Half-Dimensions
			true,						// Physics Enabled?
			0.0f,						// Physical Mass (must have physics enabled)
			true,						// Physically Collidable (has collision shape)
			false,						// Dragable by user?
			Vector4(0.9f, 0.1f, 0.1f, 1.0f)); // Render color
	}

	target->Physics()->SetOnCollisionCallback(std::bind(&TargetScene::TargetCollision,
			this,							//Any non-placeholder param will be passed into the function each time it is called
			std::placeholders::_1,			//The placeholders correlate to the expected parameters being passed to the callback
			std::placeholders::_2
		));

	this->AddGameObject(target);

}

bool TargetScene::TargetCollision(PhysicsNode * self, PhysicsNode * collidingObject)
{
	if (collidingObject->GetParent()->GetName() == "thrown") {
		if (self->GetParent()->GetName() == "goodTarget") {
			score += 100;
		}
		else if (self->GetParent()->GetName() == "badTarget") {
			score -= 50;
		}

		PhysicsEngine::Instance()->RemovePhysicsObject(collidingObject);
		delete collidingObject;
	}

	return false;
}
