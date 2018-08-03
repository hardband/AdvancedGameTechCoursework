#pragma once
#include <ncltech\Scene.h>
#include <ncltech\CommonUtils.h>
class BallScene :
	public Scene
{
public:
	BallScene(const std::string& friendly_name) : Scene(friendly_name) {};
	~BallScene();

	virtual void OnInitializeScene() override;

	void spawnSphere(Vector3 pos);
};

