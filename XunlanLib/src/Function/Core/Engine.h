#pragma once

#include "src/Common/Common.h"
#include "src/Function/Core/Timer.h"

namespace Xunlan
{
    class EngineSystem final : public Singleton<EngineSystem>
    {
        friend class Singleton<EngineSystem>;

    private:

        EngineSystem() = default;

    public:

        bool Initialize();
        void Shutdown();
        void Tick();
        bool IsRunning();

    private:

        void TickLogical(float deltaTime);
        void TickRender(float deltaTime);

    private:

        Timer m_timer;
    };
}