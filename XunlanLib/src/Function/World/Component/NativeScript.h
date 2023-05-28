#pragma once

#include "src/Function/Script/ScriptableEntity.h"
#include <functional>
#include <type_traits>

namespace Xunlan
{
	template<typename T>
	concept NativeScriptConcept = requires
	{
		requires std::is_base_of_v<ScriptableEntity, T>;
	};

	struct NativeScriptComponent
	{
		using CreateFunc = std::function<void(const Ref<Entity>&)>;
		using DestroyFunc = std::function<void()>;

		ScriptableEntity* m_instance = nullptr;

		CreateFunc m_createFunc;
		DestroyFunc m_destroyFunc;
	};

	class ScriptSystem final
	{
	public:

		template<NativeScriptConcept T>
		static void Bind(NativeScriptComponent& script)
		{
			script.m_createFunc = std::bind(
				[](NativeScriptComponent* script, const Ref<Entity>& entity) { script->m_instance = new T(entity); },
				&script, std::placeholders::_1
			);
			script.m_destroyFunc = std::bind(
				[](NativeScriptComponent* script) { delete script->m_instance; script->m_instance = nullptr; },
				&script
			);
		}

		static void Create();
		static void Destroy();

		static void Initialize();
		static void Update(float deltaTime);
		static void OnDestroy();
	};
}