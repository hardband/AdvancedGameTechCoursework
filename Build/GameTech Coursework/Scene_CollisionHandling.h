#include <ncltech\Scene.h>
#include <nclgl\Vector4.h>
#include <ncltech\GraphicsPipeline.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>

#include "RenderNodeParticles.h"
#include "CudaCollidingParticles.cuh"

using namespace CommonUtils;

class Scene_CollisionHandling : public Scene
{
public:
	Scene_CollisionHandling(const std::string& friendly_name)
		: Scene(friendly_name)
	{
	}

	virtual ~Scene_CollisionHandling()
	{

	}


	virtual void OnInitializeScene() override
	{
		//<--- SCENE CREATION --->
		//Create Ground
		this->AddGameObject(BuildCuboidObject("Ground", Vector3(0.0f, -1.0f, 0.0f), Vector3(20.0f, 1.0f, 20.0f), true, 0.0f, true, false, Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

		cudaParticleProg = new CudaCollidingParticles();

		//The dam size (<value> * PARTICLE_RADIUS * 2) must be smaller than the simulation world size!
		cudaParticleProg->InitializeParticleDam(32, 32, 32);

		uint num_particles = cudaParticleProg->GetNumParticles();

		RenderNodeParticles* rnode = new RenderNodeParticles();
		rnode->SetParticleRadius(PARTICLE_RADIUS);
		rnode->SetColor(Vector4(1.f, 0.f, 1.f, 1.f));
		rnode->GeneratePositionBuffer(num_particles, NULL);

		const float half_grid_world_size = PARTICLE_GRID_SIZE * PARTICLE_GRID_CELL_SIZE * 0.5f;
		rnode->SetTransform(Matrix4::Translation(Vector3(-half_grid_world_size, -half_grid_world_size, -half_grid_world_size)));

		//We don't need any game logic, or model matrices, just a means to render our
		// particles to the screen.. so this is just a wrapper  to our actual
		// vertex buffer that holds each particles world position.
		this->AddGameObject(new GameObject("", rnode, NULL));


		cudaParticleProg->InitializeOpenGLVertexBuffer(rnode->GetGLVertexBuffer());

		sphere = BuildSphereObject(
			"",					// Optional: Name
			Vector3(0, 25, 0),				// Position
			BALL_RADIUS,			// Half-Dimensions
			true,				// Physics Enabled?
			4.0f,				// Physical Mass (must have physics enabled)
			true,				// Physically Collidable (has collision shape)
			true,				// Dragable by user?
			Vector4(1, 1, 1, 1));// Render color

		this->AddGameObject(sphere);
	}

	virtual void OnCleanupScene() override
	{
		Scene::OnCleanupScene();
		delete cudaParticleProg;
	}

	virtual void OnUpdateScene(float dt) override
	{
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Info ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "  Example broadphase using cuda thrust library. The thrust library");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "  is the GPU equivalent of the C++ STL and makes things easier ");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "  with vector's, sorting, iterators and array manipulation.");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "  No. Particles: %d", cudaParticleProg->GetNumParticles());
		
		//Does not do what I want, need to work out way to add new particles
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {


			RenderNodeParticles* rnode = new RenderNodeParticles();
			rnode->SetParticleRadius(PARTICLE_RADIUS * 5);
			rnode->SetColor(Vector4(1.f, 0.f, 1.f, 1.f));
			rnode->GeneratePositionBuffer(1, NULL);

			const float half_grid_world_size = PARTICLE_GRID_SIZE * PARTICLE_GRID_CELL_SIZE * 0.5f;
			rnode->SetTransform(Matrix4::Translation(Vector3(half_grid_world_size, 5000, -half_grid_world_size)));
			this->AddGameObject(new GameObject("", rnode, NULL));

		}

		float3 pos;

		pos.x = sphere->Physics()->GetPosition().x + 9.6;
		pos.y = sphere->Physics()->GetPosition().y;
		pos.z = sphere->Physics()->GetPosition().z + 9.6;

		float3 vel;

		vel.x = sphere->Physics()->GetLinearVelocity().x;
		vel.y = sphere->Physics()->GetLinearVelocity().y;
		vel.z = sphere->Physics()->GetLinearVelocity().z;

		cudaParticleProg->UpdateParticles(dt, pos, vel);
	}


protected:
	CudaCollidingParticles* cudaParticleProg;
	GameObject* sphere;
};