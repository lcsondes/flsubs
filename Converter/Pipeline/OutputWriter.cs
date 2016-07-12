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
using System.Collections.Concurrent;
using System.IO;
using System.Runtime.ExceptionServices;
using System.Threading.Tasks;

namespace Converter.Pipeline
{
    sealed class OutputWriter : IDisposable
    {
        private BlockingCollection<HashedEntry> inputEntries;
        private BinaryWriter binaryWriter;
        private Task task;

        public OutputWriter(BlockingCollection<HashedEntry> inputEntries)
        {
            this.inputEntries = inputEntries;
            binaryWriter = new BinaryWriter(File.Create("flsd.dat"));
        }

        private void Run()
        {
            binaryWriter.Write(new byte[] { (byte)'F', (byte)'L', (byte)'S', (byte)'D' });

            try
            {
                for(;;)
                {
                    try
                    {
                        var entry = inputEntries.Take();
                        binaryWriter.Write(entry.AudioHash);
                        binaryWriter.Write((byte)entry.TextArea);
                        binaryWriter.Write((byte)entry.CaptionType);
                        binaryWriter.WriteWithLength(entry.Content);

                        Console.WriteLine($"Write: {entry.AudioHash.DebugSubtitleKey()} {entry.TextArea} {entry.CaptionType}");
                    }
                    catch (InvalidOperationException)
                    {
                        //inputEntries got completed during Take()
                        break;
                    }
                }
                binaryWriter.Write(new byte[6]); // zero hash to indicate end of file
            }
            finally
            {
                binaryWriter.Close();
            }
        }

        public void Start()
        {
            task = new Task(Run, TaskCreationOptions.LongRunning);
            task.Start();
        }

        public void Wait()
        {
            if (task == null)
                throw new InvalidOperationException();

            try
            {
                task.Wait();
                task = null;
                Console.WriteLine("*** Writer done ***");
            }
            catch (AggregateException e)
            {
                ExceptionDispatchInfo.Capture(e.InnerException).Throw();
            }
        }

        #region IDisposable
        public void Dispose() => binaryWriter.Dispose();
        #endregion
    }
}
