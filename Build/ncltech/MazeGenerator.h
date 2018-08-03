#pragma once
#include <ncltech\GameObject.h>
#include <ncltech\Scene.h>
#include "SearchAlgorithm.h"

struct MazeParams {
	int size;
	float density;
};

struct DataEdge {
	Vector3 first;
	Vector3 second;

	float weighting;

	bool connected;
	bool wall;

	inline bool operator<(const DataEdge &A)const {
		return first + second < A.first + A.second;
	}

	inline bool operator>(const DataEdge &A)const {
		return first + second > A.first + A.second;
	}
};

struct DataNode {
	Vector3 pos;
	deque<DataEdge> neighbourPos;
};

struct MazeData {
	uint size;
	DataNode startNode;
	DataNode endNode;
	vector<DataNode> allNodes;
	vector<DataEdge> allEdges;

};


class MazeGenerator
{
public:
	MazeGenerator(); //Maze_density goes from 1 (single path to exit) to 0 (no walls at all)
	virtual ~MazeGenerator();

	void Generate(int size, float maze_density);
	
	void GenerateFromData(MazeData data);
	MazeData GetMazeData();

	//All points on the maze grid are connected in some shape or form
	// so any two nodes picked randomly /will/ have a path between them
	GraphNode* GetStartNode() const		{ return start; }
	GraphNode* GetGoalNode()  const		{ return end; }
	uint GetSize() const { return size; }


	//Used as a hack for the MazeRenderer to generate the walls more effeciently
	GraphNode* GetAllNodesArr() { return allNodes; }

	string GetMazeString();
	void ReadFromString(string maze);

	void SetTravelNodes(Vector3 start, Vector3 end);

	void RandomiseStartEndNodes();

protected:
	void GetRandomStartEndNodes();

	void Initiate_Arrays();

	void Generate_Prims();
	void Generate_Sparse(float density);

	DataEdge translateEdge(GraphEdge* edge);

	

public:
	uint size;
	GraphNode *start, *end;

	GraphNode* allNodes;
	GraphEdge* allEdges;
};