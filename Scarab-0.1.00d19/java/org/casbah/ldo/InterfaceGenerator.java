package org.casbah.ldo;

import java.lang.reflect.*;
import java.util.*;

/**
 * Generates LDO L2 compatible interface definitions of any Java Object
 */
public class InterfaceGenerator implements L2Constants {
    private static int refid;

    public static Object generateInterface(Object job, Hashtable marks) {
	Class ofJob=job.getClass();
	return generateInterface(ofJob, marks);
    }

    public static Object generateInterface(Class ofJob, Hashtable marks) {
	if (marks==null) marks=new Hashtable();
	LDOReference ref=null;
	if (marks.get(ofJob)!=null) 
	    return marks.get(ofJob);
	else 
	    marks.put(ofJob, ref=new LDOReference(refid++));

	Hashtable ointerface=new Hashtable(3);
	ointerface.put(CLASSNAME, ofJob.getName());

	Method[] methods=ofJob.getDeclaredMethods();
	Vector instanceMethods=new Vector();
	Vector staticMethods  =new Vector();
	
	for (int i=0; i<methods.length; i++) {
	    int modifiers=methods[i].getModifiers();
	    if ((modifiers & Modifier.PUBLIC)!=0) {
		//Method is public, we can export it
		Hashtable methodDef=new Hashtable(3);
		
		methodDef.put(NAME, methods[i].getName());
		methodDef.put(RESULT, typeOf(methods[i].getReturnType(), marks));
		methodDef.put(ARGUMENTS, genArgList(methods[i], marks));
		methodDef.put(EXCEPTIONS, genExceptionList(methods[i], marks));

		if ((modifiers & Modifier.STATIC)!=0) 
		    //Its static
		    staticMethods.addElement(methodDef);
		else 
		    //Its instance
		    instanceMethods.addElement(methodDef);
	    }
	}
	ointerface.put(IMETHODS, instanceMethods);
	ointerface.put(SMETHODS, staticMethods);

	ref.referent=ointerface;
	return ointerface;
    }

    public static Vector genExceptionList(Method m, Hashtable marks) {
	Class[] exceptions=m.getExceptionTypes();
	Vector v=new Vector(exceptions.length);
	for (int i=0; i < exceptions.length; i++) {
	    Hashtable argDef=new Hashtable(1);
	    argDef.put(NAME, exceptions[i].getName());
	    v.addElement(argDef);
	}
	return v;
    }

    public static Vector genArgList(Method m, Hashtable marks) {
	Class[] parameters=m.getParameterTypes();
	Vector v=new Vector(parameters.length);
	
	for (int i=0; i < parameters.length; i++) {
	    Hashtable argDef=new Hashtable(3);
	    argDef.put(NAME, "arg"+i);
	    argDef.put(TYPE, typeOf(parameters[i], marks));
	    v.addElement(argDef);
	}
	return v;
    }

    static final int QUAD = 32, OCTO = 64;

    public static Object typeOf(Class clazz, Hashtable marks) {
	if (clazz==Integer.TYPE) {
	    return new Integer(TYPE_INTEGER+(QUAD<<8));
	} else if (clazz==Long.TYPE) {
	    return new Integer(TYPE_INTEGER+(OCTO<<8));
	} else if (clazz==Float.TYPE) {
	    return new Integer(TYPE_FLOAT+(QUAD<<8));
	} else if (clazz==Double.TYPE) {
	    return new Integer(TYPE_FLOAT+(OCTO<<8));
	} else if (clazz==String.class) {
	    return new Integer(TYPE_OPAQUE);
	} else if (clazz==Vector.class) {
	    return new Integer(TYPE_LIST);
	} else if (clazz==Hashtable.class) {
  	    return new Integer(TYPE_DICTIONARY);
  	} else if (clazz==Void.TYPE) {
	    return new Integer(TYPE_VOID);
	} else {
  	    return generateInterface(clazz, marks);
  	}
    }
    /*
    public static void main(String[] args) throws Exception {
	Hashtable itf=(Hashtable)generateInterface(Test.class, null);
	System.out.println(org.casbah.util.Opossum.toString(itf));
	LDOProxyManager pm=new LDOProxyManager();
	Class ofProxy=pm.makeProxy(itf);
	Object proxy=pm.instantiateProxy(ofProxy);
	System.err.println(proxy+" "+proxy.getClass());
	Method m=proxy.getClass().getDeclaredMethod("helloWorld", new Class[] {String.class});
	m.invoke(proxy, new Object[] {"Hi!"});
    }
    */
}










