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
using System.Linq;
using System.Threading;
using System.Xml;
using System.Xml.Schema;

using Converter.Pipeline;

namespace Converter
{
    public static class Program
    {
        public static void Main(string[] args)
        {
            var cfgxsd = typeof(Program).Assembly.GetManifestResourceStream("config.xsd");
            var subxsd = typeof(Program).Assembly.GetManifestResourceStream("subtitles.xsd");

            try
            {
                using (var config = File.OpenRead("subtitles.cfg"))
                {
                    ConfigWriter.Convert(config, cfgxsd);
                }

                using (var rawEntries = new BlockingCollection<RawEntry>())
                using (var hashedEntries = new BlockingCollection<HashedEntry>())
                using (var reader = new InputReader(File.OpenRead("subtitles.xml"), subxsd, rawEntries))
                using (var writer = new OutputWriter(hashedEntries))
                {
                    var converter = new EntryConverter(rawEntries, hashedEntries);

                    writer.Start();
                    converter.Start();

                    reader.Run();

                    converter.Wait();
                    writer.Wait();
                }

                Console.WriteLine();

                for (int i = 5; i >= 1; --i)
                {
                    Console.Write($"\rAll done, quitting in {i}...");
                    Thread.Sleep(1000);
                }
            }
            catch(Exception e)
            {
                Console.WriteLine("*** ERROR ***");

                if(e is XmlException ||
                   e is XmlSchemaValidationException)
                {
                    // Dig through the stack trace to find which xml caused the error
                    // ConfigWriter.Convert : subtitles.cfg
                    // InputReader.Run      : subtitles.xml
                    var ext = e.StackTrace.Split('\n')
                               .Any(l => l.Contains($"{typeof(InputReader).FullName}.{nameof(InputReader.Run)}"))
                               ? "xml" : "cfg";

                    Console.WriteLine($"XML error in subtitles.{ext}:\n{e.Message}");
                }
                else if(e is FileNotFoundException)
                {
                    Console.WriteLine(e.Message);
                }
                else if(e is FileSanityException)
                {
                    var fse = (FileSanityException)e;
                    Console.WriteLine($"{fse.FileName}: {fse.Message}");
                }
                else
                {
                    Console.WriteLine($"{e.GetType().Name}: {e.Message}\n{e.StackTrace}");
                }

                File.Delete("flsc.dat");
                File.Delete("flsd.dat");

                Console.WriteLine("\nPress Enter to continue...");
                Console.ReadLine();
            }
        }
    }
}
