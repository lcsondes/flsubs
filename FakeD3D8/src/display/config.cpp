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
#include "config.h"

using namespace std;

namespace Display
{

Config* config = nullptr;

const int Config::NUM_TEXTAREAS;

Config::Config()
{
    FILE* f = fopen("flsc.dat", "rb");
    if (!f)
        throw runtime_error("Couldn't open flsc.dat");

    uint hdr = 0;
    fread(&hdr, 4, 1, f);
    if (hdr != 'CSLF')
        throw runtime_error("flsc.dat is invalid");

    fread(&screenWidth, 4, 1, f);
    fread(&screenHeight, 4, 1, f);
    fread(&dpi, 4, 1, f);

    for (int i = 0; i < NUM_TEXTAREAS; ++i)
    {
        byte flags = (byte)fgetc(f);
        entries[i].enabled = !!(flags & 1);
        entries[i].direct = !!(flags & 2);
        fread(&entries[i].x, 4, 1, f);
        fread(&entries[i].y, 4, 1, f);
        fread(&entries[i].w, 4, 1, f);
        fread(&entries[i].h, 4, 1, f);
        fread(&entries[i].size, 4, 1, f);
        entries[i].alignment = (DWRITE_TEXT_ALIGNMENT)fgetc(f);
        uint length = 0;
        fread(&length, 4, 1, f);
        void* family = alloca(length);
        fread(family, length, 1, f);
        entries[i].family = wstring((wchar_t*)family);
    }

    fclose(f);
}

}
