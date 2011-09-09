/*-MPL--------------------------------------------------------------------
 * ldo.java
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
 * The Original Code is ldo.java
 *
 * The Initial Developer of the Original Code is Scott G. Miller. Portions
 * created by Scott G. Miller are Copyright (C) 1999 The Casbah Project. All
 * Rights Reserved.
 *
 * Contributor(s):
 */

package org.casbah.ldo;

import org.casbah.util.*;
import java.io.*;
import java.net.*;
import java.util.Vector;

public class ldo {

    public static void main(String[] args) throws Exception {
	if (args.length<4) {
	    System.out.println("usage: java ldo [url] [object] [method] [args");
	    System.exit(1);
	}
	Socket s=new Socket(args[0].substring(0, args[0].indexOf(":")),
			    Integer.parseInt(
                            args[0].substring(args[0].indexOf(":")+1)));
	LDOInputStream ldoin=new LDOInputStream(s.getInputStream());
	LDOOutputStream ldoout=new LDOOutputStream(s.getOutputStream());
	LDOManager m=new LDOManager(ldoin, ldoout);
	String rargs="";
	for (int i=3; i<args.length; i++) {
	    rargs+=(args[i].indexOf(" ")!=-1 ? "\""+args[i]+"\" " :
		    args[i]+" ");
	}
	Vector margs=new Vector(args.length-3);
	Opossum proc=new Opossum(rargs);
	for (int i=3; i<args.length; i++) {
	    margs.addElement(proc.readObject());
	}
	System.out.println(m.call(args[1], args[2], margs));
    }
}
	
