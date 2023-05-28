#pragma once

#include "src/Function/World/Entity.h"

namespace Xunlan
{
	class ScriptableEntity
	{
	public:

		explicit ScriptableEntity(const Ref<Entity>& entity) : m_entity(entity) {}
		DISABLE_COPY(ScriptableEntity)
		DISABLE_MOVE(ScriptableEntity)
		virtual ~ScriptableEntity() = default;

	public:

		virtual void Initialize() = 0;
		virtual void OnDestroy() = 0;
		virtual void OnUpdate(float deltaTime) = 0;

	protected:

		Ref<Entity> m_entity;
	};
}