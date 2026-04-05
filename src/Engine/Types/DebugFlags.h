#pragma once
#include <array>
#include <Engine/Types/CommonTypes.h>
#include <Engine/Util/BitwiseMacros.h>

namespace WEngine
{
    struct DebugFlags
    {
    private:
        _GLOBAL_ std::array<uint16, 16> flags;
    public:
        static bool GetFlag(uint8 column, uint8 row)
        {
            uint16 flagColumn = flags[column];
            return CheckBitSet(flagColumn, row);
        }
        static void SetFlag(uint8 column, uint8 row, bool value)
        {
            if (column >= 16 || row >= 16)
                return;

            uint16 flagColumn = flags[column];
            if (value)
                SetBit(&flagColumn, row);
            else
                ClearBit(&flagColumn, row);
            flags[column] = flagColumn;
        }
    };
}
