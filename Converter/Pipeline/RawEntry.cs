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
using System.Text.RegularExpressions;
using System.Xml;

namespace Converter.Pipeline
{
    sealed class RawEntry
    {
        public string AudioFile { get; private set; }
        public Hack Hack { get; private set; }
        public TextArea TextArea { get; private set; }
        public CaptionType CaptionType { get; private set; }
        public byte[] Content { get; private set; }

        private RawEntry(string audioFile, Hack hack, TextArea textArea, CaptionType captionType, byte[] content)
        {
            AudioFile = audioFile;
            Hack = hack;
            TextArea = textArea;
            CaptionType = captionType;
            Content = content;
        }

        private static uint ParseTime(string str)
        {
            uint time = 0;

            // Patch inputs without a dot:
            // "1" is actually "1.0" and should return 1000
            if (!str.Contains("."))
                str += ".0";

            // Patch fractional digits for integer Parse, e.g. .01 is actually .010
            str += new string('0', 3 - str.Split('.')[1].Length);

            var ss = Regex.Split(str, @"([:.])");
            foreach(var s in ss)
            {
                switch(s)
                {
                    case ":":
                        time *= 60;
                        break;
                    case ".":
                        time *= 1000;
                        break;
                    default:
                        time += uint.Parse(s, CultureInfo.InvariantCulture);
                        break;
                }
            }

            return time;
        }

        private static byte[] AssembleDynamicContent(XmlReader xml)
        {
            using(var bw = new BinaryWriter(new MemoryStream()))
            {
                while(xml.Read())
                {
                    if (!xml.IsStartElement())
                        continue;

                    uint start = ParseTime(xml.GetAttribute("Start"));
                    uint end = ParseTime(xml.GetAttribute("End"));
                    byte[] caption = (xml.ReadElementContentAsString() + "\0").ToUTF16();

                    bw.Write(start);
                    bw.Write(end);
                    bw.WriteWithLength(caption);
                }
                bw.Write(0xFFFFFFFF);

                return ((MemoryStream)bw.BaseStream).ToArray();
            }
        }

        public static RawEntry Read(XmlReader xml)
        {
            xml.Read();
            string audioFile = xml.GetAttribute("AudioFile");
            string hack = xml.GetAttribute("Hack") ?? Hack.None.ToString();
            string textArea = xml.GetAttribute("TextArea");

            Console.WriteLine($"Read: {xml.Name} {audioFile} {hack} {textArea}");

            byte[] content;
            CaptionType type = (CaptionType)Enum.Parse(typeof(CaptionType), xml.Name);
            switch (type)
            {
                case CaptionType.Static:
                    content = (xml.ReadElementContentAsString() + "\0").ToUTF16();
                    break;
                case CaptionType.Dynamic:
                    content = AssembleDynamicContent(xml);
                    break;
                default:
                    //shouldn't happen because of XSD validation
                    throw new XmlException("Invalid subtitle type. How did this get through the XSD?");
            }

            return new RawEntry(audioFile,
                                (Hack)Enum.Parse(typeof(Hack), hack),
                                (TextArea)Enum.Parse(typeof(TextArea), textArea),
                                type,
                                content);
        }
    }
}
