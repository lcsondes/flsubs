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
#include "subtitlekey.h"
#include "crc32.h"

using namespace std;

namespace Utility
{

SubtitleKey::SubtitleKey(uint crc, ushort sizetag)
    :crc(crc), sizetag(sizetag)
{
}

SubtitleKey::SubtitleKey(const binary_type input)
{
	memcpy(this, input, 6);
}

bool SubtitleKey::operator<(const SubtitleKey& o) const
{
    return crc < o.crc || (sizetag < o.sizetag && crc == o.crc);
}

void SubtitleKey::Save(binary_type output) const
{
	memcpy(output, this, 6);
}

SubtitleKey SubtitleKey::Load(const binary_type input)
{
	return SubtitleKey(input);
}

SubtitleKey SubtitleKey::MakeKey(const void* data, uint size)
{
    uint crc = CRC32::Compute(data, size);
    ushort sizetag = (size >> 1) & 0xffff;

    return SubtitleKey(crc, sizetag);
}

string SubtitleKey::Debug() const
{
	char buf[8 + 1 + 4 + 1];
	sprintf_s(buf, "%08X-%04X", crc, sizetag);
	return string(buf);
}

}
