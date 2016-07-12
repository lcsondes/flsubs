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
#include "caption.h"
#include "utility/subtitlekey.h"

namespace Engine
{

class Subtitles
{
    std::map<Utility::SubtitleKey, std::unique_ptr<Caption>> subtitles;
    std::map<DWORD, Caption*> playing;
public:
    Subtitles();
    Subtitles(Subtitles&&) = delete;

    void Start(DWORD id, const byte* data, uint size);
    void Stop(DWORD id);
};

extern Subtitles* subtitles;

}
