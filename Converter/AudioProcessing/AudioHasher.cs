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
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace Converter.AudioProcessing
{
    sealed class AudioHasher
    {
        private static void Ensure(bool x, string msg)
        {
            if (!x)
                throw new FileSanityException(msg);
        }

        /// <summary>
        /// Advances the stream to position 12
        /// </summary>
        private static void SanityCheck1(BinaryReader br)
        {
            Ensure(br.BaseStream.Length > 58, "File is empty");
            Ensure(br.ReadUInt32() == 0x46464952u/*FFIR*/, "Not a RIFF file");
            br.ReadBytes(4);
            Ensure(br.ReadUInt32() == 0x45564157u/*EVAW*/, "Not a WAVE file");
        }

        private static unsafe void SanityCheck2(IDictionary<string, byte[]> chunks)
        {
            byte[] fmt = chunks["fmt "];
            Ensure(fmt != null, "Format header missing");
            Ensure(fmt.Length == sizeof(Native.MPEGLAYER3WAVEFORMAT), "Format header has wrong size");
            Ensure(BitConverter.ToUInt16(fmt, 0) == 0x55, "Format header is not MP3");
            Ensure(chunks["trim"] != null, "Trim header missing");
            Ensure(chunks["fact"] != null, "Fact header missing");
            Ensure(chunks["data"] != null, "Data header missing");
        }

        private static unsafe byte[] Convert(byte[] data, Native.MPEGLAYER3WAVEFORMAT fmt, out Native.WAVEFORMATEX dstfmt)
        {
            dstfmt = new Native.WAVEFORMATEX();
            dstfmt.wFormatTag = 1;

            Native.AcmFormatSuggest(IntPtr.Zero, ref fmt, ref dstfmt, 0x12, 0x10000);

            var stream = new IntPtr();
            Native.AcmStreamOpen(out stream, IntPtr.Zero, ref fmt, ref dstfmt, IntPtr.Zero, 0, 0, 4/*nonrealtime*/);

            try
            {
                uint outputSize;
                Native.AcmStreamSize(stream, (uint)data.Length, out outputSize, 0);

                var pcm = new byte[outputSize];

                fixed (byte* pSrc = data)
                fixed (byte* pDst = pcm)
                {
                    var hdr = new Native.ACMSTREAMHEADER();
                    hdr.cbStruct = (uint)sizeof(Native.ACMSTREAMHEADER);
                    hdr.pbSrc = pSrc;
                    hdr.cbSrcLength = (uint)data.Length;
                    hdr.pbDst = pDst;
                    hdr.cbDstLength = outputSize;

                    Native.AcmStreamPrepareHeader(stream, ref hdr, 0);
                    Native.AcmStreamConvert(stream, ref hdr, 0x30/*start+end*/);
                    Native.AcmStreamUnprepareHeader(stream, ref hdr, 0);
                }

                return pcm;
            }
            finally
            {
                Native.AcmStreamClose(stream, 0);
            }
        }

        /// <summary>
        /// Assumes that the base stream is at position 12
        /// </summary>
        private static IDictionary<string, byte[]> GetChunks(BinaryReader br)
        {
            System.Diagnostics.Debug.Assert(br.BaseStream.Position == 12);

            var chunks = new Dictionary<string, byte[]>();

            while(br.BaseStream.Position != br.BaseStream.Length)
            {
                string name = new string(Encoding.ASCII.GetChars(br.ReadBytes(4)));
                uint size = br.ReadUInt32();
                chunks.Add(name, br.ReadBytes((int)size));
            }

            return chunks;
        }

        public static unsafe byte[] Hash(Stream str, Hack hack)
        {
            IDictionary<string, byte[]> chunks;
            using (var br = new BinaryReader(str))
            {
                SanityCheck1(br);
                chunks = GetChunks(br);
            }
            SanityCheck2(chunks);

            Native.MPEGLAYER3WAVEFORMAT srcfmt;
            Native.WAVEFORMATEX dstfmt;
            byte[] pcm;
            fixed (byte* p = chunks["fmt "])
            {
                srcfmt = Marshal.PtrToStructure<Native.MPEGLAYER3WAVEFORMAT>((IntPtr)p);
                pcm = Convert(chunks["data"], srcfmt, out dstfmt);
            }

            uint trim = BitConverter.ToUInt32(chunks["trim"], 0) * dstfmt.nChannels * (dstfmt.wBitsPerSample / 8u);
            uint fact = BitConverter.ToUInt32(chunks["fact"], 0) * dstfmt.nChannels * (dstfmt.wBitsPerSample / 8u);

            if(hack == Hack.Streamed)
            {
                //Load 80860 bytes into a 89856 wide buffer padded with 0 and hash that
                if (fact < 80860 ||
                    dstfmt.nSamplesPerSec != 22050 ||
                    dstfmt.nChannels != 2)
                {
                    throw new NotImplementedException("Stream hack is not good enough");
                }
                var pcm2 = new byte[89856];
                Array.Copy(pcm, trim, pcm2, 0, 80860);

                trim = 0;
                fact = 89856;
                pcm = pcm2;
            }

            Ensure(pcm.Length >= trim + fact, "Decoded PCM is too short for trimming");
            var hash = new byte[6];
            fixed (byte* p = pcm)
            fixed (byte* h = hash)
            {
                Native.MakeSubtitleKey(p + trim, fact, h);
            }
            return hash;
        }
    }
}
