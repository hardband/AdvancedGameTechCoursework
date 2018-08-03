#pragma once

#include <string>

//Instructions a client can send to the server
enum Instructions {
	NONE,
	MAZE_PARAMS,
	TRAVEL_NODES,
	CREATE_ROUTE,
	GET_ROUTE
};

//Server to client message saying what server is sending
enum ClientPacketType {
	MAZE_UPDATE,
	PLAYER_UPDATE,
	ROUTE_UPDATE,
	CONNECT_MAZE,
	ENEMY_UPDATE,
	CONFIRM_NODES
};

//Struct to store data for updating start and end nodes
struct TravelNodes {
	Vector3 start;
	Vector3 goal;
};