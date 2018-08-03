#pragma once

#include "PhysicsNode.h"
#include <vector>

#define MAX_TREES 5

struct CollisionPair	//Forms the output of the broadphase collision detection
{
	PhysicsNode* pObjectA;
	PhysicsNode* pObjectB;
};

struct Quad {
	std::vector<PhysicsNode*> posCols;
	Vector3 origin;
	Vector3 size;
};


//The actual worst octree class. Needs A LOT of fixing to be usuable. Currently works but is significantly worse than a basic sphere-sphere broadphase check. Needs to be reimplemented from scratch
class Octree
{
public:
	Octree(Vector3 worldSize = Vector3(100,100,100), Vector3 worldOrigin = Vector3(0,0,0), int objectThreshold = 3);
	~Octree();

	//void addNode(PhysicsNode* node);

	std::vector<CollisionPair> getPossibleCollisions(std::vector<PhysicsNode*> pNodes);

	void setOrigin(Vector3 origin) { worldOrigin = origin; };
	void setSize(Vector3 size) { worldSize = size; };


private:
	//std::vector<PhysicsNode*> pNodes;
	Vector3 worldSize;
	Vector3 worldOrigin;
	int objectThreshold;

	std::vector<PhysicsNode*> checkQuadrant(Vector3 size, Vector3 origin, std::vector<PhysicsNode*> possibleNodes);
};

//bool operator==(const Quad& first, const Quad& second) {
//	return first.origin == second.origin && first.posCols == second.posCols && first.size == second.size;
//}
