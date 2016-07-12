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

#include "global.h"
#include "crc32.h"

#include <intrin.h>
typedef unsigned char byte;

namespace Utility
{

CRC32 CRC32::_i;
uint* CRC32::swtable = nullptr;
uint(*CRC32::fn)(const void*, uint);

CRC32::CRC32()
{
    int CPUInfo[4];
    __cpuid(CPUInfo, 1);
    if(CPUInfo[2] & (1 << 20/*SSE4.2*/))
    {
        fn = &ComputeSSE42;
    }
    else
    {
        InitTable();
        fn = &ComputeSW;
    }
}

CRC32::~CRC32()
{
    delete[] swtable;
}

void CRC32::InitTable()
{
    static const uint POLY = 0x82f63b78; // reversed 0x1edc6f41

    swtable = new uint[256];
    for(uint i = 0; i < 256; ++i)
    {
        uint value = i;
        for(int j = 0; j < 8; ++j)
        {
            value = value & 1 ? (value >> 1) ^ POLY : value >> 1;
        }
        swtable[i] = value;
    }
}

uint CRC32::ComputeSW(const void* data, uint size)
{
    uint crc = ~0;

    byte* data8 = (byte*)data;
    while(size--)
    {
        crc = (crc >> 8) ^ swtable[(crc ^ *data8++) & 0xff];
    }

    return ~crc;
}

uint CRC32::ComputeSSE42(const void* data, uint size)
{
    uint crc = ~0;

    uint* data32 = (uint*)data;
    uint dwords = size / 4;
    while(dwords--)
    {
        crc = _mm_crc32_u32(crc, *data32++);
    }

    byte* data8 = (byte*)data32; // last few bytes
    uint bytes = size % 4;
    while(bytes--)
    {
        crc = _mm_crc32_u8(crc, *data8++);
    }

    return ~crc;
}

}
