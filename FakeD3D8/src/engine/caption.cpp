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
#include "caption.h"
#include "display/renderer.h"

using namespace std;
using namespace Display;
using namespace Engine;

StaticCaption::StaticCaption(uint textarea, const void* data, uint)
	:textarea(textarea), text((wchar_t*)data)
{
}

void StaticCaption::Start(DWORD id)
{
    renderer->AddLine(id, textarea, text);
}

void StaticCaption::Stop(DWORD id)
{
    renderer->RemoveLine(id);
}

DynamicCaption::DynamicCaption(uint textarea, const void* data, uint size)
    :textarea(textarea)
{
	uint timecursor = 0;
	union
	{
		const uint* u;
		const byte* b;
	} p {(const uint*)data};

	for (;;)
	{
		uint start = *p.u++;
		if (start == 0xFFFFFFFF)
		{
			break;
		}
		uint end = *p.u++;
		if (start < timecursor)
			throw runtime_error("Overlapping dynamic captions are not implemented");
		if (end <= start)
			throw runtime_error("Negative or zero length caption");

		uint length = *p.u++;

		timings.push_back(make_pair(start, end));
		lines.push_back(wstring((const wchar_t*)p.b));
		
		p.b += length;
		timecursor = end;
	}
}

struct ThreadParam
{
    DynamicCaption* dc;
    DWORD id;
};

void DynamicCaption::Start(DWORD id)
{
    thread = CreateThread(nullptr, 0, &ThreadProc, new ThreadParam{this, id}, 0, nullptr);
    if(!thread)
        throw runtime_error("Couldn't create thread for dynamic caption");
}

void DynamicCaption::Stop(DWORD id)
{
    //printf("DC stop\n");
    DWORD exitCode;
    GetExitCodeThread(thread, &exitCode);
    if(exitCode == STILL_ACTIVE)
    {
        auto emptyAPC = [](ULONG_PTR) { /*printf("In APC\n");*/ }; // MSVC automagically uses __stdcall if needed
        //printf("Fire APC\n");
        QueueUserAPC(emptyAPC, thread, 0);
        WaitForSingleObject(thread, INFINITE);
        //printf("Back from thread\n");
    }
    //printf("Thread has ended\n");
    CloseHandle(thread);

    renderer->RemoveLine(id);
}

DWORD __stdcall DynamicCaption::ThreadProc(void* param)
{
    unique_ptr<ThreadParam> tp((ThreadParam*)param);

    uint id = tp->id;
    const DynamicCaption* This = tp->dc;
    const list<wstring>& lines = tp->dc->lines;
    const list<pair<uint, uint>>& timings = tp->dc->timings;

    auto l = begin(lines);
    auto t = begin(timings);

    //let's go with GetTickCount/Sleep for now, and see
    //if we need something more accurate

    DWORD time0 = GetTickCount();

    struct Aborted {};
    auto waitUntil = [time0](uint time)
    {
        int sleepFor = time + (time0 - GetTickCount());
        if(sleepFor > 0)
        {
            auto s = SleepEx(sleepFor, TRUE);
            if(s == WAIT_IO_COMPLETION)
                throw Aborted();
        }
    };

    try
    {
        for(; l != end(lines); ++l, ++t)
        {
            waitUntil(t->first);
            renderer->AddLine(tp->id, tp->dc->textarea, *l);

            waitUntil(t->second);
            renderer->RemoveLine(tp->id);
        }
    }
    catch(const Aborted&)
    {
        printf("Thread aborted\n");
        return 1;
    }

    return 0;
}
