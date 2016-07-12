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

namespace Display
{

struct Config
{
    struct Entry
    {
        bool enabled;
        bool direct;
        uint x, y, w, h;
        float size;
        DWRITE_TEXT_ALIGNMENT alignment;
        std::wstring family;
    };
    static const int NUM_TEXTAREAS = 3; // debug, cinematic, ingame

    uint screenWidth;
    uint screenHeight;
    float dpi;

    Entry entries[NUM_TEXTAREAS];

    Config();
    Config(Config&&) = delete;
};

extern Config* config;

}
