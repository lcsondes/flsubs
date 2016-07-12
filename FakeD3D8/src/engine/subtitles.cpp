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
#include "subtitles.h"
#include "display/config.h"
#include "display/renderer.h"
#include "utility/crc32.h"

using namespace std;
using namespace Display;
using namespace Utility;

namespace Engine
{

Subtitles* subtitles = nullptr;

Subtitles::Subtitles()
{
	C_ASSERT(sizeof(wchar_t) == 2);

    printf("Loading subtitles...\n");

    FILE* f = fopen("flsd.dat", "rb");
    if(!f)
        throw runtime_error("Couldn't open flsd.dat");

	uint hdr = 0;
	fread(&hdr, 4, 1, f);
	if (hdr != 'DSLF')
		throw runtime_error("flsd.dat is invalid");

	for (int i = 0; !feof(f); ++i)
	{
		printf("%d\r", i);
		SubtitleKey::binary_type key;
		fread(key, sizeof(key), 1, f);
		if (count(begin(key), end(key), 0) == sizeof(key))
		{
			//all 0 key signifies end of file
			break;
		}

		byte textarea = (byte)fgetc(f);
		Caption::Type type = (Caption::Type)fgetc(f);

		uint length = 0;
		fread(&length, 4, 1, f);

		void* buf = alloca(length);
		fread(buf, length, 1, f);

		switch (type)
		{
		case Caption::TYPE_STATIC:
			subtitles.insert(make_pair(SubtitleKey::Load(key), make_unique<StaticCaption>(textarea, buf, length)));
			break;
		case Caption::TYPE_DYNAMIC:
			subtitles.insert(make_pair(SubtitleKey::Load(key), make_unique<DynamicCaption>(textarea, buf, length)));
		}
	}
    printf("\nDone!\n");
    fclose(f);
}

void Subtitles::Start(DWORD id, const byte* data, uint size)
{
    auto key = SubtitleKey::MakeKey(data, size);
    auto iter = subtitles.find(key);
    if(iter != end(subtitles))
    {
        playing.insert(make_pair(id, iter->second.get()));
        iter->second->Start(id);
    }
    else
    {
        string str = key.Debug();
        renderer->AddLine(id, 0, wstring(begin(str), end(str)));
    }
}

void Subtitles::Stop(DWORD id)
{
    auto iter = playing.find(id);
    if(iter != end(playing))
    {
        iter->second->Stop(id);
        playing.erase(iter);
    }
    else
    {
        renderer->RemoveLine(id);
    }
}

}
