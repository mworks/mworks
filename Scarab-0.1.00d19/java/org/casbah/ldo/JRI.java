/*-MPL--------------------------------------------------------------------
 * JRI.java
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
 * The Original Code is JRI.java
 *
 * The Initial Developer of the Original Code is Scott G. Miller. Portions
 * created by Scott G. Miller are Copyright (C) 1999 The Casbah Project. All
 * Rights Reserved.
 *
 * Contributor(s):
 */

package org.casbah.ldo;

import java.lang.reflect.*;
import java.util.*;
import org.casbah.util.*;

public class JRI {
    static final String MODIFIERS="modifiers";
    static final String VALUE="value";
    static final String TYPE="type";
    static final String MEMBERS="members";
    static final String DICT="__DICT__";
    static final String ALTTYPE="alt-type";
    static final String ARRAY="__ARRAY__";
    static final String LIST ="__LIST__";
    static final String METHOD="__METHOD__";
    static final String FIELD="__FIELD__";
    static final String PRIVATE_TYPE="__PRIVATE/PROTECTED";
    static final String OBJ_TYPE="__CLASS__";
    static final String UNKNOWN="__UNKNOWN__";
    static final String CLASS="class";
    static int reclevel=0;
    static String[] modstrings=new String[11];

    static {
     	modstrings[lgbase2(Modifier.ABSTRACT)]="abstract";
     	modstrings[lgbase2(Modifier.FINAL)]="final";
     	modstrings[lgbase2(Modifier.INTERFACE)]="interface";
     	modstrings[lgbase2(Modifier.NATIVE)]="native";
     	modstrings[lgbase2(Modifier.PRIVATE)]="private";
     	modstrings[lgbase2(Modifier.PROTECTED)]="protected";
     	modstrings[lgbase2(Modifier.PUBLIC)]="public";
     	modstrings[lgbase2(Modifier.STATIC)]="static";
     	modstrings[lgbase2(Modifier.SYNCHRONIZED)]="synchronized";
     	modstrings[lgbase2(Modifier.TRANSIENT)]="transient";
     	modstrings[lgbase2(Modifier.VOLATILE)]="volatile";
    }

    public static Hashtable introspect(Object o) {
  	String type=null;
  	Hashtable table=new Hashtable(2);
  	if (o instanceof Hashtable) {
  	    table.put(TYPE, DICT);
  	    table.put(VALUE, o);
  	} else if (o instanceof Vector) {
  	    table.put(TYPE, LIST);
  	    table.put(VALUE, o);
  	} else if (o.getClass().isArray()) {
	    table.put(MODIFIERS, modifiers(o.getClass().getModifiers()));
  	    table.put(TYPE, LIST);
  	    table.put(ALTTYPE, ARRAY);
  	    Object[] oa=(Object[])o;
  	    Vector v=new Vector(oa.length);
  	    for (int i=0; i<oa.length; i++) {
  		v.addElement(oa[i]);
  	    }
  	    table.put(VALUE, v);
  	} else {
  	    Field[] fields=o.getClass().getDeclaredFields();
  	    Method[] methods=o.getClass().getDeclaredMethods();
  	    table=new Hashtable(fields.length+methods.length+1);
  	    table.put(TYPE, OBJ_TYPE);

	    table.put(MODIFIERS, modifiers(o.getClass().getModifiers()));
  	    table.put(CLASS, o.getClass().getName());
  	    addMembers(fields, methods, table, o);
  	}
  	return table;
      }

    public static int lgbase2(int modifier) {
	switch (modifier) {
	case 2: return 1;
	case 4: return 2;
	case 8: return 3;
	case 16: return 4;
	case 32: return 5;
	case 64: return 6;
	case 128: return 7;
	case 256: return 8;
	case 512: return 9;
	case 1024: return 10;
	case 2048: return 11;
	}
	return 0;
    }

    public static String describe_mods(Vector modifiers) {
	StringBuffer b=new StringBuffer();
	for (int i=0; i<modifiers.size(); i++) {
	    int x=((Integer)modifiers.elementAt(i)).intValue();
	    b.append(modstrings[lgbase2(x)]);
	    if (i<modifiers.size()) b.append(" ");
	}
	return b.toString();
    }

    public static String describe(Hashtable descriptive, String name) {
	StringBuffer b= new StringBuffer();
	b.append(describe_mods((Vector)descriptive.get(MODIFIERS))).append(" ");
	String type=(String)descriptive.get(TYPE);
	b.append(type).append(" ").append(name).append(" {\n");
	Hashtable members=(Hashtable)descriptive.get(MEMBERS);
	Enumeration enum=members.keys();
	while (enum.hasMoreElements()) {
	    String membername=(String)enum.nextElement();
	    Hashtable member=(Hashtable)members.get(membername);
	    b.append("   ").append(describe_mods((Vector)member.get(MODIFIERS)));
	    b.append(membername);
	    if (((String)member.get(TYPE)).equals(METHOD)) {
		Vector args=(Vector)member.get(VALUE);
		b.append("(");
		for (int i=0; i<args.size(); i++) {
		    b.append((String)args.elementAt(i));
		    if ((i+1)<args.size()) b.append(", ");
		}
		b.append(")");
	    } else if (((String)member.get(TYPE)).equals(FIELD)) {
		Object newval=member.get(VALUE);
		if (newval instanceof Hashtable) {
		    Hashtable h=(Hashtable)newval;
		    Object g=h.get(TYPE);
		    try {
			if (null!=g && !DICT.equals(g)) {
			    newval=construct((Hashtable)newval);
			}
		    } catch (Exception e) {}
		}
		b.append(" = ");
		b.append(newval.toString());
	    }
		
	    b.append(";\n");
	}
	return b.append("}\n").toString();
    }
	
	
      protected static void addMembers(Field[] fields, Method[] methods,
  				    Hashtable dict, Object instance) {
  	Hashtable table=null;
	if (fields.length==0) 
	    table=new Hashtable();
	else 
	    table=new Hashtable(fields.length);
	Hashtable subtable=null;
	for (int i=0; i<fields.length; i++) {
	    subtable=new Hashtable(3);
	    subtable.put(TYPE, FIELD);
	    subtable.put(MODIFIERS, modifiers(fields[i].getModifiers()));
	    try {
		Object oi=fields[i].get(instance);
		if (oi.getClass().getName().substring(0,9).equals("java.lang")) {
		    subtable.put(VALUE, oi);
		} else {
		    //		    System.out.println("introspecting: "+fields[i].get(instance).getClass());
		    subtable.put(VALUE, introspect(oi));
		}
	    } catch (Exception e) {
		subtable.put(VALUE, PRIVATE_TYPE);
	    }
	    table.put(fields[i].getName(), subtable);
	}
	
	for (int i=0; i<methods.length; i++) {
	    Class[] paramTypes=methods[i].getParameterTypes();
	    subtable=new Hashtable(3);
	    subtable.put(TYPE, METHOD);
	    subtable.put(MODIFIERS, modifiers(methods[i].getModifiers()));
	    Vector argProto=new Vector(paramTypes.length);
	    for (int x=0; x<paramTypes.length; x++) {
		argProto.addElement(paramTypes[x].getName());
	    }
	    subtable.put(VALUE, argProto);
	    table.put(methods[i].getName(), subtable);
	}
	dict.put(MEMBERS, table);
    }

    protected static Vector modifiers(int m) {
	Vector v=new Vector(11);
	if (Modifier.isPublic(m)) 
	    v.addElement(new Integer(Modifier.PUBLIC));
	if (Modifier.isPrivate(m)) 
	    v.addElement(new Integer(Modifier.PRIVATE));
	if (Modifier.isProtected(m)) 
	    v.addElement(new Integer(Modifier.PROTECTED));
	if (Modifier.isStatic(m)) 
	    v.addElement(new Integer(Modifier.STATIC));
	if (Modifier.isFinal(m)) 
	    v.addElement(new Integer(Modifier.FINAL));
	if (Modifier.isSynchronized(m)) 
	    v.addElement(new Integer(Modifier.SYNCHRONIZED));
	if (Modifier.isVolatile(m)) 
	    v.addElement(new Integer(Modifier.VOLATILE));
	if (Modifier.isTransient(m)) 
	    v.addElement(new Integer(Modifier.TRANSIENT));
	if (Modifier.isNative(m)) 
	    v.addElement(new Integer(Modifier.NATIVE));
	if (Modifier.isInterface(m)) 
	    v.addElement(new Integer(Modifier.INTERFACE));
	if (Modifier.isAbstract(m)) 
	    v.addElement(new Integer(Modifier.ABSTRACT));
	return v;
    }	

    // Reconstructs a Java object from its description
    public static Object construct(Hashtable description) throws Exception {
	String type=(String)description.get(TYPE);
	String alt_type=(String)description.get(ALTTYPE);
	if (!type.equals(OBJ_TYPE) && (alt_type==null ||
				       !alt_type.equals(ARRAY))) {
	    return description.get(VALUE);
	} else if (alt_type!=null && alt_type.equals(ARRAY)) {
	    Vector v=(Vector)description.get(VALUE);
	    Object[] oa=new Object[v.size()];
	    v.copyInto(oa);
	    return oa;
	} else {
	    String classname=(String)description.get(CLASS);
	    System.out.println(classname);
	    Class clazz=Class.forName(classname);
	    Constructor cons=clazz.getConstructor(new Class[0]);
	    Object o=cons.newInstance(new Object[0]);
	    loadMembers(o, (Hashtable)description.get(MEMBERS));
	    return o;
	}
    }
 
    public static void loadMembers(Object inst, Hashtable memdesc) throws Exception {
	Enumeration names=memdesc.keys();
	Class ofObject=inst.getClass();
	while (names.hasMoreElements()) {
	    String membername=(String)names.nextElement();
	    Hashtable member=(Hashtable)memdesc.get(membername);
	    String type=(String)member.get(TYPE);
	    if (type.equals(FIELD)) {
		Object val=member.get(VALUE);
		if (!(val instanceof String &&
		      PRIVATE_TYPE.equals(val)))
		    {
		    Field f=ofObject.getDeclaredField(membername);
		    Object newval=member.get(VALUE);
		    if (newval instanceof Hashtable) {
			Hashtable h=(Hashtable)newval;
			Object g=h.get(TYPE);
			if (null!=g && !DICT.equals(g)) {
			    newval=construct((Hashtable)newval);
			}
		    }
		    try {
			f.set(inst, newval);
		    } catch (IllegalAccessException e)  {}
		} 
	    }
	}
    }

    public static void main(String args[]) throws Exception {
	//Object o2=new JRI();
	//Object o2=new org.casbah.util.Path();
	Object o2=new org.casbah.qibla.cpif.CPIFRequest(3, new Object[0]);
	Hashtable h=new Hashtable();
	System.out.println("Creating object descriptive...");
	Object o3=introspect(o2);
	System.out.println(Opossum.toString(o3));
	System.out.println(describe((Hashtable)o3, "Path"));
	System.out.println("Using descriptive to create new Object...");
	System.out.println(construct((Hashtable)o3));
    }
      
}
