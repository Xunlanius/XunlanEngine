#pragma once

namespace Xunlan
{
    #define DISABLE_COPY(Name)                  \
    Name(const Name& rhs) = delete;             \
    Name& operator=(const Name& rhs) = delete;

    #define DISABLE_MOVE(Name)                  \
    Name(Name&& rhs) = delete;                  \
    Name& operator=(Name&& rhs) = delete;
}