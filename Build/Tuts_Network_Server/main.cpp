
/******************************************************************************
Class: Net1_Client
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

:README:
- In order to run this demo, we also need to run "Tuts_Network_Client" at the same time.
- To do this:-
	1. right click on the entire solution (top of the solution exporerer) and go to properties
	2. Go to Common Properties -> Statup Project
	3. Select Multiple Statup Projects
	4. Select 'Start with Debugging' for both "Tuts_Network_Client" and "Tuts_Network_Server"

- Now when you run the program it will build and run both projects at the same time. =]
- You can also optionally spawn more instances by right clicking on the specific project
and going to Debug->Start New Instance.




FOR MORE NETWORKING INFORMATION SEE "Tuts_Network_Client -> Net1_Client.h"



		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <enet\enet.h>
#include <nclgl\GameTimer.h>
#include <nclgl\Vector3.h>
#include <nclgl\common.h>
#include <ncltech\NetworkBase.h>
#include <ncltech\MazeRenderer.h>
#include <ncltech\SearchAStar.h>
#include "InterpretData.h"
#include "Enemy.h"

//Needed to get computer adapter IPv4 addresses via windows
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")


#define SERVER_PORT 1234
#define UPDATE_TIMESTEP (1.0f / 30.0f) //send 30 position updates per second
#define AVATAR_SPEED 0.05f

NetworkBase server;
GameTimer timer;
float accum_time = 0.0f;
MazeGenerator gen;
bool isMaze;
map<enet_uint16, Instructions> commands;
SearchAStar* aStar;
list<const GraphNode*> currentPath;
list<const GraphNode*> completePath;
Vector3 avatarPos;
Vector3 enemyPos;
Enemy* roboGhost;
bool broadcastAvatar;

void Win32_PrintAllAdapterIPAddresses();

int onExit(int exitcode)
{
	server.Release();
	system("pause");
	exit(exitcode);
}

//Sends a message saying the type of data being sent to clients
void sendPacketType(ClientPacketType type) {
	ClientPacketType* send = new ClientPacketType(type);

	ENetPacket* packet = enet_packet_create(send, sizeof(ClientPacketType), 0);
	enet_host_broadcast(server.m_pNetwork, 0, packet);
}

//Function to send ove ra maze based on the paramaeters given
void sendMaze(MazeParams p) {

	sendPacketType(MAZE_UPDATE);

	gen.Generate(p.size, p.density);

	string mazeStr = gen.GetMazeString();
	const char* charStr = mazeStr.c_str();

	ENetPacket* mazePacket = enet_packet_create(charStr, strlen(charStr) + 1, 0);
	enet_host_broadcast(server.m_pNetwork, 0, mazePacket);

	isMaze = true;

}

//Generates a path through the maze using A*
void generatePath() {
	aStar = new SearchAStar();

	aStar->SetWeightings(1.0f, 1.0f);
	GraphNode* start = gen.GetStartNode();
	GraphNode* end = gen.GetGoalNode();
	list<const GraphNode*> oldList = currentPath;

	aStar->FindBestPath(start, end);
	currentPath = aStar->GetFinalPath();
	completePath = currentPath;

	//If the avatar is halfway throughway through a route, make it route to the new start node first
	if (oldList.size() != 0 && broadcastAvatar) {
		GraphNode* temp = new GraphNode();
		GraphNode next = **oldList.begin();
		//Neccesary to have an actual point and not have the memory disapear
		GraphNode* nextPoint = new GraphNode();

		nextPoint->_pos = next._pos;
		nextPoint->_neighbours = next._neighbours;
		nextPoint->_visited = next._visited;

		GraphEdge* edge = new GraphEdge();
		edge->weighting = 1.0f;
		edge->_a = temp;
		edge->_b = nextPoint;
		edge->_iswall = false;
		edge->_connected = true;

		temp->_pos = avatarPos;
		temp->_neighbours.push_back(edge);
		aStar->FindBestPath(temp, start);

		list<const GraphNode*> tempList;

		tempList = aStar->GetFinalPath();
		tempList.pop_back();

		for (list<const GraphNode*>::reverse_iterator i = tempList.rbegin(); i != tempList.rend();i++) {
			currentPath.push_front(*i);
		}
		
		
	}
}

int main(int arcg, char** argv)
{
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}

	//Initialize Server on Port 1234, with a possible 32 clients connected at any time
	if (!server.Initialize(SERVER_PORT, 32))
	{
		fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
		onExit(EXIT_FAILURE);
	}

	printf("Server Initiated\n");

	broadcastAvatar = false;
	isMaze = false;
	roboGhost = new Enemy();

	Win32_PrintAllAdapterIPAddresses();

	timer.GetTimedMS();
	while (true)
	{
		float dt = timer.GetTimedMS() * 0.001f;
		accum_time += dt;

		//Handle All Incoming Packets and Send any enqued packets
		server.ServiceNetwork(dt, [&](const ENetEvent& evnt)
		{
			Instructions command;

			switch (evnt.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				printf("- New Client Connected\n");
				commands[evnt.peer->incomingPeerID] = NONE;
				//If there is a maze, broadcasts it to the new client
				if (isMaze) {
					sendPacketType(CONNECT_MAZE);

					string mazeStr = gen.GetMazeString();
					const char* charStr = mazeStr.c_str();

					ENetPacket* mazePacket = enet_packet_create(charStr, strlen(charStr) + 1, 0);
					enet_host_broadcast(server.m_pNetwork, 0, mazePacket);
				}
				break;

			case ENET_EVENT_TYPE_RECEIVE:

				command = commands[evnt.peer->incomingPeerID];

				//If the last command wasn't one that needed data
				if (command == NONE) {

					Instructions ins;

					memcpy(&ins, evnt.packet->data, sizeof(Instructions));
					
					//Creates a route through the amze and sets the avatar to move through it
					if (ins == CREATE_ROUTE) {
						generatePath();

						const GraphNode* node = *currentPath.begin();

						avatarPos = node->_pos;
						broadcastAvatar = true;
						currentPath.pop_front();
					}

					//Sends a translated path to the the client to be highighted
					else if (ins == GET_ROUTE) {

						if (completePath.size() == 0) {
							generatePath();
						}

						string dat = "";

						for (list<const GraphNode*>::iterator i = completePath.begin(); i != completePath.end(); i++) {
							dat = dat + to_string((*i)->_pos.x) + ",";
							dat = dat + to_string((*i)->_pos.y) + ",";
							dat = dat + to_string((*i)->_pos.z) + ";";
						}

						const char* send = dat.c_str();

						sendPacketType(ROUTE_UPDATE);

						ENetPacket* packet = enet_packet_create(send, strlen(send) + 1, 0);
						enet_host_broadcast(server.m_pNetwork, 0, packet);
					}
					else {
						commands[evnt.peer->incomingPeerID] = ins;
					}

					enet_packet_destroy(evnt.packet);
					
					break;
				}
				else {
					//Makes a maze from the parameters sent and sends back details of the maze
					if (command == MAZE_PARAMS) {
						MazeParams p;
						memcpy(&p, evnt.packet->data, sizeof(p));

						printf("Size: %i Density: %f \n", p.size, p.density);

						enet_packet_destroy(evnt.packet);

						sendMaze(p);

						currentPath.clear();
						completePath.clear();
						broadcastAvatar = false;

					}
					//Recieives updated start and end nodes for the maze
					else if (command == TRAVEL_NODES) {
						TravelNodes nodes;
						memcpy(&nodes, evnt.packet->data, sizeof(TravelNodes));

						gen.SetTravelNodes(nodes.start, nodes.goal);
						generatePath();

						sendPacketType(CONFIRM_NODES);

						ENetPacket* packet = enet_packet_create(&nodes, sizeof(TravelNodes), 0);
						enet_host_broadcast(server.m_pNetwork, 0, packet);


					}

					commands[evnt.peer->incomingPeerID] = NONE;
					break;
				}

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("- Client %d has disconnected.\n", evnt.peer->incomingPeerID);
				break;
			}
		});
		
		if (broadcastAvatar) {
			//Updates the enemy
			roboGhost->updatePlayerPos(avatarPos);
			roboGhost->OnGlobalUpdate(dt);
		}

		//Broadcast update packet to all connected clients at a rate of UPDATE_TIMESTEP updates per second
		if (accum_time >= UPDATE_TIMESTEP)
		{

			//Packet data
			// - At the moment this is just a position update that rotates around the origin of the world
			//   though this can be any variable, structure or class you wish. Just remember that everything 
			//   you send takes up valuable network bandwidth so no sending every PhysicsObject struct each frame ;)
			accum_time = 0.0f;

			//Broadcasts the avatars position if it's moving
			if (broadcastAvatar) {
				Vector3 nextPos = (*currentPath.begin())->_pos;
				Vector3 nextStep;
				Vector3 difference;
				
				difference = nextPos - avatarPos;

				if (difference.x < 0.01f && difference.x > -0.01f && difference.y < 0.01f && difference.y > -0.01f) {
					avatarPos = nextPos;
					currentPath.pop_front();
				}
				else {
					avatarPos = avatarPos + (difference.Normalise() * AVATAR_SPEED);
				}

				sendPacketType(PLAYER_UPDATE);

				ENetPacket* position_update = enet_packet_create(&avatarPos, sizeof(Vector3), 0);
				enet_host_broadcast(server.m_pNetwork, 0, position_update);

				sendPacketType(ENEMY_UPDATE);

				//Get the enemies updated position and broadcasts it
				Vector3 enemyPos = roboGhost->getPos();

				position_update = enet_packet_create(&enemyPos, sizeof(Vector3), 0);
				enet_host_broadcast(server.m_pNetwork, 0, position_update);

				//If avatar has reached goal, don't update it
				if (currentPath.size() == 0) {
					broadcastAvatar = false;
				}
				
			}
		}

		Sleep(0);
	}

	system("pause");
	server.Release();
}




//Yay Win32 code >.>
//  - Grabs a list of all network adapters on the computer and prints out all IPv4 addresses associated with them.
void Win32_PrintAllAdapterIPAddresses()
{
	//Initially allocate 5KB of memory to store all adapter info
	ULONG outBufLen = 5000;
	

	IP_ADAPTER_INFO* pAdapters = NULL;
	DWORD status = ERROR_BUFFER_OVERFLOW;

	//Keep attempting to fit all adapter info inside our buffer, allocating more memory if needed
	// Note: Will exit after 5 failed attempts, or not enough memory. Lets pray it never comes to this!
	for (int i = 0; i < 5 && (status == ERROR_BUFFER_OVERFLOW); i++)
	{
		pAdapters = (IP_ADAPTER_INFO *)malloc(outBufLen);
		if (pAdapters != NULL) {

			//Get Network Adapter Info
			status = GetAdaptersInfo(pAdapters, &outBufLen);

			// Increase memory pool if needed
			if (status == ERROR_BUFFER_OVERFLOW) {
				free(pAdapters);
				pAdapters = NULL;
			}
			else {
				break;
			}
		}
	}

	
	if (pAdapters != NULL)
	{
		//Iterate through all Network Adapters, and print all IPv4 addresses associated with them to the console
		// - Adapters here are stored as a linked list termenated with a NULL next-pointer
		IP_ADAPTER_INFO* cAdapter = &pAdapters[0];
		while (cAdapter != NULL)
		{
			IP_ADDR_STRING* cIpAddress = &cAdapter->IpAddressList;
			while (cIpAddress != NULL)
			{
				printf("\t - Listening for connections on %s:%u\n", cIpAddress->IpAddress.String, SERVER_PORT);
				cIpAddress = cIpAddress->Next;
			}
			cAdapter = cAdapter->Next;
		}

		free(pAdapters);
	}
	
}

