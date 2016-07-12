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
#include "soundtracker.h"
#include "subtitles.h"

using namespace std;

namespace Engine
{

SoundTracker* soundTracker = nullptr;

#undef ERROR
#define ERROR(message) do{printf("%s",message);putchar('\n');}while(0)

SoundTracker::SoundBuffer::SoundBuffer(uint bytes)
    :data(new vector<byte>(bytes, 0))
{
}

SoundTracker::SoundBuffer::SoundBuffer(SoundBuffer* original)
    :data(original->data)
{
}

void SoundTracker::SoundBuffer::Lock(const void* ptr, uint offset, uint bytes)
{
    locks.insert(make_pair(ptr, Range(offset, bytes)));
}

void SoundTracker::SoundBuffer::Unlock(const void* ptr1, uint bytes1, const void* ptr2, uint bytes2)
{
    auto iter = locks.find(ptr1);
    if(iter == end(locks))
    {
        throw std::runtime_error("Unlock without Lock");
    }
    uint offset = iter->second.offset;
    uint bytes = iter->second.bytes;

    memcpy(data->data() + offset, ptr1, bytes1);
    if(ptr2)
        memcpy(data->data(), ptr2, bytes2);

    locks.erase(iter);
}

uint SoundTracker::SoundBuffer::Size() const
{
    return data->size();
}

const byte* SoundTracker::SoundBuffer::Data() const
{
    return data->data();
}

void SoundTracker::Create(IDirectSoundBuffer* buf, const DSBUFFERDESC* desc)
{
    SYNCHRONIZED;

    buffers.insert(make_pair(buf, make_unique<SoundBuffer>(desc->dwBufferBytes)));
    printf("Create %p, length %u\n", buf, desc->dwBufferBytes);
}

void SoundTracker::Duplicate(IDirectSoundBuffer* src, IDirectSoundBuffer* dst)
{
    SYNCHRONIZED;

    auto iter = buffers.find(src);
    if(iter == end(buffers))
    {
        ERROR("Unknown buffer duplicated");
    }
    buffers.insert(make_pair(dst, make_unique<SoundBuffer>(iter->second.get())));
    printf("Duplicate %p->%p (%u bytes)\n", src, dst, iter->second->Size());
}

void SoundTracker::LastRelease(IDirectSoundBuffer* buf)
{
    SYNCHRONIZED;

    buffers.erase(buf);
    auto iter = playing.find(buf);
    if(iter != end(playing))
    {
        subtitles->Stop((DWORD)buf);
        playing.erase(iter);
    }
    printf("LastRelease %p\n", buf);
}

void SoundTracker::Lock(IDirectSoundBuffer* buf, uint offset, uint bytes, const void* pvAudioPtr1)
{
    SYNCHRONIZED;

    auto iter = buffers.find(buf);
    if(iter == end(buffers))
    {
        ERROR("Lock on unknown buffer");
    }
    iter->second->Lock(pvAudioPtr1, offset, bytes);
    //printf("Lock %p, %u bytes @%u\n", buf, bytes, offset);
}

void SoundTracker::Unlock(IDirectSoundBuffer* buf, const void* ptr1, uint bytes1, const void* ptr2, uint bytes2)
{
    SYNCHRONIZED;

    auto iter = buffers.find(buf);
    if(iter == end(buffers))
    {
        ERROR("Unlock on unknown buffer");
    }
    iter->second->Unlock(ptr1, bytes1, ptr2, bytes2);
    //printf("Unlock %p (%u+%u=%u)\n", buf, bytes1, bytes2, bytes1 + bytes2);
}

/**
 * Called before IDirectSoundBuffer::Play, not after (like other functions)
 * @note This means that the write position will match the play position
 *       if the buffer is not already playing.
 */
void SoundTracker::Play(IDirectSoundBuffer* buf)
{
    SYNCHRONIZED;

    auto iter = buffers.find(buf);
    if(iter == end(buffers))
    {
        ERROR("Playing unknown buffer");
    }

    // You'd expect sounds to be played from the beginning,
    // but Freelancer skips the first few (non-constant) bytes.
    // Reading from 0 is more reliable.

    printf("Playing %p (size %u)\n", buf, iter->second->Size());

    playing.insert(buf);
    subtitles->Start((DWORD)buf, iter->second->Data(), iter->second->Size());
}

}
