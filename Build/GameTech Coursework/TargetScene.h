#pragma once
#include <ncltech\Scene.h>

#include <nclgl\Vector4.h>
#include <ncltech\GraphicsPipeline.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>

class TargetScene :
	public Scene
{
public:
	TargetScene(const std::string& friendly_name);
	~TargetScene();

	virtual void OnInitializeScene()	 override;
	virtual void OnCleanupScene()		 override;
	virtual void OnUpdateScene(float dt) override;

protected:

	void buildTarget(Vector3 pos,Vector3 size = Vector3(5.0f,5.0f,0.5f), bool good = true);
	bool TargetCollision(PhysicsNode* self, PhysicsNode* collidingObject);

	int score;
};

