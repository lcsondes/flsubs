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
#include "renderer.h"
#include "config.h"

using namespace std;

namespace Display
{

Renderer* renderer = nullptr;

Renderer::Renderer(IDirect3DDevice8* dev)
    :dev8(dev), init(false)
{
    CHK(D3D10CreateDevice1(nullptr,
                           D3D10_DRIVER_TYPE_HARDWARE,
                           nullptr,
                           D3D10_CREATE_DEVICE_BGRA_SUPPORT,
                           D3D10_FEATURE_LEVEL_9_1,
                           D3D10_1_SDK_VERSION,
                           &dev10),
        "Couldn't create Direct3D10.1 device");
}

void Renderer::Init()
{
    CComPtr<IDirect3DSurface8> rt;
    CHK(dev8->GetRenderTarget(&rt), "");
    D3DSURFACE_DESC desc;
    CHK(rt->GetDesc(&desc), "");

    float xm = desc.Width / (float)config->screenWidth;
    float ym = desc.Height / (float)config->screenHeight;

    float dpi = config->dpi * ym;

    for (int i = 0; i < Config::NUM_TEXTAREAS; ++i)
    {
        auto& entry = config->entries[i];
        if (!entry.enabled)
        {
            textareas.push_back(make_unique<TextAreaNull>());
        }
        else if (entry.direct)
        {
            textareas.push_back(make_unique<TextAreaDirect>(dev8, dev10, config->screenWidth, config->screenHeight,
                                                            uint(entry.x * xm), uint(entry.y * ym),
                                                            uint(entry.w * xm), uint(entry.h * ym),
                                                            entry.size, dpi,
                                                            entry.family, entry.alignment));
        }
        else
        {
            textareas.push_back(make_unique<TextArea>(dev8, dev10, config->screenWidth, config->screenHeight,
                                                      uint(entry.x * xm), uint(entry.y * ym),
                                                      uint(entry.w * xm), uint(entry.h * ym),
                                                      entry.size, dpi,
                                                      entry.family, entry.alignment));
        }
    }

    init = true;
}

void Renderer::AddLine(DWORD id, uint textarea, const wstring& text)
{
    SYNCHRONIZED;

    RemoveLine(id); //FIXME

    if(textarea >= textareas.size())
    {
        throw std::runtime_error("Trying to add line to undefined text area");
    }
    textareas[textarea]->AddLine(id, text);
    lines.insert(make_pair(id, textarea));
}

void Renderer::RemoveLine(DWORD id)
{
    SYNCHRONIZED;

    auto iter = lines.find(id);
    if(iter != end(lines))
    {
        //AddLine guarantees that all lines are in valid textareas
        textareas[iter->second]->RemoveLine(id);
        lines.erase(iter);
    }
}

void Renderer::Render()
{
    SYNCHRONIZED;
    if(!init)Init();

    if(GetAsyncKeyState(VK_MENU/*alt*/))
        return;

    DWORD magf, cop, ca1, aop, aa1, abe, bop, sb, db;

    //save original state
    CHK(dev8->GetTextureStageState(0, D3DTSS_MAGFILTER, &magf), "");
    CHK(dev8->GetTextureStageState(0, D3DTSS_COLOROP, &cop), "");
    CHK(dev8->GetTextureStageState(0, D3DTSS_COLORARG1, &ca1), "");
    CHK(dev8->GetTextureStageState(0, D3DTSS_ALPHAOP, &aop), "");
    CHK(dev8->GetTextureStageState(0, D3DTSS_ALPHAARG1, &aa1), "");
    CHK(dev8->GetRenderState(D3DRS_ALPHABLENDENABLE, &abe), "");
    CHK(dev8->GetRenderState(D3DRS_BLENDOP, &bop), "");
    CHK(dev8->GetRenderState(D3DRS_SRCBLEND, &sb), "");
    CHK(dev8->GetRenderState(D3DRS_DESTBLEND, &db), "");

    //set state to do alpha blending
    CHK(dev8->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_POINT), "");
    CHK(dev8->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1), "");
    CHK(dev8->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE), "");
    CHK(dev8->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1), "");
    CHK(dev8->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE), "");
    CHK(dev8->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE), "");
    CHK(dev8->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD), "");
    CHK(dev8->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE), ""); //source is premultiplied
    CHK(dev8->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA), "");

    D3DMATRIX tf; ZeroMemory(&tf, sizeof(tf));
    tf._11 = tf._22 = tf._33 = tf._44 = 1.0f;
    CHK(dev8->SetTransform(D3DTS_WORLD, &tf), "");
    CHK(dev8->SetTransform(D3DTS_WORLD1, &tf), "");
    CHK(dev8->SetTransform(D3DTS_WORLD2, &tf), "");
    CHK(dev8->SetTransform(D3DTS_WORLD3, &tf), "");
    CHK(dev8->SetTransform(D3DTS_VIEW, &tf), "");
    CHK(dev8->SetTransform(D3DTS_PROJECTION, &tf), "");

    for(unique_ptr<ITextArea>& textarea : textareas)
    {
        textarea->Render();
    }

    //restore original state
    CHK(dev8->SetTextureStageState(0, D3DTSS_MAGFILTER, magf), "");
    CHK(dev8->SetTextureStageState(0, D3DTSS_COLOROP, cop), "");
    CHK(dev8->SetTextureStageState(0, D3DTSS_COLORARG1, ca1), "");
    CHK(dev8->SetTextureStageState(0, D3DTSS_ALPHAOP, aop), "");
    CHK(dev8->SetTextureStageState(0, D3DTSS_ALPHAARG1, aa1), "");
    CHK(dev8->SetRenderState(D3DRS_ALPHABLENDENABLE, abe), "");
    CHK(dev8->SetRenderState(D3DRS_BLENDOP, bop), "");
    CHK(dev8->SetRenderState(D3DRS_SRCBLEND, sb), "");
    CHK(dev8->SetRenderState(D3DRS_DESTBLEND, db), "");
}

}
