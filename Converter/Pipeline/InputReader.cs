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
using System.Xml;
using System.Xml.Schema;

namespace Converter.Pipeline
{
    sealed class InputReader : IDisposable
    {
        private BlockingCollection<RawEntry> outputEntries;
        private XmlReader xmlReader;

        public InputReader(Stream str, Stream xsd, BlockingCollection<RawEntry> outputEntries)
        {
            this.outputEntries = outputEntries;

            var schemaSet = new XmlSchemaSet();
            schemaSet.Add(XmlSchema.Read(xsd, null));

            var settings = new XmlReaderSettings();
            settings.ConformanceLevel = ConformanceLevel.Document;
            settings.ValidationType = ValidationType.Schema;
            settings.Schemas = schemaSet;

            xmlReader = XmlReader.Create(str, settings);
        }

        public void Run()
        {
            xmlReader.MoveToContent();
            while (xmlReader.Read())
            {
                if (!xmlReader.IsStartElement())
                    continue;

                outputEntries.Add(RawEntry.Read(xmlReader.ReadSubtree()));
            }
            outputEntries.CompleteAdding();
            Console.WriteLine("*** Reader done ***");
        }

        #region IDisposable
        public void Dispose() => xmlReader.Dispose();
        #endregion
    }
}
