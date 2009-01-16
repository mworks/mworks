package org.casbah.ldo;

import org.casbah.classlib.proxygen.*;

/**
 * The Proxy Host implementation for LDO
 */
public class LDOProxyHost implements ProxyHost {
    protected LDOProxyManager parent;

    public LDOProxyHost(LDOProxyManager m) {
	parent=m;
    }

    public Object methodCall(Object target, String method, Object[] args) {
	System.err.println("["+target+"] "+method+" called");
	return null;
    }

    public Object methodCall(Object target, String method, Object[] args,
			     int[] wrapped) {
	System.err.println("["+target+"] "+method+" called");
	return null;
    }
}
