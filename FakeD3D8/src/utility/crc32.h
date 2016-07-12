/*
 * Copyright © 2014, 2015, 2016 László József Csöndes
 *
 * This file is part of FLSubs.
 *
 * FLSubs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FLSubs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLSubs.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "global.h"

namespace Utility
{

class CRC32
{
    static CRC32 _i; //fake instance for static initialization
    static uint* swtable;
    static uint(*fn)(const void*, uint);

    CRC32();
    ~CRC32();
    static void InitTable();
    static uint ComputeSW(const void*, uint);
    static uint ComputeSSE42(const void*, uint);

public:
    static inline uint Compute(const void* data, uint size)
    {
        return (*fn)(data, size);
    }
};

}
