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

using Converter.AudioProcessing;

namespace Converter
{
    class FileSanityException : ApplicationException
    {
        public FileSanityException(string message = null, Exception innerException = null)
            : base(message, innerException)
        {
        }

        public string FileName { get; internal set; }
    }

    class MMException : ApplicationException
    {
        public MMException(Native.MMRESULT mmresult)
            : base(mmresult.ToString())
        {
        }
    }
}