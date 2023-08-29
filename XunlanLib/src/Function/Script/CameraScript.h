#pragma once

#include "src/Function/World/Entity.h"
#include "src/Function/World/Component/NativeScript.h"
#include "src/Function/Input/InputSystem.h"

namespace Xunlan
{
    class CameraScript final : public ScriptableEntity
    {
    public:

        explicit CameraScript(const Ref<Entity>& entity)
            : ScriptableEntity(entity)
        {
            m_inputHandler.Register(this, &CameraScript::OnMouseMove);
        }
        virtual ~CameraScript() override
        {
            m_inputHandler.Unregister(this, &CameraScript::OnMouseMove);
        }

        virtual void Initialize() override {}
        virtual void OnDestroy() override {}
        virtual void OnUpdate(float deltaTime) override
        {
            InputSystem& inputSystem = InputSystem::Instance();

            if (inputSystem.Get(InputDevice::MOUSE, InputCode::MOUSE_RIGHT).m_curr.x > 0)
            {
                Math::float3 localMoveDir = {};
                localMoveDir.x -= inputSystem.Get(InputDevice::KEYBOARD, InputCode::KEY_A).m_curr.x;
                localMoveDir.x += inputSystem.Get(InputDevice::KEYBOARD, InputCode::KEY_D).m_curr.x;
                localMoveDir.y -= inputSystem.Get(InputDevice::KEYBOARD, InputCode::KEY_Q).m_curr.x;
                localMoveDir.y += inputSystem.Get(InputDevice::KEYBOARD, InputCode::KEY_E).m_curr.x;
                localMoveDir.z += inputSystem.Get(InputDevice::KEYBOARD, InputCode::KEY_W).m_curr.x;
                localMoveDir.z -= inputSystem.Get(InputDevice::KEYBOARD, InputCode::KEY_S).m_curr.x;
                Math::Normalize3(localMoveDir);

                auto [transformer] = m_entity->GetComponent<TransformerComponent>();

                const Math::float3 globalMoveDir = Math::Rotate(localMoveDir, transformer.m_rotation);
                const Math::float3 offset = Math::Mul(globalMoveDir, m_scalarVelocity * deltaTime);

                transformer.m_position = Math::Add(transformer.m_position, offset);
            }
        }

    private:

        void OnMouseMove(InputDevice device, InputCode code, const InputValue& mousePos)
        {
            if (device == InputDevice::MOUSE && code == InputCode::MOUSE_POS)
            {
                const InputValue mouseRight = InputSystem::Instance().Get(InputDevice::MOUSE, InputCode::MOUSE_RIGHT);
                if (mouseRight.m_curr.x == 0.0f) return;

                constexpr float scale = 0.002f;
                const float dx = (mousePos.m_curr.x - mousePos.m_prev.x) * scale;
                const float dy = (mousePos.m_curr.y - mousePos.m_prev.y) * scale;

                auto [transformer] = m_entity->GetComponent<TransformerComponent>();

                TransformerSystem::RotateLocalX(transformer, dy);
                TransformerSystem::RotateGlobalY(transformer, dx);
            }
        }

    private:

        InputHandler<CameraScript> m_inputHandler;

        float m_scalarVelocity = 0.02f;
    };
}