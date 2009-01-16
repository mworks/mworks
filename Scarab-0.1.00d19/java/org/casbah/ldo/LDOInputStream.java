/*-MPL--------------------------------------------------------------------
 * LDOInputStream.java
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
 * The Original Code is LDOInputStream.java
 *
 * The Initial Developer of the Original Code is Scott G. Miller. Portions
 * created by Scott G. Miller are Copyright (C) 1999 The Casbah Project. All
 * Rights Reserved.
 *
 * Contributor(s):
 */

package org.casbah.ldo;

import java.io.*;
import java.util.*;

public class LDOInputStream implements LDOConstants {    

    protected DataInputStream in;
    protected int[] streamVersion=new int[2];
    protected byte lastByte;
    protected int bc;
    protected Hashtable references, attributes;

    public LDOInputStream(InputStream ins) throws IOException {
	in=new DataInputStream(new BufferedInputStream(ins));
	byte[] rmagic=new byte[MAGIC.length];
	in.readFully(rmagic);
	int mm=0;
	if ((mm=byteComp(MAGIC, 0, rmagic, 0, MAGIC.length))!=-1)
  	    if (mm<4)
  		throw new IOException("LDO MAGIC does not match");
  	    else if (mm==4) 
		throw new IOException("Expected VERSION after MAGIC");
	    else 
		throw new IOException("LDO Version mismatch");
  	references=new Hashtable();
  	attributes=new Hashtable();
 
    }

    public static int byteComp(byte[] source, int sstartpos, byte[] dest,
				   int dstartpos, int len) {
	for (int i=0; i<len; i++) {
	    if (source[sstartpos+i]!=dest[dstartpos+i])
		return i;
	}
	return -1;
    }

    static int[] barray;
    static int bpointer;

    protected byte read() throws IOException {
	bc++;
	return (byte)in.read();
    }
    
    protected Object getReference(long refid) {
	return references.get(new Long(refid));
    }

    protected void putReference(long refid, Object referent) {
	references.put(new Long(refid), referent);
    }

    public long readBER() throws IOException {
	byte b=read();
	long quantity = 0;
	while ((b & 0x80)!=0) { 
       	    quantity=(quantity<<7) + (b & 0x7F);
	    b=read();
	}
	quantity = (quantity<<7) + b;
	return quantity;
    }

    public Vector readList(long refid) throws IOException {
	int length=(int)readBER();
	Vector list=new Vector();
	if (refid!=-1) putReference(refid, list);
	for (;length>0; length--) {
	    list.addElement(readObject());
	}
	return list;
    }

    public Hashtable readDict(long refid) throws IOException {
	int length=(int)readBER();
	Hashtable dict=new Hashtable(length);
	if (refid!=-1) putReference(refid, dict);
	for (; length>0; length--) {
	    Object key=readObject();
	    Object val=readObject();
	    dict.put(key, val);
	}
	return dict;
    }

    public Double readFloat(int sign, int esign) throws IOException {
	long mantissa=sign*readBER();
	double exponent=Math.pow(10, esign*readBER());
	return new Double(mantissa*exponent);
    }

    public Object readObject() throws IOException {
	lastByte=read();
	Object attrib=null;
	long reference=-1;
	if (lastByte==DEFINE_REFERENCE) {
	    reference=readBER();
	    lastByte=read();
	}
	if (lastByte==ATTRIBUTES) {
	    lastByte=read();
	    long ref=0;
	    if (lastByte==REFERENCE) {
		ref=readBER();
	    } else if (lastByte==DEFINE_REFERENCE) {
		ref=readBER();
		readDict(ref);
	    }
	    attrib=getReference(ref);
	    lastByte=read();
	}
        int sign=1;
	int esign=1;
	Object obj=null;
	switch (lastByte) {
	case INTEGER_N:
	    sign=-1;
	case INTEGER_P:
	    obj=new Long(readBER()*sign);
	    break;
	case FLOAT_NN:
	    esign=-1;
      	case FLOAT_NP:
	    sign=-1;
	    obj = readFloat(sign, esign);
	    break;
	case FLOAT_PN:
	    esign=-1;
	case FLOAT_PP:
	    obj = readFloat(sign, esign);
	    break;
	case FLOAT_INF:
	    obj=new Double(Double.POSITIVE_INFINITY);
	    break;
	case FLOAT_NAN:
	    obj=new Double(Double.NaN);
	    break;
	case OPAQUE:
	    int len=(int)readBER();
	    byte[] data=new byte[len];
	    in.readFully(data);
	    bc+=data.length;
	    obj=new String(data);
	    lastByte=OPAQUE;
	    break;
	case DICTIONARY:
	    obj=readDict(reference);
	    reference=-1;
	    break;
	case LIST:
	    obj=readList(reference);
	    reference=-1;
	    break;
	case REFERENCE:
	    obj=getReference(readBER());
	    break;
	case NULL:
	    break;
	case -1:
	    throw new EOFException("End of stream");
	default:
	    throw new IOException("Stream corrupted (unexpected token received at byte:"+bc+")");
	}
	if (reference!=-1) putReference(reference, obj);
	else if (attrib!=null) 
	    attributes.put(obj, attrib);
	return obj;
    }

    public static void main(String[] args) throws Exception {
	LDOInputStream in = new LDOInputStream(System.in);
	while (true) {
	    System.out.println(org.casbah.util.Opossum.toString(in.readObject()));
	}
    }
    
    public void close() throws IOException {
	in.close();
    }
}












