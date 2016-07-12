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
using System.Runtime.InteropServices;
using System.Text;

namespace Converter.IO
{
    sealed class UTFReader
    {
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct UTFHeader
        {
            public uint TreeOffset;
            public uint TreeSize;
            public uint TreeFirstElementOffset;
            public uint TreeEntrySize;
            public uint StringOffset;
            public uint StringSize;
            public uint StringUsed;
            public uint DataOffset;
            public uint DataFirstElementOffset;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct UTFEntry
        {
            public uint SiblingOffset;
            public uint KeyOffset;
            public uint Flags;
            public uint Unknown1;
            public uint DataOffset; //only if(!(Flags & 0x10) || Flags & 0x80)
            public uint Unknown2;
            private uint DataSize1; //data size is the minimum
            private uint DataSize2; //of these two
            public uint Unknown3;
            public uint Unknown4;
            public uint Unknown5;

            public uint DataSize => Math.Min(DataSize1, DataSize2);
        }

        public static unsafe Stream ReadEntry(Stream str, string key)
        {
            byte[] stringtable;
            byte[] tree;
            uint dataStart;

            using (var br = new BinaryReader(str, Encoding.ASCII, leaveOpen: true))
            {
                if (br.ReadUInt32() != 0x20465455u/* FTU*/)
                    throw new FileSanityException("File is not an UTF file");
                if (br.ReadUInt32() != 0x00000101u)
                    throw new FileSanityException("Unknown UTF version");

                UTFHeader hdr;
                var data = br.ReadBytes(sizeof(UTFHeader));
                fixed (byte* p = data)
                {
                    hdr = Marshal.PtrToStructure<UTFHeader>((IntPtr)p);
                }

                br.BaseStream.Seek(hdr.StringOffset, SeekOrigin.Begin);
                stringtable = br.ReadBytes((int)hdr.StringUsed);
                br.BaseStream.Seek(hdr.TreeOffset + hdr.TreeFirstElementOffset, SeekOrigin.Begin);
                tree = br.ReadBytes((int)(hdr.TreeSize - hdr.TreeFirstElementOffset));
                dataStart = hdr.DataOffset;
            }

            fixed (byte* pt = tree)
            {
                for (int i = 0; i < tree.Length; i += sizeof(UTFEntry))
                {
                    UTFEntry entry = Marshal.PtrToStructure<UTFEntry>((IntPtr)(pt + i));

                    //skip tree nodes
                    if ((entry.Flags & 0x10) != 0 && (entry.Flags & 0x80) == 0)
                        continue;

                    fixed (byte* ps = stringtable)
                    {
                        if (key == Marshal.PtrToStringAnsi((IntPtr)(ps + entry.KeyOffset)))
                        {
                            return new SubStream(str, dataStart + entry.DataOffset, entry.DataSize);
                        }
                    }
                }
            }
            throw new FileNotFoundException($"Couldn't find {key}");
        }
    }
}
