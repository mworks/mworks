/*-MPL--------------------------------------------------------------------
 * LDOConstants.java
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.0 (the "License"); you may not use this file except in compliance with the
 * License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
 * the specific language governing rights and limitations under the License.
 *
 * The Original Code is LDOConstants.java
 *
 * The Initial Developer of the Original Code is Scott G. Miller. Portions
 * created by Scott G. Miller are Copyright (C) 1999 The Casbah Project. All
 * Rights Reserved.
 *
 * Contributor(s):
 */

package org.casbah.ldo;

public interface LDOConstants {
    public int
	VER_MAJOR    = 0x00,
	VER_MINOR    = 0x00,

	VERSION      = 0x01,
	INTEGER_N    = 0x02,
	INTEGER_P    = 0x03,
	FLOAT_NN     = 0x04,
	FLOAT_NP     = 0x05,
	FLOAT_PN     = 0x06,
	FLOAT_PP     = 0x07,
	FLOAT_INF    = 0x08,
	FLOAT_NAN    = 0x09,
	OPAQUE       = 0x0A,
	NULL         = 0x0B,
	LIST         = 0x0C,
	DICTIONARY   = 0x0D,
	DEFINE_REFERENCE 
	             = 0x0E,
	REFERENCE    = 0x0F,
	ATTRIBUTES   = 0x10;

    public byte[] MAGIC = 
    {(byte)0x89, (byte)'C', (byte)'B', (byte)'F',
     (byte)VERSION, (byte)VER_MAJOR, (byte)VER_MINOR};
    
    
}






