#include "MeshRenderComponent.h"
#include "Transformer.h"
#include "src/Utility/Math.h"

namespace Xunlan
{
    void MeshRenderSystem::Update()
    {
        for (const ECS::EntityID entityID : m_entityIDs)
        {
            auto [transformer, meshRender] = m_manager.GetComponent<TransformerComponent, MeshRenderComponent>(entityID);

            CBufferPerObject* perObject = (CBufferPerObject*)meshRender.m_renderItem->GetPerObject()->GetData();
            perObject->m_world = GetWorld(transformer);
            perObject->m_invWorld = Math::GetInverse(perObject->m_world);
        }
    }
}