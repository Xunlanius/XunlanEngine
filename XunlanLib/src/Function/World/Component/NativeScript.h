#pragma once

#include "src/Function/Script/ScriptableEntity.h"
#include <type_traits>

namespace Xunlan
{
	template<typename T>
	concept NativeScriptConcept = requires
	{
		requires std::is_base_of_v<ScriptableEntity, T>;
		requires std::is_trivially_constructible_v<T>;
		requires std::is_trivially_destructible_v<T>;
	};

	struct NativeScriptComponent
	{
		ScriptableEntity* m_instance = nullptr;

		using ScriptCreateFunc = ScriptableEntity*(*)();
		using ScriptDestroyFunc = void(*)(NativeScriptComponent*);

		ScriptCreateFunc m_scriptCreateFunc;
		ScriptDestroyFunc m_scriptDestroyFunc;

		template<NativeScriptConcept T>
		void Bind()
		{
			m_scriptCreateFunc = []() { return new T(); };
			m_scriptDestroyFunc = [](NativeScriptComponent& script) { delete script.m_instance; script.m_instance = nullptr; };
		}
	};

	struct ScriptSystem : public ECS::ISystem
	{
		void Initialize()
		{
			for (const ECS::EntityID entityID : m_entityIDs)
			{
				auto [script] = m_manager.GetComponent<NativeScriptComponent>(entityID);
				script.m_instance->Initialize();
				std::is_trivial_v<NativeScriptComponent>;
			}
		}

		void OnUpdate(float deltaTime)
		{
			for (const ECS::EntityID entityID : m_entityIDs)
			{
				auto [script] = m_manager.GetComponent<NativeScriptComponent>(entityID);
				script.m_instance->OnUpdate(deltaTime);
			}
		}

		void OnDestroy()
		{
			for (const ECS::EntityID entityID : m_entityIDs)
			{
				auto [script] = m_manager.GetComponent<NativeScriptComponent>(entityID);
				script.m_instance->OnDestroy();
			}
		}
	};
}