#include <ncltech\PhysicsEngine.h>
#include <ncltech\SceneManager.h>
#include <nclgl\Window.h>
#include <nclgl\NCLDebug.h>
#include <nclgl\PerfTimer.h>

#include <cuda_gl_interop.h>
#include <cuda_runtime.h>

#include "TestScene.h"
#include "EmptyScene.h"
#include "BallScene.h"
#include "Scene_CollisionHandling.h"
#include "TargetScene.h"


#define BALL_SPEED 10

const Vector4 status_colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
const Vector4 status_colour_header = Vector4(0.8f, 0.9f, 1.0f, 1.0f);

bool show_perf_metrics = false;
PerfTimer timer_total, timer_physics, timer_update, timer_render;
uint shadowCycleKey = 4;


// Program Deconstructor
//  - Releases all global components and memory
//  - Optionally prints out an error message and
//    stalls the runtime if requested.
void Quit(bool error = false, const std::string &reason = "") {
	//Release Singletons
	SceneManager::Release();
	PhysicsEngine::Release();
	GraphicsPipeline::Release();
	Window::Destroy();

	//Show console reason before exit
	if (error) {
		std::cout << reason << std::endl;
		system("PAUSE");
		exit(-1);
	}
}


// Program Initialise
//  - Generates all program wide components and enqueues all scenes
//    for the SceneManager to display
void Initialize()
{
	//Initialise the Window
	if (!Window::Initialise("Game Technologies", 1920, 1080, false))
		Quit(true, "Window failed to initialise!");

	//Initialize Renderer
	GraphicsPipeline::Instance();

	//Initialise the PhysicsEngine
	PhysicsEngine::Instance();

	//Enqueue All Scenes
	SceneManager::Instance()->EnqueueScene(new TestScene("GameTech #1 - Framework Sandbox!"));
	SceneManager::Instance()->EnqueueScene(new BallScene("GameTech #2 - Ball Pit"));
	SceneManager::Instance()->EnqueueScene(new TargetScene("GameTech #3 - Targets"));
	SceneManager::Instance()->EnqueueScene(new Scene_CollisionHandling("Cuda Collision - Test/Example"));
}

// Print Debug Info
//  - Prints a list of status entries to the top left
//    hand corner of the screen each frame.
void PrintStatusEntries()
{
	//Print Engine Options
	NCLDebug::AddStatusEntry(status_colour_header, "NCLTech Settings");
	NCLDebug::AddStatusEntry(status_colour, "     Physics Engine: %s (Press P to toggle)", PhysicsEngine::Instance()->IsPaused() ? "Paused  " : "Enabled ");
	NCLDebug::AddStatusEntry(status_colour, "     Monitor V-Sync: %s (Press V to toggle)", GraphicsPipeline::Instance()->GetVsyncEnabled() ? "Enabled " : "Disabled");
	NCLDebug::AddStatusEntry(status_colour, "");

	//Print Current Scene Name
	NCLDebug::AddStatusEntry(status_colour_header, "[%d/%d]: %s",
		SceneManager::Instance()->GetCurrentSceneIndex() + 1,
		SceneManager::Instance()->SceneCount(),
		SceneManager::Instance()->GetCurrentScene()->GetSceneName().c_str()
		);
	NCLDebug::AddStatusEntry(status_colour, "     \x01 T/Y to cycle or R to reload scene");

	//Print Performance Timers
	NCLDebug::AddStatusEntry(status_colour, "     FPS: %5.2f  (Press G for %s info)", 1000.f / timer_total.GetAvg(), show_perf_metrics ? "less" : "more");
	if (show_perf_metrics)
	{
		timer_total.PrintOutputToStatusEntry(status_colour, "          Total Time     :");
		timer_update.PrintOutputToStatusEntry(status_colour, "          Scene Update   :");
		timer_physics.PrintOutputToStatusEntry(status_colour, "          Physics Update :");
		timer_render.PrintOutputToStatusEntry(status_colour, "          Render Scene   :");
	}
	NCLDebug::AddStatusEntry(status_colour, "");
}


// Process Input
//  - Handles all program wide keyboard inputs for
//    things such toggling the physics engine and 
//    cycling through scenes.
void HandleKeyboardInputs()
{
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_P))
		PhysicsEngine::Instance()->SetPaused(!PhysicsEngine::Instance()->IsPaused());

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_V))
		GraphicsPipeline::Instance()->SetVsyncEnabled(!GraphicsPipeline::Instance()->GetVsyncEnabled());

	uint sceneIdx = SceneManager::Instance()->GetCurrentSceneIndex();
	uint sceneMax = SceneManager::Instance()->SceneCount();
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_Y))
		SceneManager::Instance()->JumpToScene((sceneIdx + 1) % sceneMax);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_T))
		SceneManager::Instance()->JumpToScene((sceneIdx == 0 ? sceneMax : sceneIdx) - 1);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_R))
		SceneManager::Instance()->JumpToScene(sceneIdx);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_G))
		show_perf_metrics = !show_perf_metrics;

	//Throw a ball
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J)) {

		Vector3 pos = GraphicsPipeline::Instance()->GetCamera()->GetPosition();

		GameObject* sphere = CommonUtils::BuildSphereObject(
			"thrown",					// Optional: Name
			pos,				// Position
			0.4f,			// Half-Dimensions
			true,				// Physics Enabled?
			30.f,				// Physical Mass (must have physics enabled)
			true,				// Physically Collidable (has collision shape)
			false,				// Dragable by user?
			Vector4(1, 1, 1, 1));// Render color

		Matrix4 viewMatrix = GraphicsPipeline::Instance()->GetCamera()->BuildViewMatrix();
		Vector3 direction = Matrix3::Transpose(Matrix3(viewMatrix)) * Vector3(0,0,-1);

		sphere->Physics()->SetLinearVelocity(direction * BALL_SPEED);

		SceneManager::Instance()->GetCurrentScene()->AddGameObject(sphere);
	}
}


// Program Entry Point
int main()
{
	//Initialize our Window, Physics, Scenes etc
	Initialize();
	GraphicsPipeline::Instance()->SetVsyncEnabled(false);

	Window::GetWindow().GetTimer()->GetTimedMS();

	//Create main game-loop
	while (Window::GetWindow().UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE) && !Window::GetKeyboard()->KeyDown(KEYBOARD_X)) {
		//Start Timing
		
		float dt = Window::GetWindow().GetTimer()->GetTimedMS() * 0.001f;	//How many milliseconds since last update?
																		//Update Performance Timers (Show results every second)
		timer_total.UpdateRealElapsedTime(dt);
		timer_physics.UpdateRealElapsedTime(dt);
		timer_update.UpdateRealElapsedTime(dt);
		timer_render.UpdateRealElapsedTime(dt);

		//Print Status Entries
		PrintStatusEntries();

		//Handle Keyboard Inputs
		HandleKeyboardInputs();

		
		timer_total.BeginTimingSection();

		//Update Scene
		timer_update.BeginTimingSection();
		SceneManager::Instance()->GetCurrentScene()->OnUpdateScene(dt);
		timer_update.EndTimingSection();

		//Update Physics	
		timer_physics.BeginTimingSection();
		PhysicsEngine::Instance()->Update(dt);
		PhysicsEngine::Instance()->DebugRender();
		timer_physics.EndTimingSection();

		//Render Scene
		timer_render.BeginTimingSection();
		GraphicsPipeline::Instance()->UpdateScene(dt);
		GraphicsPipeline::Instance()->RenderScene();
		{
			//Forces synchronisation if vsync is disabled
			// - This is solely to allow accurate estimation of render time
			// - We should NEVER normally lock our render or game loop!		
		//	glClientWaitSync(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, NULL), GL_SYNC_FLUSH_COMMANDS_BIT, 1000000);
		}
		timer_render.EndTimingSection();

		

		//Finish Timing
		timer_total.EndTimingSection();		
	}

	//Cleanup
	Quit();
	return 0;
}