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
#include "memwrite.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdexcept>

void WriteProtectedPtr(void** target, void* data)
{
    DWORD oldProtect;
    if (!VirtualProtect(target, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect))
        throw std::runtime_error("Couldn't unlock page for vtable pointer swap");
    *target = data;
    VirtualProtect(target, sizeof(void*), oldProtect, &oldProtect);
}

void* SwapVPtr(void* object, uint ordinal, void* function)
{
    // Directly references the vtable entry
    void*& vptr = (*(void***)object)[ordinal];

    void* retval = vptr;
    WriteProtectedPtr(&vptr, function);
    return retval;
}
