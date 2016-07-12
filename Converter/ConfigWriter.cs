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
using System.Globalization;
using System.IO;
using System.Linq;
using System.Xml;
using System.Xml.Linq;
using System.Xml.Schema;

namespace Converter
{
    // Matches DirectWrite
    enum Alignment
    {
        Leading,
        Trailing,
        Center,
        Justified
    }

    static class ConfigWriter
    {
        private static void ConvertParams(XDocument dom, BinaryWriter bw)
        {
            var screen = dom.Root.Element("Screen");
            bw.Write(uint.Parse(screen.Attribute("Width").Value, CultureInfo.InvariantCulture));
            bw.Write(uint.Parse(screen.Attribute("Height").Value, CultureInfo.InvariantCulture));
            bw.Write(float.Parse(screen.Attribute("DPI").Value, CultureInfo.InvariantCulture));

            foreach(var name in new string[]{"Debug", "Cinematic", "Ingame"})
            {
                var textarea = dom.Root.Descendants(name).First();
                bool enabled = bool.Parse(textarea.Attribute("Enabled").Value);
                bool direct = bool.Parse(textarea.Attribute("Direct").Value);
                bw.Write((byte)((enabled ? 1 : 0) | (direct ? 2 : 0)));
                bw.Write(uint.Parse(textarea.Attribute("X").Value, CultureInfo.InvariantCulture));
                bw.Write(uint.Parse(textarea.Attribute("Y").Value, CultureInfo.InvariantCulture));
                bw.Write(uint.Parse(textarea.Attribute("Width").Value, CultureInfo.InvariantCulture));
                bw.Write(uint.Parse(textarea.Attribute("Height").Value, CultureInfo.InvariantCulture));
                var font = textarea.Element("Font");
                bw.Write(float.Parse(font.Attribute("Size").Value, CultureInfo.InvariantCulture));
                bw.Write((byte)(Alignment)Enum.Parse(typeof(Alignment), font.Attribute("Alignment").Value));
                bw.WriteWithLength((font.Attribute("Family").Value + "\0").ToUTF16());
            }
        }

        public static void Convert(Stream str, Stream xsd)
        {
            var schemaSet = new XmlSchemaSet();
            schemaSet.Add(XmlSchema.Read(xsd, null));

            var settings = new XmlReaderSettings();
            settings.ConformanceLevel = ConformanceLevel.Document;
            settings.ValidationType = ValidationType.Schema;
            settings.Schemas = schemaSet;

            using(var reader = XmlReader.Create(str, settings))
            using(var bw = new BinaryWriter(File.Create("flsc.dat")))
            {
                bw.Write(new byte[] { (byte)'F', (byte)'L', (byte)'S', (byte)'C' });
                ConvertParams(XDocument.Load(reader), bw);
            }
        }
    }
}
