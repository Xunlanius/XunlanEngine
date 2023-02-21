#pragma once

#include "ITest.h"

class TestRenderer : public ITest
{
public:

    virtual bool Initialize() override;
    virtual void Run() override;
    virtual void Shutdown() override;
};