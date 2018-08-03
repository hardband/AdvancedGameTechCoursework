#include "Octree.h"


Octree::Octree(Vector3 worldSize, Vector3 worldOrigin, int objectThreshold)
{
	this->worldSize = worldSize;
	this->worldOrigin = worldOrigin;
	this->objectThreshold = objectThreshold;
}

Octree::~Octree()
{
}

//void Octree::addNode(PhysicsNode* node)
//{
//	pNodes.push_back(node);
//}

std::vector<CollisionPair> Octree::getPossibleCollisions(std::vector<PhysicsNode*> pNodes)
{
	std::vector<Quad> collisions;
	std::vector<CollisionPair> colPairs;

	Quad worldQ;

	worldQ.size = worldSize;
	worldQ.origin = worldOrigin;
	worldQ.posCols = pNodes;

	collisions.push_back(worldQ);

	int trees = 0;

	for (std::vector<Quad>::iterator iter = collisions.begin(); iter != collisions.end() && !(trees >= MAX_TREES) ;iter++){

		std::vector<Quad> newQuads;

		if ((*iter).posCols.size() > objectThreshold) {

			for (int x = -1; x < 2; x = x + 2) {
				for (int y = -1; y < 2; y = y + 2) {
					for (int z = -1; z < 2; z = z + 2) {

						Quad newQ;

						newQ.size = (*iter).size / 2;
						newQ.origin = Vector3((*iter).origin.x + ((newQ.size.x / 2) * x), (*iter).origin.y + ((newQ.size.y / 2) * y), (*iter).origin.z + ((newQ.size.z / 2)  * z));
						newQ.posCols = checkQuadrant(newQ.size, newQ.origin, (*iter).posCols);

						newQuads.push_back(newQ);
					}
				}
			}

			collisions.erase(iter);

			for (std::vector<Quad>::iterator q = newQuads.begin(); q != newQuads.end(); q++) {
				collisions.push_back(*q);
			}

			iter = collisions.begin();

			trees++;

		}
	}
	
	for (std::vector<Quad>::iterator iter = collisions.begin(); iter != collisions.end();iter++) {
		for (std::vector<PhysicsNode*>::iterator first = (*iter).posCols.begin(); first != (*iter).posCols.end();first++) {
			
			std::vector<PhysicsNode*>::iterator second = first;

			if (second != (*iter).posCols.end()) {
				second++;
			}
			
			for (;second != (*iter).posCols.end(); second++) {
				CollisionPair pair;

				pair.pObjectA = (*first);
				pair.pObjectB = (*second);

				colPairs.push_back(pair);
				
			}

		}
	}

	return colPairs;
}

//Returns a list of physics nodes withing the quadrant
std::vector<PhysicsNode*> Octree::checkQuadrant(Vector3 size, Vector3 origin, std::vector<PhysicsNode*> possibleNodes)
{

	std::vector<PhysicsNode*> quadNodes;

	for (std::vector<PhysicsNode*>::iterator iter = possibleNodes.begin(); iter != possibleNodes.end(); iter++) {
		
		if ((*iter)->GetCollisionShape() != NULL) {
			Vector3 pos = (*iter)->GetPosition();
			float radius = (*iter)->GetCollisionShape()->getRadius();

			Vector3 min = origin - (size / 2);
			Vector3 max = origin + (size / 2);

			if (pos.x + radius >= min.x && pos.x - radius <= max.x
				&& pos.y + radius >= min.y && pos.y - radius <= max.y
				&& pos.z + radius >= min.z && pos.z - radius <= max.z) {

				quadNodes.push_back(*iter);
			}
		}
		

	}

	return quadNodes;
}
