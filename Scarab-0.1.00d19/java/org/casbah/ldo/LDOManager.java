/*-MPL--------------------------------------------------------------------
 * LDOManager.java
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
 * The Original Code is LDOManager.java
 *
 * The Initial Developer of the Original Code is Scott G. Miller. Portions
 * created by Scott G. Miller are Copyright (C) 1999 The Casbah Project. All
 * Rights Reserved.
 *
 * Contributor(s):
 */

package org.casbah.ldo;

import java.io.*;
import java.net.*;
import java.util.*;
import java.lang.reflect.*;

public class LDOManager {
    protected LDOInputStream in;
    protected LDOOutputStream out;
    protected Hashtable registry;
    
    public LDOManager(LDOInputStream i, LDOOutputStream o) {
	registry=new Hashtable(20);
	in=i;
	out=o;
    }

    public Object lookup(String objname) {
	return registry.get(objname);
    }

    public void register(String name, Object obj) {
	registry.put(name, obj);
    }

    public Hashtable service(Hashtable request) {
	Hashtable result=new Hashtable(1);
	Object object=registry.get(request.get("object"));
	if (object==null) {
	    result.put("error", "No such object");
	    return result;
	}
	Class ofObj=object.getClass();
	Vector argl=(Vector)request.get("args");
	Class[] argProto=new Class[argl.size()];
	Object[] argary=new Object[argl.size()];
	argl.copyInto(argary);
	for (int i=0; i<argl.size(); i++) {
	    argProto[i]=argl.elementAt(i).getClass();
	}
	Method[] pm=ofObj.getDeclaredMethods();
	Class[] methodProto=null;
	String methodName=(String)request.get("method");
	Method m=null;
	for (int i=0; i<pm.length; i++) {
	    if (pm[i].getName().equals(methodName) &&
		(methodProto=pm[i].getParameterTypes()).length==argProto.length)
		{
		    boolean correct=true;
		    for (int x=0; x<methodProto.length; x++) {
			while (argProto[x]!=null &&
			       !methodProto[x].equals(argProto[x])) {
			    argProto[x]=argProto[x].getSuperclass();
			}
			if (!argProto[x].equals(methodProto[x])) {
			    correct=false;
			    break;
			}
		    }
		    if (correct) {
			m=pm[i];
			break;
		    }
		}
	}
	if (m==null) {
	    result.put("error", "No such method");
	    return result;
	}
	try {
	    Object returnval=m.invoke(object, argary);
	    result.put("result", returnval);
	} catch (InvocationTargetException e) {
	    result.put("error", e.getTargetException().toString());
	} catch (IllegalAccessException e2) {
	    result.put("error", e2.toString());
	}
	return result;
    }

    public Object call(Object obj, String method, Vector args) throws Exception {
	Vector arg=new Vector(1);
	arg.addElement(args);
	Hashtable request=new Hashtable(3);
	request.put("object", obj);
	request.put("method", method);
	request.put("args", args);
	out.writeObject(request, false);
	Hashtable response=(Hashtable)in.readObject();
	if (response.get("result")==null) {
	    Object error=response.get("error");
	    if (error!=null) {
		throw new Exception((String)error);
	    }
	} 
	return response.get("result");
    }

    public static void main(String[] args) throws Exception {
	Socket sock=null;
	boolean server=false;
	if (args.length==0) {
	    server=true;
	    ServerSocket s=new ServerSocket(1946);
	    sock=s.accept();
	} else {
	    sock=new Socket(args[0], Integer.parseInt(args[1]));
	}
	LDOOutputStream o=new LDOOutputStream(sock.getOutputStream());
	LDOInputStream i=new LDOInputStream(sock.getInputStream());

	LDOManager m=new LDOManager(i, o);
	m.register("echo", new Object() {
	    public String echo(String message) {
		return message;
	    }
	});
	while (server) {
	    Hashtable request=(Hashtable)m.in.readObject();
	    Hashtable result=m.service(request);
	    m.out.writeObject(result, false);
	}
	BufferedReader in=new BufferedReader(new InputStreamReader(System.in));
	Vector v=new Vector(1);
	v.addElement("");
	while (true) {
	    v.setElementAt(in.readLine(), 0);
	    System.out.println(m.call("echo", "echo", v));
	}
    }

}
