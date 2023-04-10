#pragma once

#include "src/Function/World/ECS/ECS.h"

namespace Xunlan
{
	class ScriptableEntity
	{
	public:

		ScriptableEntity() = default;
		virtual ~ScriptableEntity() = default;

		template<typename... Args>
		inline std::tuple<Args&...> GetComponent() { return Singleton<ECS::ECSManager>::Instance().GetComponent<Args...>(m_entityID); }

		virtual void Initialize() = 0;
		virtual void OnDestroy() = 0;
		virtual void OnUpdate(float deltaTime) = 0;

	private:

		ECS::EntityID m_entityID;
	};
}