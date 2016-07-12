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

class ITextArea
{
protected:
    ITextArea() = default;

public:
    ITextArea(ITextArea&&) = delete;
    virtual ~ITextArea() = default;

    virtual void AddLine(DWORD id, const std::wstring&) = 0;
    virtual void RemoveLine(DWORD id) = 0;
    virtual void Render() = 0;
};

// Null implementation used for disabled textareas
class TextAreaNull : public ITextArea
{
public:
    void AddLine(DWORD id, const std::wstring&) override
    {}

    void RemoveLine(DWORD id) override
    {}

    void Render() override
    {}
};

class TextArea : public ITextArea
{
protected:
    std::map<DWORD, std::wstring> lines;
    std::list<DWORD> ordering; //TODO optimize this

    bool changed; //used to call Update only if needed
    bool empty; //set by Update
    float dpim; // 96.0f/dpi

    IDirect3DDevice8* dev8; // owned by the game
    ID3D10Device1* dev10; // owned by Renderer

    RECT rect;

    //D3D8 resources
    CComPtr<IDirect3DTexture8> tex8;
    CComPtr<IDirect3DVertexBuffer8> vb8;

    //D3D10/D2D/DW resources
    CComPtr<IDWriteTextFormat> fmt;
    CComPtr<ID3D10Texture2D> tex10; //DEFAULT
    CComPtr<ID3D10Texture2D> bmp10; //STAGING
    CComPtr<ID2D1RenderTarget> rt;
    CComPtr<ID2D1Brush> brush;

    void Update();

public:
    TextArea(IDirect3DDevice8* dev8, ID3D10Device1* dev10,
             uint sw, uint sh,
             uint x, uint y, uint w, uint h,
             float pt, float dpi,
             const std::wstring& fontFamily, DWRITE_TEXT_ALIGNMENT alignment);

    void AddLine(DWORD id, const std::wstring&) override;
    void RemoveLine(DWORD id) override;
    void Render() override;
};

/*
 * Hacky version of TextArea that puts its pixels directly onto the render target using the CPU.
 * Necessary to work around the disappearing GPU-drawn text areas during cinematics until
 * a solution is found.
 */
class TextAreaDirect : public TextArea
{
public:
    TextAreaDirect(IDirect3DDevice8* dev8, ID3D10Device1* dev10,
                   uint sw, uint sh,
                   uint x, uint y, uint w, uint h,
                   float pt, float dpi,
                   const std::wstring& fontFamily, DWRITE_TEXT_ALIGNMENT alignment);
    void Render() override;
};

}
