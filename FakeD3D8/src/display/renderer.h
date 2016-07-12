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
#include "textarea.h"
#include "utility/mutex.h"

namespace Display
{

class Renderer
{
    Utility::Mutex _m;

    IDirect3DDevice8* dev8; // owned by the game
    CComPtr<ID3D10Device1> dev10;

    std::vector<std::unique_ptr<ITextArea>> textareas;

    std::map<DWORD, uint> lines;

    bool init;
    void Init();

public:
    Renderer(IDirect3DDevice8*);
    Renderer(Renderer&&) = delete;

#pragma region Thread-safe
    void AddLine(DWORD id, uint textarea, const std::wstring& text);
    void RemoveLine(DWORD id);
    void Render();
#pragma endregion
};

extern Renderer* renderer;

}
