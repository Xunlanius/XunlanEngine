#pragma once

#include "src/Common/Common.h"
#include "src/Function/Renderer/Abstract/CBuffer.h"
#include "src/Function/World/Entity.h"
#include "src/Function/World/Component/Transformer.h"
#include "src/Utility/Math/Math.h"

namespace Xunlan
{
    class RenderPassBase
    {
    public:

        virtual void ResizeResolution() {}

    protected:

        void UpdateCBufferPerObject(const TransformerComponent& transformer, const Ref<CBuffer>& cBufferPerObject)
        {
            CBufferPerObject* perObject = (CBufferPerObject*)cBufferPerObject->GetData();
            perObject->m_world = TransformerSystem::GetWorld(transformer);
            perObject->m_invWorld = Math::GetInverse(perObject->m_world);
        }
    };
}