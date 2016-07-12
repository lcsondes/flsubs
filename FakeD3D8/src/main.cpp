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

#include <shlobj.h>

using namespace std;

HMODULE realD3D8Dll;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    /* It's unsafe to call LoadLibrary/FreeLibrary from DllMain,
     * but our dependency graph is a tree, so let's live dangerously */

    switch(fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        printf("Fake D3D8 attached\n");
        {
            wchar_t* system32;
            SHGetKnownFolderPath(FOLDERID_SystemX86, 0, nullptr, &system32);
            wstring d3d8path = wstring(system32) + L"\\d3d8.dll";
            CoTaskMemFree(system32);
            wprintf(L"Loading real D3D8 from %s\n", d3d8path.c_str());
            realD3D8Dll = LoadLibraryW(d3d8path.c_str());
        }
        if(realD3D8Dll)
        {
            printf("Real D3D8 loaded\n");
        }
        else
        {
            printf("Couldn't load real D3D8!\n");
            return false;
        }
        break;
    case DLL_PROCESS_DETACH:
        if(FreeLibrary(realD3D8Dll))
        {
            printf("Real D3D8 unloaded\n");
        }
        printf("Fake D3D8 detached\n");
        //prevent crash
        TerminateProcess(GetCurrentProcess(), 0);
        break;
    }
    return TRUE;
}
