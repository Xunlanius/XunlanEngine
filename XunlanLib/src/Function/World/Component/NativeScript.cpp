#include "NativeScript.h"
#include "src/Function/World/Scene.h"

namespace Xunlan
{
	void ScriptSystem::Create()
	{
		Scene& scene = Scene::Instance();
		ECS::World& world = ECS::World::Instance();
		const auto& entityIDs = world.GetView<ScriptSystem>();

		for (const ECS::EntityID entityID : entityIDs)
		{
			auto [script] = world.GetComponent<NativeScriptComponent>(entityID);
			const Ref<Entity> entity = scene.GetEntity(entityID).lock();
			assert(entity);
			script.m_createFunc(entity);
		}
	}
	void ScriptSystem::Destroy()
	{
		ECS::World& world = ECS::World::Instance();
		const auto& entityIDs = world.GetView<ScriptSystem>();

		for (const ECS::EntityID entityID : entityIDs)
		{
			auto [script] = world.GetComponent<NativeScriptComponent>(entityID);
			script.m_destroyFunc();
		}
	}

	void ScriptSystem::Initialize()
	{
		ECS::World& world = ECS::World::Instance();
		const auto& entityIDs = world.GetView<ScriptSystem>();

		for (const ECS::EntityID entityID : entityIDs)
		{
			auto [script] = world.GetComponent<NativeScriptComponent>(entityID);
			script.m_instance->Initialize();
		}
	}
	void ScriptSystem::Update(float deltaTime)
	{
		ECS::World& world = ECS::World::Instance();
		const auto& entityIDs = world.GetView<ScriptSystem>();

		for (const ECS::EntityID entityID : entityIDs)
		{
			auto [script] = world.GetComponent<NativeScriptComponent>(entityID);
			script.m_instance->OnUpdate(deltaTime);
		}
	}
	void ScriptSystem::OnDestroy()
	{
		ECS::World& world = ECS::World::Instance();
		const auto& entityIDs = world.GetView<ScriptSystem>();

		for (const ECS::EntityID entityID : entityIDs)
		{
			auto [script] = world.GetComponent<NativeScriptComponent>(entityID);
			script.m_instance->OnDestroy();
		}
	}
}

