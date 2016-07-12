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
using System.Runtime.InteropServices;

namespace Converter.AudioProcessing
{
    static class Native
    {
        [DllImport("d3d8.dll", CharSet = CharSet.Ansi)]
        public static unsafe extern void MakeSubtitleKey(byte* data,
                                                         uint size,
                                                         byte* target);

        #region ACM
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct WAVEFORMATEX
        {
            public ushort wFormatTag;
            public ushort nChannels;
            public uint nSamplesPerSec;
            public uint nAvgBytesPerSec;
            public ushort nBlockAlign;
            public ushort wBitsPerSample;
            public ushort cbSize;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct MPEGLAYER3WAVEFORMAT
        {
            public WAVEFORMATEX wfx;
            public ushort wID;
            public uint fdwFlags;
            public ushort nBlockSize;
            public ushort nFramesPerBlock;
            public ushort nCodecDelay;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public unsafe struct ACMSTREAMHEADER
        {
            public uint cbStruct;
            public uint fdwStatus;
            public uint dwUser;
            public byte* pbSrc;
            public uint cbSrcLength;
            public uint cbSrcLengthUsed;
            public IntPtr dwSrcUser;
            public byte* pbDst;
            public uint cbDstLength;
            public uint cbDstLengthUsed;
            public uint dwDstUser;
            public fixed uint dwReservedDriver[10];
        }

        public enum MMRESULT : uint
        {
            MMSYSERR_NOERROR = 0,
            MMSYSERR_ERROR = 1,
            MMSYSERR_BADDEVICEID = 2,
            MMSYSERR_NOTENABLED = 3,
            MMSYSERR_ALLOCATED = 4,
            MMSYSERR_INVALHANDLE = 5,
            MMSYSERR_NODRIVER = 6,
            MMSYSERR_NOMEM = 7,
            MMSYSERR_NOTSUPPORTED = 8,
            MMSYSERR_BADERRNUM = 9,
            MMSYSERR_INVALFLAG = 10,
            MMSYSERR_INVALPARAM = 11,
            MMSYSERR_HANDLEBUSY = 12,
            MMSYSERR_INVALIDALIAS = 13,
            MMSYSERR_BADDB = 14,
            MMSYSERR_KEYNOTFOUND = 15,
            MMSYSERR_READERROR = 16,
            MMSYSERR_WRITEERROR = 17,
            MMSYSERR_DELETEERROR = 18,
            MMSYSERR_VALNOTFOUND = 19,
            MMSYSERR_NODRIVERCB = 20,
            WAVERR_BADFORMAT = 32,
            WAVERR_STILLPLAYING = 33,
            WAVERR_UNPREPARED = 34
        }

        [DllImport("msacm32.dll")]
        private static extern MMRESULT acmFormatSuggest(IntPtr had,
                                                        ref MPEGLAYER3WAVEFORMAT pwfxSrc,
                                                        ref WAVEFORMATEX pwfxDst,
                                                        uint cbwfxDst,
                                                        uint fdwSuggest);

        [DllImport("msacm32.dll")]
        private static extern MMRESULT acmStreamOpen(out IntPtr phas,
                                                     IntPtr had,
                                                     ref MPEGLAYER3WAVEFORMAT pwfxSrc,
                                                     ref WAVEFORMATEX pwfxDst,
                                                     IntPtr pwfltr,
                                                     uint dwCallback,
                                                     uint dwInstance,
                                                     uint fdwOpen);

        [DllImport("msacm32.dll")]
        private static extern MMRESULT acmStreamSize(IntPtr has,
                                                     uint cbInput,
                                                     out uint pdwOutputBytes,
                                                     uint fdwSize);

        [DllImport("msacm32.dll")]
        private static extern MMRESULT acmStreamPrepareHeader(IntPtr has,
                                                              ref ACMSTREAMHEADER pash,
                                                              uint fdwPrepare);

        [DllImport("msacm32.dll")]
        private static extern MMRESULT acmStreamConvert(IntPtr has,
                                                        ref ACMSTREAMHEADER pash,
                                                        uint fdwConvert);

        [DllImport("msacm32.dll")]
        private static extern MMRESULT acmStreamUnprepareHeader(IntPtr has,
                                                                ref ACMSTREAMHEADER pash,
                                                                uint fdwUnprepare);

        [DllImport("msacm32.dll")]
        private static extern MMRESULT acmStreamClose(IntPtr has,
                                                      uint fdwClose);
        #endregion

        private static void MMErrCheck(MMRESULT mmresult)
        {
            if (mmresult != MMRESULT.MMSYSERR_NOERROR)
                throw new MMException(mmresult);
        }

        #region ACM wrappers
        public static void AcmFormatSuggest(IntPtr had,
                                            ref MPEGLAYER3WAVEFORMAT pwfxSrc,
                                            ref WAVEFORMATEX pwfxDst,
                                            uint cbwfxDst,
                                            uint fdwSuggest)
        => MMErrCheck(acmFormatSuggest(had, ref pwfxSrc, ref pwfxDst, cbwfxDst, fdwSuggest));

        public static void AcmStreamOpen(out IntPtr phas,
                                         IntPtr had,
                                         ref MPEGLAYER3WAVEFORMAT pwfxSrc,
                                         ref WAVEFORMATEX pwfxDst,
                                         IntPtr pwfltr,
                                         uint dwCallback,
                                         uint dwInstance,
                                         uint fdwOpen)
        => MMErrCheck(acmStreamOpen(out phas, had, ref pwfxSrc, ref pwfxDst, pwfltr, dwCallback, dwInstance, fdwOpen));

        public static void AcmStreamSize(IntPtr has,
                                         uint cbInput,
                                         out uint pdwOutputBytes,
                                         uint fdwSize)
        => MMErrCheck(acmStreamSize(has, cbInput, out pdwOutputBytes, fdwSize));

        public static void AcmStreamPrepareHeader(IntPtr has,
                                                  ref ACMSTREAMHEADER pash,
                                                  uint fdwPrepare)
        => MMErrCheck(acmStreamPrepareHeader(has, ref pash, fdwPrepare));

        public static void AcmStreamConvert(IntPtr has,
                                            ref ACMSTREAMHEADER pash,
                                            uint fdwConvert)
        => MMErrCheck(acmStreamConvert(has, ref pash, fdwConvert));

        public static void AcmStreamUnprepareHeader(IntPtr has,
                                                    ref ACMSTREAMHEADER pash,
                                                    uint fdwUnprepare)
        => MMErrCheck(acmStreamUnprepareHeader(has, ref pash, fdwUnprepare));

        public static void AcmStreamClose(IntPtr has,
                                          uint fdwClose)
        => MMErrCheck(acmStreamClose(has, fdwClose));
        #endregion
    }
}
