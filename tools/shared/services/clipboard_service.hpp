#pragma once

#include "domain/uid_types.hpp"

namespace symulator::tools
{

class UidClipboardService
{
public:
    void copyUid(UID uid) const;
};

}  // namespace symulator::tools
