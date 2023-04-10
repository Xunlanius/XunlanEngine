#pragma once

#include "src/Common/Common.h"
#include "src/Function/Core/Timer.h"

namespace Xunlan
{
    class EngineSystem final
    {
        friend class Singleton<EngineSystem>;

    private:

        EngineSystem() = default;
        DISABLE_COPY(EngineSystem)
        DISABLE_MOVE(EngineSystem)

    public:

        bool Initialize();
        void OnTick();
        void Shutdown();
        bool IsRunning();

    private:

        void OnTickLogical(float deltaTime);
        void OnTickRender(float deltaTime);

    private:

        bool m_running = false;
        Timer m_timer;
    };
}