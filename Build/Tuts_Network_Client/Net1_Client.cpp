/******************************************************************************
Class: Net1_Client
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

:README:
- In order to run this demo, we also need to run "Tuts_Network_Server" at the same time.
- To do this:-
	1. right click on the entire solution (top of the solution exporerer) and go to properties
	2. Go to Common Properties -> Statup Project
	3. Select Multiple Statup Projects
	4. Select 'Start with Debugging' for both "Tuts_Network_Client" and "Tuts_Network_Server"

- Now when you run the program it will build and run both projects at the same time. =]
- You can also optionally spawn more instances by right clicking on the specific project
  and going to Debug->Start New Instance.




This demo scene will demonstrate a very simple network example, with a single server
and multiple clients. The client will attempt to connect to the server, and say "Hellooo!" 
if it successfully connects. The server, will continually broadcast a packet containing a 
Vector3 position to all connected clients informing them where to place the server's player.

This designed as an example of how to setup networked communication between clients, it is
by no means the optimal way of handling a networked game (sending position updates at xhz).
If your interested in this sort of thing, I highly recommend finding a good book or an
online tutorial as there are many many different ways of handling networked game updates
all with varying pitfalls and benefits. In general, the problem always comes down to the
fact that sending updates for every game object 60+ frames a second is just not possible,
so sacrifices and approximations MUST be made. These approximations do result in a sub-optimal
solution however, so most work on networking (that I have found atleast) is on building
a network bespoke communication system that sends the minimal amount of data needed to
produce satisfactory results on the networked peers.


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::: IF YOUR BORED! :::
::::::::::::::::::::::
	1. Try setting up both the server and client within the same Scene (disabling collisions
	on the objects as they now share the same physics engine). This way we can clearly
	see the affect of packet-loss and latency on the network. There is a program called "Clumsy"
	which is found within the root directory of this framework that allows you to inject
	latency/packet loss etc on network. Try messing around with various latency/packet-loss
	values.

	2. Packet Loss
		This causes the object to jump in large (and VERY noticable) gaps from one position to 
		another.

	   A good place to start in compensating for this is to build a buffer and store the
	   last x update packets, now if we miss a packet it isn't too bad as the likelyhood is
	   that by the time we need that position update, we will already have the next position
	   packet which we can use to interpolate that missing data from. The number of packets we
	   will need to backup will be relative to the amount of expected packet loss. This method
	   will also insert additional 'buffer' latency to our system, so we better not make it wait
	   too long.

	3. Latency
	   There is no easy way of solving this, and will have all felt it's punishing effects
	   on all networked games. The way most games attempt to hide any latency is by actually
	   running different games on different machines, these will react instantly to your actions
	   such as shooting which the server will eventually process at some point and tell you if you
	   have hit anything. This makes it appear (client side) like have no latency at all as you
	   moving instantly when you hit forward and shoot when you hit shoot, though this is all smoke
	   and mirrors and the server is still doing all the hard stuff (except now it has to take into account
	   the fact that you shot at time - latency time).

	   This smoke and mirrors approach also leads into another major issue, which is what happens when
	   the instances are desyncrhonised. If player 1 shoots and and player 2 moves at the same time, does
	   player 1 hit player 2? On player 1's screen he/she does, but on player 2's screen he/she gets
	   hit. This leads to issues which the server has to decipher on it's own, this will also happen
	   alot with generic physical elements which will ocasional 'snap' back to it's actual position on 
	   the servers game simulation. This methodology is known as "Dead Reckoning".

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


*//////////////////////////////////////////////////////////////////////////////

#include "Net1_Client.h"

const Vector3 status_color3 = Vector3(1.0f, 0.6f, 0.6f);
const Vector4 status_color = Vector4(status_color3.x, status_color3.y, status_color3.z, 1.0f);


Net1_Client::Net1_Client(const std::string& friendly_name)
	: Scene(friendly_name)
	, serverConnection(NULL)
	, avatar(NULL)
{
}

void Net1_Client::OnInitializeScene()
{

	mParams = new MazeParams();

	mParams->size = 10;
	mParams->density = 0.5f;

	//Initialize Client Network
	if (network.Initialize(0))
	{
		NCLDebug::Log("Network: Initialized!");

		//Attempt to connect to the server on localhost:1234
		serverConnection = network.ConnectPeer(127, 0, 0, 1, 1234);
		NCLDebug::Log("Network: Attempting to connect to server.");
	}

	//Generate Simple Scene with a box that can be updated upon recieving server packets
	avatar = CommonUtils::BuildCuboidObject(
		"Server",
		Vector3(0.0f, 1.0f, 0.0f),
		Vector3(0.1f, 0.3f, 0.1f),
		true,									//Physics Enabled here Purely to make setting position easier via Physics()->SetPosition()
		0.0f,
		false,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f));

	roboGhost = CommonUtils::BuildCuboidObject(
		"Enemy",
		Vector3(0.0f, -20.0f, 0.0f),
		Vector3(0.1f, 0.3f, 0.1f),
		true,									//Physics Enabled here Purely to make setting position easier via Physics()->SetPosition()
		0.0f,
		false,
		false,
		Vector4(1.0f, 0.1f, 0.1f, 0.5f));

	this->AddGameObject(avatar);
	this->AddGameObject(roboGhost);
	waitingForData = false;
}

void Net1_Client::OnCleanupScene()
{
	Scene::OnCleanupScene();
	avatar = NULL; // Deleted in above function

	//Send one final packet telling the server we are disconnecting
	// - We are not waiting to resend this, so if it fails to arrive
	//   the server will have to wait until we time out naturally
	enet_peer_disconnect_now(serverConnection, 0);

	//Release network and all associated data/peer connections
	network.Release();
	serverConnection = NULL;
}

void Net1_Client::OnUpdateScene(float dt)
{
	Scene::OnUpdateScene(dt);
	if (mParams != NULL) {
		//Update parameters of maze to send
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_UP)) {
			if (mParams->density + 0.05 < 1.0f) {
				mParams->density = mParams->density + 0.05f;
			}
			else {
				mParams->density = 1.0f;
			}
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_DOWN)) {
			if (mParams->density - 0.05 > 0.0f) {
				mParams->density = mParams->density - 0.05f;
			}
			else {
				mParams->density = 0.0f;
			}
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_LEFT)) {
			if (mParams->size - 1 >= 5) {
				mParams->size -= 1;
			}
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_RIGHT)) {
			if (mParams->size + 1 <= 25) {
				mParams->size += 1;
			}
		}

		//Build maze
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {
			RequestMaze();
		}
		if (maze != NULL) {
			//Make avatar run maze
			if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2)) {
				RequestAvatar();
			}
			//Show route on ground
			if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3)) {
				RequestRoute();
			}
			//Get random new start and end nodes
			if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_4)) {
				gen.RandomiseStartEndNodes();
				SendTravelNodes();
			}

		}
	}


	//Update Network
	auto callback = std::bind(
		&Net1_Client::ProcessNetworkEvent,	// Function to call
		this,								// Associated class instance
		std::placeholders::_1);				// Where to place the first parameter
	network.ServiceNetwork(dt, callback);



	//Add Debug Information to screen
	uint8_t ip1 = serverConnection->address.host & 0xFF;
	uint8_t ip2 = (serverConnection->address.host >> 8) & 0xFF;
	uint8_t ip3 = (serverConnection->address.host >> 16) & 0xFF;
	uint8_t ip4 = (serverConnection->address.host >> 24) & 0xFF;

	//NCLDebug::DrawTextWs(avatar->Physics()->GetPosition() + Vector3(0.f, 0.6f, 0.f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0.f, 0.f, 0.f, 1.f),
	//	"Peer: %u.%u.%u.%u:%u", ip1, ip2, ip3, ip4, serverConnection->address.port);

	
	NCLDebug::AddStatusEntry(status_color, "Network Traffic");
	NCLDebug::AddStatusEntry(status_color, "    Incoming: %5.2fKbps", network.m_IncomingKb);
	NCLDebug::AddStatusEntry(status_color, "    Outgoing: %5.2fKbps", network.m_OutgoingKb);
	NCLDebug::AddStatusEntry(status_color, "Maze Parameters");
	NCLDebug::AddStatusEntry(status_color, "	Maze Size: %i", mParams->size);
	NCLDebug::AddStatusEntry(status_color, "	MazeDensity: %1.2f", mParams->density);

	if (path != NULL) {
		maze->DrawSearchHistory(path, 0.5f);
	}
}

void Net1_Client::ProcessNetworkEvent(const ENetEvent& evnt)
{
	switch (evnt.type)
	{
	//New connection request or an existing peer accepted our connection request
	case ENET_EVENT_TYPE_CONNECT:
		{
			if (evnt.peer == serverConnection)
			{
				NCLDebug::Log(status_color3, "Network: Successfully connected to server!");
			}	
		}
		break;


	//Server has sent us a new packet
	case ENET_EVENT_TYPE_RECEIVE:
		{

			if (!waitingForData)
			{
				//Get what type of data will be sent
				if (evnt.packet->dataLength == sizeof(ClientPacketType)) {
					ClientPacketType packType;
					memcpy(&packType, evnt.packet->data, sizeof(ClientPacketType));

					nextPacket = packType;
					waitingForData = true;
				}
			}
			else {
				//Updates the maze
				if (nextPacket == MAZE_UPDATE) {

					path = NULL;

					char* mazeStr = (char*)evnt.packet->data;

					gen.ReadFromString(mazeStr);

					SendTravelNodes();

					if (maze != NULL) {
						this->RemoveGameObject(maze);
						delete maze;
					}

					maze = new MazeRenderer(&gen);

					maze->Render()->SetTransform(Matrix4::Scale(Vector3(MAZE_SCALE, 1, MAZE_SCALE)));

					this->AddGameObject(maze);
				}
				//Gets avatar position
				else if (nextPacket == PLAYER_UPDATE) {
					Vector3 pos;
					memcpy(&pos, evnt.packet->data, sizeof(Vector3));
					avatar->Physics()->SetPosition(maze->TranslateToWorld(pos));
				}
				//Get current route
				else if (nextPacket == ROUTE_UPDATE) {
					path = (char*)evnt.packet->data;
				}
				//Send maze to new clients
				else if (nextPacket == CONNECT_MAZE) {
					if (maze == NULL) {
						char* mazeStr = (char*)evnt.packet->data;

						gen.ReadFromString(mazeStr);

						maze = new MazeRenderer(&gen);

						maze->Render()->SetTransform(Matrix4::Scale(Vector3(MAZE_SCALE, 1, MAZE_SCALE)));

						this->AddGameObject(maze);
					}
				}
				//Update enemy position
				else if (nextPacket == ENEMY_UPDATE) {
					Vector3 pos;
					memcpy(&pos, evnt.packet->data, sizeof(Vector3));
					roboGhost->Physics()->SetPosition(maze->TranslateToWorld(pos));
				}
				//Confirms to all clients new start and end nodes
				else if (nextPacket == CONFIRM_NODES) {
					TravelNodes nodes;
					memcpy(&nodes, evnt.packet->data, sizeof(TravelNodes));

					gen.SetTravelNodes(nodes.start, nodes.goal);

					if (maze != NULL) {
						this->RemoveGameObject(maze);
						delete maze;
					}

					maze = new MazeRenderer(&gen);
					maze->Render()->SetTransform(Matrix4::Scale(Vector3(MAZE_SCALE, 1, MAZE_SCALE)));
					this->AddGameObject(maze);

					path = NULL;
				}
				waitingForData = false;
			}
			
		}
		//enet_packet_destroy(evnt.packet);
		break;


	//Server has disconnected
	case ENET_EVENT_TYPE_DISCONNECT:
		{
			NCLDebug::Log(status_color3, "Network: Server has disconnected!");
		}
		break;
	}
}

void Net1_Client::RequestMaze()
{

	if (maze != NULL) {
		this->RemoveGameObject(maze);
	}

	if (path != NULL) {
		path = NULL;
	}

	Instructions* inst = new Instructions(MAZE_PARAMS);

	ENetPacket* packet = enet_packet_create(inst, sizeof(Instructions), 0);
	enet_peer_send(serverConnection, 0, packet);

	packet = enet_packet_create(mParams, sizeof(MazeParams), 0);
	enet_peer_send(serverConnection, 0, packet);

}

//Send request to move avatar
void Net1_Client::RequestAvatar()
{
	Instructions* inst = new Instructions(CREATE_ROUTE);

	ENetPacket* packet = enet_packet_create(inst, sizeof(Instructions), 0);
	enet_peer_send(serverConnection, 0, packet);
}

//Send request to get route through maze
void Net1_Client::RequestRoute()
{
	Instructions* inst = new Instructions(GET_ROUTE);

	ENetPacket* packet = enet_packet_create(inst, sizeof(Instructions), 0);
	enet_peer_send(serverConnection, 0, packet);
}

//Send updated start and end nodes
void Net1_Client::SendTravelNodes()
{
	Instructions* send = new Instructions(TRAVEL_NODES);

	ENetPacket* packet = enet_packet_create(send, sizeof(Instructions), 0);
	enet_peer_send(serverConnection, 0, packet);

	TravelNodes nodes;

	nodes.start = gen.GetStartNode()->_pos;
	nodes.goal = gen.GetGoalNode()->_pos;

	packet = enet_packet_create(&nodes, sizeof(TravelNodes), 0);
	enet_peer_send(serverConnection, 0, packet);
}




