#include "Enemy.h"



Enemy::Enemy(Vector3 pos, float speed)
{
	battery = MAX_BATTERY;
	currentPosition = pos;
	this->speed = speed;
	currentState = std::bind(&Enemy::OnIdleState, this, std::placeholders::_1);
}


Enemy::~Enemy()
{
}


void Enemy::OnGlobalUpdate(float dt)
{
	currentState(dt);
}

Vector3 Enemy::getPos()
{
	return currentPosition;
}

void Enemy::updatePlayerPos(Vector3 pos)
{
	playerPos = pos;
}

void Enemy::OnIdleState(float dt)
{

	battery -= IDLE_DRAIN * dt;

	//If battery is drained recharge
	if (battery <= 0.0f) {
		battery = 0.0f;
		currentState = std::bind(&Enemy::OnRechargeState, this, std::placeholders::_1);
	}
	//If player is with aggro range
	else if ((currentPosition - playerPos).Length() <= AGRO_RANGE) {
		currentState = std::bind(&Enemy::OnMoveTowardsPlayer, this, std::placeholders::_1);
	}


}

void Enemy::OnRechargeState(float dt)
{
	battery += RECHARGE_RATE * dt;

	//Become idle if battery is charged
	if (battery >= MAX_BATTERY) {
		battery = MAX_BATTERY;
		currentState = std::bind(&Enemy::OnIdleState, this, std::placeholders::_1);
	}
}

void Enemy::OnMoveTowardsPlayer(float dt)
{
	currentPosition = currentPosition + ((playerPos - currentPosition).Normalise() * speed * dt);
	battery -= MOVE_DRAIN * dt;

	//If battery becomes empty, start recharging
	if (battery <= 0.0f) {
		battery = 0.0f;
		currentState = currentState = std::bind(&Enemy::OnIdleState, this, std::placeholders::_1);
	}
}
