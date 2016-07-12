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

namespace Converter.IO
{
    /// <summary>
    /// Read-only wrapper class for a range within a Stream
    /// </summary>
    sealed class SubStream : Stream
    {
        private Stream stream;
        private long start;
        private long end; // index of the byte after the last

        public SubStream(Stream stream, long start, long size)
        {
            if (!stream.CanRead || !stream.CanSeek)
                throw new ArgumentException("Base stream needs to be able to Read and Seek", nameof(stream));
            if (start < 0 || size < 0 || stream.Length < start + size)
                throw new ArgumentOutOfRangeException("start/size");

            this.stream = stream;
            this.start = start;
            this.end = start + size;
            stream.Seek(start, SeekOrigin.Begin);
        }

        #region Stream
        public override bool CanRead => true;
        public override bool CanSeek => true;
        public override bool CanWrite => false;
        public override void Flush() => stream.Flush();
        public override long Length => end - start;
        public override long Position
        {
            get { return stream.Position - start; }
            set
            {
                if (value < 0 || value >= Length)
                    throw new ArgumentOutOfRangeException();
                stream.Position = value + start;
            }
        }

        public override int Read(byte[] buffer, int offset, int count) =>
            stream.Read(buffer, offset, (int)Math.Min(count, end - Position - 1));

        public override long Seek(long offset, SeekOrigin origin)
        {
            switch(origin)
            {
                case SeekOrigin.Begin:
                    if (offset < 0 || offset >= Length)
                        throw new ArgumentOutOfRangeException(nameof(offset));
                    return stream.Seek(start + offset, origin);

                case SeekOrigin.Current:
                    var newPosition = Position + offset;
                    if (newPosition < 0 || newPosition >= Length)
                        throw new ArgumentOutOfRangeException(nameof(offset));
                    return stream.Seek(offset, origin);

                case SeekOrigin.End:
                    if (offset >= 0 || -offset > Length)
                        throw new ArgumentOutOfRangeException(nameof(offset));
                    return stream.Seek(end + offset, origin);

                default:
                    throw new ArgumentException("Invalid SeekOrigin", nameof(origin));
            }
        }

        public override void SetLength(long value)
        {
            throw new NotImplementedException();
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            throw new InvalidOperationException("Stream is read-only");
        }
        #endregion

        #region IDisposable
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                stream.Dispose();
            }
            base.Dispose(disposing);
        }
        #endregion
    }
}
