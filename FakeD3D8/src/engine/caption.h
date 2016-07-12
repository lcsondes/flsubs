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

namespace Engine
{

class Caption
{
public:
	enum Type
	{
		TYPE_STATIC,
		TYPE_DYNAMIC
	};

    Caption() = default;
    Caption(Caption&&) = delete;
    virtual ~Caption() = default;

    virtual void Start(DWORD id) = 0;
    virtual void Stop(DWORD id) = 0;
};

class StaticCaption : public Caption
{
    uint textarea;
    std::wstring text;

public:
	StaticCaption(uint textarea, const void* data, uint size);
    void Start(DWORD id) override;
    void Stop(DWORD id) override;
};

class DynamicCaption : public Caption
{
    HANDLE thread;

    uint textarea;
	std::list<std::pair<uint, uint>> timings;
	std::list<std::wstring> lines;

    static DWORD __stdcall ThreadProc(void* param);

public:
    DynamicCaption(uint textarea, const void* data, uint size);
    void Start(DWORD id) override;
    void Stop(DWORD id) override;
};

}
