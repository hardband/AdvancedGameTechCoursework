
#pragma once

#include <ncltech\Scene.h>
#include <ncltech\NetworkBase.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <nclgl\NCLDebug.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\MazeRenderer.h>
#include "../Tuts_Network_Server/InterpretData.h"

#define MAZE_SCALE 15.0f

//Netowrk client code for maze

class Net1_Client : public Scene
{
public:
	Net1_Client(const std::string& friendly_name);

	virtual void OnInitializeScene() override;
	virtual void OnCleanupScene() override;
	virtual void OnUpdateScene(float dt) override;


	void ProcessNetworkEvent(const ENetEvent& evnt);

protected:

	void RequestMaze();
	void RequestAvatar();
	void RequestRoute();
	void SendTravelNodes();
	//void ChangeState(Status s);

	GameObject* avatar;
	GameObject* roboGhost;
	MazeRenderer* maze;
	MazeGenerator gen;

	MazeParams* mParams;

	ClientPacketType nextPacket;

	bool waitingForData;

	char* path = NULL;

	NetworkBase network;
	ENetPeer*	serverConnection;
};