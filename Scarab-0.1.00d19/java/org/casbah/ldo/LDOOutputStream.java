/*-MPL--------------------------------------------------------------------
 * LDOOutputStream.java
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
 * The Original Code is LDOOutputStream.java
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

public class LDOOutputStream implements LDOConstants {    

    protected static final Integer CIRCULAR=new Integer(0);
    protected static final Integer DEFINED=new Integer(1);
    protected BufferedOutputStream out;
    protected int[] streamVersion=new int[2];
    protected byte lastByte;
    protected long reference_id=1;
    protected Hashtable references;
    
    public LDOOutputStream(OutputStream out) throws IOException {
  	this.out=new BufferedOutputStream(out);
	references=new Hashtable();
  	this.out.write(MAGIC);
	this.out.flush();
    }

    public void flush() throws IOException {
	out.flush();
    }

    public synchronized void writeBER(long quantity) throws IOException {
	if (quantity<128) out.write((int)quantity);
	else {
	    long q1=quantity & 0x7f;
	    quantity=quantity>>7;
	    writeBERh(quantity);
	    out.write((int)q1);
	  }
    }
    
    public void writeBERh(long quantity) throws IOException {
	if (quantity<128) out.write((int)(quantity|0x80));
	else {
	    long q1=quantity & 0x7f;
	    quantity=quantity>>7;
	    writeBERh(quantity);
	    out.write((int)(q1|0x80));
	}
    }
    
    public void writeInt(long num) throws IOException{
	synchronized(out) {
	    if (num<0) {
		out.write(INTEGER_N);
		num=Math.abs(num);
	    } else 
		out.write(INTEGER_P);
	    writeBER(num);
	}
    }
    
    public void writeDouble(double d) throws IOException {
	//	System.out.println(d+" "+(long)d);
	boolean adjusted=true;
	int exp=0;
	while (adjusted) {
	    adjusted=false;
	    //	    System.out.println(d+" "+(long)d);
	    if ((long)d!=d) {
		d*=10;
		exp--;
		adjusted=true;
		continue;
	    } 
	    if (((int)d/10)*10==d) {
		d/=10;
		exp++;
		adjusted=true;
	    }
	}
	synchronized(out) {
	    if (d<0) {
		if (exp<0) out.write(FLOAT_NN);
		else out.write(FLOAT_NP);
	    } else {
		if (exp<0) out.write(FLOAT_PN);
		else out.write(FLOAT_PP);
	    }
	}
	writeBER(Math.abs((long)d));
	writeBER(Math.abs(exp));
    }
    	    
    public void writeDict(Hashtable h) throws IOException {
	synchronized(out){ 
	    out.write(DICTIONARY);
	    writeBER(h.size());
	    for (Enumeration enum=h.keys(); enum.hasMoreElements();) {
		Object key=enum.nextElement();
		writeObject(key, false);
		writeObject(h.get(key), false);
	    }
	}
    }
    
    public void writeList(Vector v) throws IOException {
	synchronized(out) {
	    out.write(LIST);
	    int length=v.size();
	    writeBER(length);
	    for (int i=0; i<length; i++) {
		writeObject(v.elementAt(i), false);
	    }
	}
    }
    
    public long writeObjectReference(Object o) throws IOException {
	Long id=(Long)references.get(o);
	if (id!=null) {
	    writeReference(id.longValue());
	    return id.longValue();
	} else {
	    return writeObject(o, true);
	}
    }
    
    public void writeNull() throws IOException {
	out.write(NULL);
    }
    
    public void writeReference(long refid) throws IOException {
	synchronized(out) {
	    out.write(REFERENCE);
	    writeBER(refid);
	}
    }

    public long writeObject(Object o) throws IOException {
	return _writeObject(o, new Hashtable());
    }

    public long _writeObject(Object o, 
			    Hashtable context) throws IOException {
	boolean defined=false;
	if (references.get(o)==null) {
	    out.write(DEFINE_REFERENCE);
	    writeBER(reference_id);
	    references.put(o, new Long(reference_id++));
	    defined=true;
	}
	Long l;
	if (o!=null)
	    context.put(
	if (o==null) {
	    writeNull(); 
	} else if (!defined && (l=(Long)references.get(o))!=null) {
	    writeReference(l.longValue());
	} else if (o instanceof LDOReference) {
	    LDOReference r=(LDOReference)o;
	    if (r.realid==-1) 
		r.realid=writeObjectReference(r.referent);
	    else 
		writeReference(r.realid);
	} else if (o instanceof String) {
	    writeString((String)o);
	} else if (o instanceof byte[]) {
	    writeOpaque((byte[])o);
	} else if (o instanceof Hashtable) {
	    writeDict((Hashtable)o);
	} else if (o instanceof Vector) {
	    writeList((Vector)o);
	} else if (o instanceof Integer) {
	    writeInt(((Integer)o).intValue());
	} else if (o instanceof Long) {
	    writeInt(((Long)o).longValue());
	} else if (o instanceof Double) {
	    writeDouble(((Double)o).doubleValue());
	} else if (o instanceof Float) {
	    writeDouble(((Float)o).floatValue());
	} else if (o instanceof int[]) {
	    int[] oi=(int[])o;
	    for (int i=0; i<oi.length; i++) {
		writeInt(oi[i]);
	    }
	} else if (o instanceof Object[]) {
	    Object[] oo=(Object[])o;
	    for (int i=0; i<oo.length; i++) {
		writeObject(oo[i], false);
	    }
	}
	out.flush();
	return (define?reference_id:0);
    }
    
    
    public void writeString(String s) throws IOException {
	writeOpaque(s.getBytes());
    }
    
    public void writeOpaque(byte[] data) throws IOException {
	synchronized(out) {
	    out.write(OPAQUE);
	    writeBER(data.length);
	    out.write(data);
	}
    }
    
    public static void main(String[] args) throws Exception {
	LDOOutputStream out=new LDOOutputStream(System.out);
	
	Hashtable h=new Hashtable();
	Vector l1=new Vector();
	Vector l2=new Vector();
	Vector l3=new Vector();
	l1.addElement(new Integer(1));
	l1.addElement(new Integer(3));
	l1.addElement(new Integer(5));
	l1.addElement(new Integer(7));
	l1.addElement(new Integer(11));
	l1.addElement(new Integer(13));
	l1.addElement(new Integer(17));

	l2.addElement(new Integer(2));
	l2.addElement(new Integer(4));
	l2.addElement(new Integer(16));
	l2.addElement(new Integer(4096));
	l2.addElement(new Integer(16777216));
	l3.addElement(null);
	h.put("prime sequence", l1);
	h.put("square sequence", l2);
	h.put("list with null object", l3);
	out.writeObject(h, true);
	out.writeObject(h, false);

	out.flush();
	/*	  long start=Calendar.getInstance().getTime().getTime();
		  for (int i=0; i<1000; i++) {
		  out.writeInt(5);
		  }
		  long end=Calendar.getInstance().getTime().getTime();
		  System.err.println("Elapsed: "+(end-start)+" ms");*/
    }

    public void close() throws IOException {
	out.close();
    }
}












