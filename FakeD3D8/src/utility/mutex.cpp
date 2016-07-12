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
#include "mutex.h"

namespace Utility
{

Mutex::Locker::Locker(CRITICAL_SECTION* cs)
    :cs(cs)
{
    EnterCriticalSection(cs);
}

Mutex::Locker::Locker(Locker&& o)
    :cs(o.cs)
{
    o.cs = nullptr;
}

Mutex::Locker::~Locker()
{
    if(cs)
        LeaveCriticalSection(cs);
}

Mutex::Mutex()
{
    InitializeCriticalSection(&cs);
}

Mutex::~Mutex()
{
    DeleteCriticalSection(&cs);
}

Mutex::Locker Mutex::Lock()
{
    return Locker(&cs);
}

}
