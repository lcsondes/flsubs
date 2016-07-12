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
#include "textarea.h"

using namespace std;

namespace Display
{

struct Vertex
{
    float x, y, z, rhw, u, v;
    Vertex(float x, float y, float u, float v)
        :x(x), y(y), z(0), rhw(1), u(u), v(v){}
};

TextArea::TextArea(IDirect3DDevice8* dev8, ID3D10Device1* dev10,
                   uint sw, uint sh,
                   uint x, uint y, uint w, uint h,
                   float pt, float dpi,
                   const wstring& fontFamily, DWRITE_TEXT_ALIGNMENT alignment)
    :dev8(dev8), dev10(dev10), changed(true /*for initialization*/), dpim(96.0f/dpi)
{
    rect.left   = x;
    rect.top    = y;
    rect.right  = x + w;
    rect.bottom = y + h;

    auto fcx = [sw](uint x) { return       x / float(sw / 2) - 1.0f; };
    auto fcy = [sh](uint y) { return -(int)y / float(sh / 2) + 1.0f; };

    Vertex vtx[4] = {
        Vertex(fcx(x    ), fcy(y    ), 0, 0),
        Vertex(fcx(x + w), fcy(y    ), 1, 0),
        Vertex(fcx(x    ), fcy(y + h), 0, 1),
        Vertex(fcx(x + w), fcy(y + h), 1, 1)
    };

    CHK(dev8->CreateVertexBuffer(sizeof(vtx),
                                 D3DUSAGE_WRITEONLY,
                                 D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0),
                                 D3DPOOL_MANAGED,
                                 &vb8),
        "Couldn't create DX8 vertex buffer");
    BYTE* data;
    CHK(vb8->Lock(0, sizeof(vtx), &data, D3DLOCK_DISCARD), "");
    memcpy(data, vtx, sizeof(vtx));
    CHK(vb8->Unlock(), "");

    CHK(dev8->CreateTexture(w, h, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tex8),
        "Couldn't create DX8 texture");

    CComPtr<IDWriteFactory> dwFactory;
    CHK(DWriteCreateFactory(DWRITE_FACTORY_TYPE_ISOLATED, __uuidof(IDWriteFactory), (IUnknown**)&dwFactory),
        "Couldn't create DirectWrite factory");
    CComPtr<ID2D1Factory> d2dFactory;
    CHK(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&d2dFactory)),
        "Couldn't create Direct2D factory");

    CHK(dwFactory->CreateTextFormat(fontFamily.c_str(),
                                    nullptr,
                                    DWRITE_FONT_WEIGHT_REGULAR,
                                    DWRITE_FONT_STYLE_NORMAL,
                                    DWRITE_FONT_STRETCH_NORMAL,
                                    pt,
                                    L"en-us",
                                    &fmt),
        "Couldn't create text format. Is the font installed?");
    CHK(fmt->SetTextAlignment(alignment), "");
    CHK(fmt->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR), "");

    D3D10_TEXTURE2D_DESC tdesc = {
        w, h, 1, 1,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        {1, 0},
        D3D10_USAGE_DEFAULT,
        D3D10_BIND_RENDER_TARGET,
        0,
        0
    };
    CHK(dev10->CreateTexture2D(&tdesc, nullptr, &tex10), "");

    tdesc.Usage = D3D10_USAGE_STAGING;
    tdesc.BindFlags = 0;
    tdesc.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
    CHK(dev10->CreateTexture2D(&tdesc, nullptr, &bmp10), "");

    CComPtr<IDXGISurface> surf;
    CHK(tex10->QueryInterface<IDXGISurface>(&surf), "");

    D2D1_RENDER_TARGET_PROPERTIES rprop = {
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        {DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED},
        dpi, dpi,
        D2D1_RENDER_TARGET_USAGE_NONE,
        D2D1_FEATURE_LEVEL_DEFAULT
    };
    CHK(d2dFactory->CreateDxgiSurfaceRenderTarget(surf, rprop, &rt), "");

    // color of Freelancer's letters: #87C3E0 (close to LightSkyBlue)
    CHK(rt->CreateSolidColorBrush(D2D1::ColorF(0x87C3E0),
                                  (ID2D1SolidColorBrush**)&brush),
        "");
}

void TextArea::Update()
{
    changed = false;

    rt->BeginDraw();
    rt->Clear(D2D1::ColorF(0, 0, 0, 0.66f));
    if(lines.empty())
    {
        empty = true;
    }
    else
    {
        empty = false;

        wstring combined;
        set<wstring> lineset;
        for(DWORD id : ordering)
        {
            auto line = lines[id];
            if(lineset.find(line) == end(lineset))
            {
                combined += lines[id] + L"\n";
                lineset.insert(line);
            }
        }

        rt->DrawTextW(combined.c_str(),
                      combined.length(),
                      fmt,
                      D2D1::RectF(0, 0, float(rect.right - rect.left) * dpim, float(rect.bottom - rect.top) * dpim),
                      brush);
    }
    CHK(rt->EndDraw(), "DirectWrite draw failed");

    dev10->CopyResource(bmp10, tex10);

    D3D10_MAPPED_TEXTURE2D data10;
    CHK(bmp10->Map(0, D3D10_MAP_READ, 0, &data10), "");
    D3DLOCKED_RECT data8;
    CHK(tex8->LockRect(0, &data8, nullptr, 0), "");

    int w = rect.right - rect.left;
    //todo openmp
    for(int y = 0; y < (rect.bottom - rect.top); ++y)
    {
        memcpy((char*)data8.pBits + y*data8.Pitch, (char*)data10.pData + y*data10.RowPitch, w * 4/*RGBA8*/);
    }

    CHK(tex8->UnlockRect(0), "");
    bmp10->Unmap(0);
}

void TextArea::AddLine(DWORD id, const std::wstring& str)
{
    RemoveLine(id); //FIXME
    lines.insert(make_pair(id, str));
    ordering.push_front(id);
    changed = true;
}

void TextArea::RemoveLine(DWORD id)
{
    lines.erase(id);
    auto iter = find(begin(ordering), end(ordering), id);
    if(iter != end(ordering))
    {
        ordering.erase(iter);
    }
    changed = true;
}

void TextArea::Render()
{
    if(changed)
    {
        Update();
    }
    if (empty)
    {
        return;
    }

    //blend the texture onto the render target
    CHK(dev8->SetStreamSource(0, vb8, sizeof(Vertex)), "");
    CHK(dev8->SetTexture(0, tex8), "");
    CHK(dev8->BeginScene(), "");
    CHK(dev8->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2), "");
    CHK(dev8->EndScene(), "");
}


// ---------- TextAreaDirect ----------


TextAreaDirect::TextAreaDirect(IDirect3DDevice8* dev8, ID3D10Device1* dev10,
                               uint sw, uint sh,
                               uint x, uint y, uint w, uint h,
                               float pt, float dpi,
                               const wstring& fontFamily, DWRITE_TEXT_ALIGNMENT alignment)
    :TextArea(dev8, dev10, sw, sh, x, y, w, h, pt, dpi, fontFamily, alignment)
{
}

void TextAreaDirect::Render()
{
    if(changed)
    {
        Update();
    }
    if (empty)
    {
        return;
    }

    //memcpy our stuff directly onto the render target
    D3D10_MAPPED_TEXTURE2D data10;
    CHK(bmp10->Map(0, D3D10_MAP_READ, 0, &data10), "");
    D3DLOCKED_RECT data8;
    CComPtr<IDirect3DSurface8> rt;
    CHK(dev8->GetRenderTarget(&rt), "");
    CHK(rt->LockRect(&data8, &rect, 0), "");

    int w = rect.right - rect.left;

    //TODO openmp
    for(int y = 0; y < (rect.bottom - rect.top); ++y)
    {
        memcpy((char*)data8.pBits + y*data8.Pitch, (char*)data10.pData + y*data10.RowPitch, w * 4/*RGBA8*/);
    }

    CHK(rt->UnlockRect(), "");
    bmp10->Unmap(0);
}

}
