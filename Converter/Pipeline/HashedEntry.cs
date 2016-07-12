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

namespace Converter.Pipeline
{
    sealed class HashedEntry
    {
        public byte[] AudioHash { get; private set; }
        public TextArea TextArea { get; private set; }
        public CaptionType CaptionType { get; private set; }
        public byte[] Content { get; private set; }

        public HashedEntry(RawEntry rawEntry, byte[] hash)
        {
            AudioHash = hash;
            TextArea = rawEntry.TextArea;
            CaptionType = rawEntry.CaptionType;
            Content = rawEntry.Content;
        }
    }
}
