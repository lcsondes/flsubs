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
#include "main.h"
#include "memwrite.h"
#include "display/config.h"
#include "display/renderer.h"
#include "engine/soundtracker.h"
#include "engine/subtitles.h"

using namespace std;
using namespace Display;
using namespace Engine;

HRESULT(__stdcall *realCreateDevice)(IDirect3D8*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice8**);
ULONG  (__stdcall *realRelease_dev)(IUnknown*);
HRESULT(__stdcall *realPresent)(IDirect3DDevice8*, RECT*, CONST RECT*, HWND, CONST RGNDATA*);
HRESULT(__stdcall *realCreateBuffer)(IDirectSound*, CONST DSBUFFERDESC*, IDirectSoundBuffer**, IUnknown*);
HRESULT(__stdcall *realDuplicate)(IDirectSound*, IDirectSoundBuffer*, IDirectSoundBuffer**);
ULONG  (__stdcall *realRelease_buf)(IUnknown*);
HRESULT(__stdcall *realLock)(IDirectSoundBuffer*, DWORD, DWORD, LPVOID*, LPDWORD, LPVOID*, LPDWORD, DWORD);
HRESULT(__stdcall *realPlay)(IDirectSoundBuffer*, DWORD, DWORD, DWORD);
HRESULT(__stdcall *realUnlock)(IDirectSoundBuffer*, LPVOID, DWORD, LPVOID, DWORD);

//
// IDirectSound
//

HRESULT __stdcall MyDirectSound_CreateSoundBuffer(IDirectSound* This,
                                                  CONST DSBUFFERDESC* pcDSBufferDesc,
                                                  IDirectSoundBuffer** ppDSBuffer,
                                                  IUnknown* pUnkOuter)
{
    IDirectSoundBuffer* buffer;
    HRESULT hr = (*realCreateBuffer)(This, pcDSBufferDesc, &buffer, pUnkOuter);
    *ppDSBuffer = buffer;

    if(FAILED(hr))
    {
        printf("Real CreateSoundBuffer failed (error %08x)\n", hr);
        goto end;
    }

    try
    {
        soundTracker->Create(buffer, pcDSBufferDesc);
    }
    catch(const exception& e)
    {
        printf("Error in SoundTracker::Create\n%s\n", e.what());
    }

end:return hr;
}

HRESULT __stdcall MyDirectSound_DuplicateSoundBuffer(IDirectSound* This,
                                                     IDirectSoundBuffer* pDSBufferOriginal,
                                                     IDirectSoundBuffer** ppDSBufferDuplicate)
{
    HRESULT hr = (*realDuplicate)(This, pDSBufferOriginal, ppDSBufferDuplicate);
    if(SUCCEEDED(hr))
    {
        try
        {
            soundTracker->Duplicate(pDSBufferOriginal, *ppDSBufferDuplicate);
        }
        catch(const exception& e)
        {
            printf("Error in SoundTracker::Duplicate\n%s\n", e.what());
        }
    }
    return hr;
}

//
// IDirectSoundBuffer
//

ULONG __stdcall MyDirectSoundBuffer_Release(IUnknown* This)
{
    ULONG refcnt = This->AddRef();
    if(refcnt == 2)
    {
        try
        {
            soundTracker->LastRelease((IDirectSoundBuffer*)This);
        }
        catch(const exception& e)
        {
            printf("Error in SoundTracker::LastRelease\n%s\n", e.what());
        }
    }
    (*realRelease_buf)(This);

    return (*realRelease_buf)(This);
}

HRESULT __stdcall MyDirectSoundBuffer_Lock(IDirectSoundBuffer* This,
                                           DWORD dwOffset,
                                           DWORD dwBytes,
                                           LPVOID* ppvAudioPtr1,
                                           LPDWORD pdwAudioBytes1,
                                           LPVOID* ppvAudioPtr2,
                                           LPDWORD pdwAudioBytes2,
                                           DWORD dwFlags)
{
    DWORD realOffset;
    DWORD realBytes;

    if(dwFlags & DSBLOCK_FROMWRITECURSOR)
    {
        HRESULT hr = This->GetCurrentPosition(nullptr, &realOffset);
        if(FAILED(hr))
        {
            printf("DirectSoundBuffer_Lock: Couldn't read buffer position\n");
        }
    }
    else
    {
        realOffset = dwOffset;
    }

    if(dwFlags & DSBLOCK_ENTIREBUFFER)
    {
        DSBCAPS caps;
        caps.dwSize = sizeof(DSBCAPS);
        HRESULT hr = This->GetCaps(&caps);
        if(FAILED(hr))
        {
            printf("DirectSoundBuffer_Lock: Couldn't read buffer size\n");
        }
        realBytes = caps.dwBufferBytes;
    }
    else
    {
        realBytes = dwBytes;
    }

    HRESULT hr = (*realLock)(This, dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);
    if(SUCCEEDED(hr))
    {
        try
        {
            soundTracker->Lock(This, realOffset, realBytes, *ppvAudioPtr1);
        }
        catch(const exception& e)
        {
            printf("Error in SoundTracker::Lock\n%s\n", e.what());
        }
    }
    return hr;
}

HRESULT __stdcall MyDirectSoundBuffer_Play(IDirectSoundBuffer* This,
                                           DWORD dwReserved1,
                                           DWORD dwPriority,
                                           DWORD dwFlags)
{
    try
    {
        soundTracker->Play(This); //must come first for perfect timing
    }
    catch(const exception& e)
    {
        printf("Error in SoundTracker::Play\n%s\n", e.what());
    }
    HRESULT hr = (*realPlay)(This, dwReserved1, dwPriority, dwFlags);
    return hr;
}

HRESULT __stdcall MyDirectSoundBuffer_Unlock(IDirectSoundBuffer* This,
                                             LPVOID lpvAudioPtr1,
                                             DWORD dwAudioBytes1,
                                             LPVOID lpvAudioPtr2,
                                             DWORD dwAudioBytes2)
{
    HRESULT hr = (*realUnlock)(This, lpvAudioPtr1, dwAudioBytes1, lpvAudioPtr2, dwAudioBytes2);
    if(SUCCEEDED(hr))
    {
        try
        {
            soundTracker->Unlock(This, lpvAudioPtr1, dwAudioBytes1, lpvAudioPtr2, dwAudioBytes2);
        }
        catch(const exception& e)
        {
            printf("Error in SoundTracker::Unlock\n%s\n", e.what());
        }
    }
    return hr;
}

//
// IDirect3DDevice8
//

HRESULT __stdcall MyDirect3DDevice8_Present(IDirect3DDevice8* This,
                                            RECT* pSourceRect,
                                            CONST RECT* pDestRect,
                                            HWND hDestWindowOverride,
                                            CONST RGNDATA* pDirtyRegion)
{
    try
    {
        renderer->Render();
    }
    catch(const exception& e)
    {
        printf("Error in Renderer::Render\n%s\n", e.what());
    }
    return (*realPresent)(This, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

// This is only overridden in IDirect3DDevice8, not globally in IUnknown
ULONG __stdcall MyDirect3DDevice8_Release(IUnknown* This)
{
    ULONG refcnt = This->AddRef();
    if(refcnt == 2)
    {
        delete config;
        delete renderer;
        delete subtitles;
        delete soundTracker;
    }
    (*realRelease_dev)(This);

    return (*realRelease_dev)(This);
}

//
// IDirect3D8
//

HRESULT __stdcall MyDirect3D8_CreateDevice(IDirect3D8* This,
                                           UINT Adapter,
                                           D3DDEVTYPE DeviceType,
                                           HWND hFocusWindow,
                                           DWORD BehaviorFlags,
                                           D3DPRESENT_PARAMETERS* pPresentationParameters,
                                           IDirect3DDevice8** ppReturnedDeviceInterface)
{
    static bool firstrun = true;

    if(!firstrun || renderer || soundTracker)
    {
        printf("BUG: Multiple D3D8 devices unimplemented\n");
    }
    firstrun = false;

    printf("CreateDevice\n");

    HRESULT hr;
    IDirect3DDevice8* device;
    hr = (*realCreateDevice)(This, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &device);
    *ppReturnedDeviceInterface = device;

    if(FAILED(hr))
    {
        printf("Real CreateDevice failed (error %08x)\n", hr);
        goto end;
    }

    try
    {
        *(void**)&realRelease_dev = SwapVPtr(device, 2, &MyDirect3DDevice8_Release);
        printf("IDirect3DDevice8::Release %p -> %p\n", realRelease_dev, &MyDirect3DDevice8_Release);
        *(void**)&realPresent = SwapVPtr(device, 15, &MyDirect3DDevice8_Present);
        printf("IDirect3DDevice8::Present %p -> %p\n", realPresent, &MyDirect3DDevice8_Present);
    }
    catch(const exception& e)
    {
        printf("Couldn't replace IDirect3DDevice8 functions with own wrappers\n%s\n", e.what());
        goto end;
    }

    try
    {
        config = new Config();
        renderer = new Renderer(device);
        subtitles = new Subtitles();
        soundTracker = new SoundTracker();
    }
    catch(const exception& e)
    {
        printf("Couldn't initialize subtitle system\n%s\n", e.what());
    }

end:return hr;
}

#pragma comment(linker, "/EXPORT:Direct3DCreate8=_MyDirect3DCreate8@4")
extern "C" IDirect3D8* __stdcall MyDirect3DCreate8(UINT SDKVersion)
{
    printf("Direct3DCreate8\n");

    //get real D3D8
    auto realCreate = GetProcAddress(realD3D8Dll, "Direct3DCreate8");

    IDirect3D8* d3d8 = (*(IDirect3D8*(__stdcall*)(UINT))realCreate)(SDKVersion);

    try
    {
        //Swap CreateDevice (virtual #15) with our function
        *(void**)&realCreateDevice = SwapVPtr(d3d8, 15, &MyDirect3D8_CreateDevice);
        printf("IDirect3D8::CreateDevice %p -> %p\n", realCreateDevice, &MyDirect3D8_CreateDevice);
    }
    catch(const exception& e)
    {
        printf("Couldn't replace IDirect3D8::CreateDevice with own wrapper\n%s\n", e.what());
    }

    //hack DirectSound
    IDirectSound* dsound;
    HRESULT hr = DirectSoundCreate(nullptr, &dsound, nullptr);

    if(FAILED(hr))
    {
        printf("Couldn't create DirectSound object\n");
    }

    // Create a sound buffer similar to what Freelancer is using

    WAVEFORMATEX wfex = {
        1,
        1,
        11025,
        22050,
        2,
        16,
        0
    };

    DSBUFFERDESC dsbd = {
        sizeof(DSBUFFERDESC),
        0x400C0,
        DSBSIZE_MIN,
        0,
        &wfex
    };

    IDirectSoundBuffer* dsbuf;
    hr = dsound->CreateSoundBuffer(&dsbd, &dsbuf, nullptr);

    if(FAILED(hr))
    {
        printf("Couldn't create mock DirectSound buffer\n");
    }

    try
    {
        *(void**)&realCreateBuffer = SwapVPtr(dsound, 3, &MyDirectSound_CreateSoundBuffer);
        printf("IDirectSound::CreateSoundBuffer %p -> %p\n", realCreateBuffer, &MyDirectSound_CreateSoundBuffer);
        *(void**)&realDuplicate = SwapVPtr(dsound, 5, &MyDirectSound_DuplicateSoundBuffer);
        printf("IDirectSound::DuplicateSoundBuffer %p -> %p\n", realDuplicate, &MyDirectSound_DuplicateSoundBuffer);

        *(void**)&realRelease_buf = SwapVPtr(dsbuf, 2, &MyDirectSoundBuffer_Release);
        printf("IDirectSoundBuffer::Release %p -> %p\n", realRelease_buf, &MyDirectSoundBuffer_Release);
        *(void**)&realLock = SwapVPtr(dsbuf, 11, &MyDirectSoundBuffer_Lock);
        printf("IDirectSoundBuffer::Lock %p->%p\n", realLock, &MyDirectSoundBuffer_Lock);
        *(void**)&realPlay = SwapVPtr(dsbuf, 12, &MyDirectSoundBuffer_Play);
        printf("IDirectSoundBuffer::Play %p->%p\n", realPlay, &MyDirectSoundBuffer_Play);
        *(void**)&realUnlock = SwapVPtr(dsbuf, 19, &MyDirectSoundBuffer_Unlock);
        printf("IDirectSoundBuffer::Unlock %p->%p\n", realUnlock, &MyDirectSoundBuffer_Unlock);
    }
    catch(const exception& e)
    {
        printf("Couldn't replace IDirectSoundBuffer functions with own wrappers\n%s\n", e.what());
    }

    (*realRelease_buf)(dsbuf);
    dsound->Release();

    return d3d8;
}
