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

using Converter.AudioProcessing;
using Converter.IO;

namespace Converter.Pipeline
{
    sealed class EntryConverter
    {
        private BlockingCollection<RawEntry> inputEntries;
        private BlockingCollection<HashedEntry> outputEntries;
        private Task[] tasks;

        public EntryConverter(BlockingCollection<RawEntry> inputEntries,
                              BlockingCollection<HashedEntry> outputEntries)
        {
            this.inputEntries = inputEntries;
            this.outputEntries = outputEntries;
        }

        private void Run()
        {
            while (!inputEntries.IsCompleted)
            {
                RawEntry entry = null;
                try
                {
                    entry = inputEntries.Take();
                    var parts = entry.AudioFile.Split('|');

                    byte[] hash;
                    Stream str = null;
                    try
                    {
                        str = File.OpenRead(@"..\DATA\AUDIO\" + parts[0]);

                        // If it was a UTF file, grab the entry from it
                        if (parts.Length > 1)
                            str = UTFReader.ReadEntry(str, parts[1]);
                        
                        hash = AudioHasher.Hash(str, entry.Hack);

                        Console.WriteLine($"Convert: {entry.AudioFile} -> {hash.DebugSubtitleKey()}");
                    }
                    finally
                    {
                        str?.Dispose();
                    }

                    outputEntries.Add(new HashedEntry(entry, hash));
                }
                catch(FileSanityException e)
                {
                    e.FileName = entry?.AudioFile;
                    throw;
                }
                catch (InvalidOperationException)
                {
                    //inputEntries got completed during Take()
                    break;
                }
            }
        }

        public void Start()
        {
            tasks = new Task[Environment.ProcessorCount];
            for (int i = 0; i < Environment.ProcessorCount; ++i)
            {
                tasks[i] = new Task(Run, TaskCreationOptions.LongRunning);
                tasks[i].Start();
            }
            Task.WhenAll(tasks).ContinueWith(x => outputEntries.CompleteAdding());
        }

        public void Wait()
        {
            if (tasks == null)
                throw new InvalidOperationException();

            try
            {
                Task.WaitAll(tasks);
                // the continuation might not have run yet, but that's OK
                Console.WriteLine("*** Converter done ***");
            }
            catch(AggregateException e)
            {
                ExceptionDispatchInfo.Capture(e.InnerException).Throw();
            }
        }
    }
}
