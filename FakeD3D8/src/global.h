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

#define _CRT_SECURE_NO_WARNINGS
#include <atlbase.h>
#include <cstdio>
#include <cwctype>
#include <d2d1.h>
#include "d3d8.h"
#include <d3d10_1.h>
#include <dsound.h>
#include <dwrite.h>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#define CHK(operation,message) do{                         \
    HRESULT _=(operation);                                 \
    if(FAILED(_)){                                         \
    ::std::printf("%s failed with %08x\n", #operation, _); \
    throw ::std::runtime_error(message);                   \
    }}while(0)

#define SYNCHRONIZED auto _l=_m.Lock()

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;
