#pragma once

#include <ncltech\Scene.h>

class TestScene : public Scene
{
public:
	TestScene(const std::string& friendly_name);
	virtual ~TestScene();

	virtual void OnInitializeScene()	 override;
	virtual void OnCleanupScene()		 override;
	virtual void OnUpdateScene(float dt) override;

protected:

	void GenerateSoftBody(int width = 5, int height = 10, bool fixed = false, Vector3 pos = Vector3(0,0,0));

	float m_AccumTime;
	GameObject* m_pPlayer;
};