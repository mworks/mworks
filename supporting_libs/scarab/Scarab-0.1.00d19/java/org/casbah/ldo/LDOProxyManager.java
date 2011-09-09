package org.casbah.ldo;

import org.casbah.classlib.proxygen.*;
import org.casbah.classlib.*;
import java.util.*;
import java.io.*;
import java.lang.reflect.*;

/**
 * Handles proxy generation for Java LDO
 */
public class LDOProxyManager implements L2Constants, ClassConstants {
    protected static final Class[] PCONSPROTO= 
	new Class[] {ProxyHost.class, Object.class};

    protected ProxyGenerator pg;
    protected LDOProxyHost host;
    protected Hashtable proxies;

   public LDOProxyManager() {
	proxies=new Hashtable(50);
	host=new LDOProxyHost(this);
	pg=new ProxyGenerator(host, 
			      "/home/scgmille/cbsrc/ldo/LDOProxyBase.class");
    }

    public Object instantiateProxy(Class result) throws Exception {
	Constructor c=result.getConstructor(PCONSPROTO);
	return c.newInstance(new Object[] {host, null});
    }

    public Class makeProxy(Hashtable ointerface) throws IOException {
	ClassBuffer base=pg.getNew();
	base.setName((String)ointerface.get(CLASSNAME));
	
	Vector staticMethods=(Vector)ointerface.get(SMETHODS);
	Vector instanceMethods=(Vector)ointerface.get(IMETHODS);
	addMethods(base, staticMethods, ACC_PUBLIC+ACC_STATIC);
	addMethods(base, instanceMethods, ACC_PUBLIC);
	ClassWriter w=new ClassWriter(new FileOutputStream("out.class"));
	w.writeClass(base);
	try {
	    pg.forgeClass(base);
	    Class result=pg.loadClass(base.getName(), false);
	    proxies.put(base.getName(), result);
	    return result;
	} catch (ClassNotFoundException e) {
	    throw new IOException("Christ! It doesn't like our proxy!");
	}
    }

    void addMethods(ClassBuffer base, Vector methodList, 
		    int modifiers) throws IOException {

	for (int i=0; i<methodList.size(); i++) {
	    Hashtable methodDef=(Hashtable)methodList.elementAt(i);
	    String methodName=(String)methodDef.get(NAME);
	    String returnType=typeFor(methodDef.get(RESULT));

	    Vector argList=(Vector)methodDef.get(ARGUMENTS);
	    String[] proto=new String[argList.size()];
	    for (int x=0; x<proto.length; x++) {
		Hashtable argDef=(Hashtable)argList.elementAt(x);
		proto[x]=typeFor(argDef.get(TYPE));
	    }
	    pg.addMethod(base, modifiers, returnType, methodName, 
			 proto, new String[] {"java.lang.Exception"},
			 pg.assemble(methodName, proto, base)); 
	    
	}
    }

    public String typeFor(Object typev) throws IOException {
	if (typev instanceof Hashtable) {
	    return (String)((Hashtable)typev).get(CLASSNAME);
	} else if (typev instanceof LDOReference) {
	    return typeFor(((LDOReference)typev).referent);
	} else {
	    int type=((Integer)typev).intValue();
	    switch (type & 0x0F) {
	    case TYPE_INTEGER:
		if (type>>>8==32) 
		    return ITYPE;
		else
		    return LTYPE;
	    case TYPE_FLOAT:
		if (type>>>8==32) 
		    return FTYPE;
		else
		    return DTYPE;
	    case TYPE_OPAQUE:
		return STRINGTYPE;
	    case TYPE_VOID:
		return VTYPE;
	    case TYPE_LIST:
		return LISTTYPE;
	    case TYPE_DICTIONARY:
		return DICTIONARYTYPE;
	    default:
		return null;
	    }
	}
    }
}













