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
#include "utility/mutex.h"

namespace Engine
{

class SoundTracker
{
    Utility::Mutex _m;

    class SoundBuffer
    {
        struct Range
        {
            uint offset;
            uint bytes;
            Range(uint offset, uint bytes) noexcept
                : offset(offset), bytes(bytes)
            {}
        };
        // audioptr1 -> (offset, bytes)
        std::map<const void*, Range> locks;
        std::shared_ptr<std::vector<byte>> data;

    public:
        SoundBuffer(uint);
        SoundBuffer(SoundBuffer*);
        SoundBuffer(SoundBuffer&&) = delete;

        void Lock(const void* ptr, uint offset, uint bytes);
        void Unlock(const void* ptr1, uint bytes1, const void* ptr2, uint bytes2);
        uint Size() const;
        const byte* Data() const;
    };

    std::map<IDirectSoundBuffer*, std::unique_ptr<SoundBuffer>> buffers;
    std::set<IDirectSoundBuffer*> playing;

public:
    SoundTracker() = default;
    SoundTracker(SoundTracker&&) = delete;

#pragma region Thread-safe
    void Create(IDirectSoundBuffer* buf, const DSBUFFERDESC* desc);
    void Duplicate(IDirectSoundBuffer* src, IDirectSoundBuffer* dst);
    void LastRelease(IDirectSoundBuffer* buf);
    void Lock(IDirectSoundBuffer* buf, uint offset, uint bytes, const void* ptr1);
    void Unlock(IDirectSoundBuffer* buf, const void* ptr1, uint bytes1, const void* ptr2, uint bytes2);
    void Play(IDirectSoundBuffer* buf);
#pragma endregion
};

extern SoundTracker* soundTracker;

}
