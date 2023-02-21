#pragma once

#include "Common/Common.h"
#include "EngineAPI/Entity.h"
#include "EngineAPI/Transformer.h"

class ITest
{
public:

    virtual ~ITest() = 0 {};

    virtual bool Initialize() = 0;
    virtual void Run() = 0;
    virtual void Shutdown() = 0;
};