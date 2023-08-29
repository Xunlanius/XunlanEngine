#include "MeshRender.h"
#include "Transformer.h"
#include "src/Utility/Math/Math.h"

namespace Xunlan
{
    void MeshRenderSystem::Update()
    {
        ECS::World& world = ECS::World::Instance();
        const auto& entityIDs = world.GetView<MeshRenderSystem>();

        for (const ECS::EntityID entityID : entityIDs)
        {
            auto [transformer, meshRender] = world.GetComponent<TransformerComponent, MeshRenderComponent>(entityID);

            CStruct::PerObject* perObject = (CStruct::PerObject*)meshRender.m_renderItem->GetPerObject()->GetData();
            perObject->m_world = TransformerSystem::GetWorld(transformer);
            perObject->m_invWorld = Math::GetInverse(perObject->m_world);
        }
    }
}