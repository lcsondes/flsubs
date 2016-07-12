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

namespace Utility
{

#pragma pack(push, 1)
class SubtitleKey
{
public:
	typedef byte binary_type[6];

private:
    uint crc;
    ushort sizetag;

    SubtitleKey(uint crc, ushort sizetag);
	explicit SubtitleKey(const binary_type);

public:
    bool operator<(const SubtitleKey& o) const;
	void Save(binary_type) const;
	static SubtitleKey Load(const binary_type);
    static SubtitleKey MakeKey(const void* data, uint size);

	std::string Debug() const;
};
#pragma pack(pop)

C_ASSERT(sizeof(SubtitleKey) == 6);


}
