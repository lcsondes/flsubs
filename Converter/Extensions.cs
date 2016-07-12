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

using System;
using System.IO;
using System.Text;

namespace Converter
{
    public static class BinaryWriterExtensions
    {
        public static void WriteWithLength(this BinaryWriter bw, byte[] arr)
        {
            bw.Write(arr.Length);
            bw.Write(arr);
        }
    }

    public static class StringExtensions
    {
        public static byte[] ToUTF16(this string str)
        {
            return Encoding.Unicode.GetBytes(str);
        }
    }

    public static class ByteArrayExtensions
    {
        public static string DebugSubtitleKey(this byte[] hash)
        {
            if (hash.Length != 6)
                throw new ArgumentException("Subtitle keys have to be exactly 6 bytes", nameof(hash));

            return $"{hash[3]:X2}{hash[2]:X2}{hash[1]:X2}{hash[0]:X2}-{hash[5]:X2}{hash[4]:X2}";
        }
    }
}
