#pragma once

#include <functional>
#include <algorithm>
#include <ncltech\Scene.h>
#include <ncltech\GameObject.h>

#define MAX_BATTERY 50
#define RECHARGE_RATE 10
#define IDLE_DRAIN 4
#define MOVE_DRAIN 8
#define AGRO_RANGE 10

typedef std::function<void(float dt)> OnStateUpdateCallback;

//FSM based enemy class
class Enemy
{
public:
	Enemy(Vector3 pos = Vector3(0,0,0), float speed = 2.0f);
	~Enemy();

	void OnGlobalUpdate(float dt);

	Vector3 getPos();
	void updatePlayerPos(Vector3 pos);

protected:
	//Idle state
	void OnIdleState(float dt);
	//Recharging State
	void OnRechargeState(float dt);
	//Following state
	void OnMoveTowardsPlayer(float dt);

	Vector3 currentPosition;
	Vector3 playerPos;

	float speed;

	float battery;

	OnStateUpdateCallback currentState;
};

