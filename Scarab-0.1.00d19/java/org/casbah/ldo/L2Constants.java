package org.casbah.ldo;

public interface L2Constants {

    public static String
	LDO_LAYER_II       = "LDO-LAYER-II",
	CLASSNAME          = "className",
	SMETHODS           = "class_meth",
	IMETHODS           = "inst_meth",

	NAME               = "name",
	RESULT             = "result",
	ARGUMENTS          = "arguments",
	EXCEPTIONS         = "exceptions", 
	TYPE               = "type",
	OPTIONAL           = "optional",

	OBJECTID           = "objectID",
	METHODNAME         = "methodName",
	ARGS               = "args",
	CALLID             = "callID",
	EXCEPTION          = "exception",
	ERRMSG             = "errmsg",
	SERVER             = "server";

    
    public static int
	AF_INET            = 0x01,
	AF_UNIX            = 0x02,
	AF_SHMCOPTIC       = 0x03,
	
	INET_TCP           = 0x01,
	INET_UDP           = 0x02,
	
	TYPE_VOID          = 0x01,
	TYPE_INTEGER       = 0x06,
	TYPE_FLOAT         = 0x07,
	TYPE_OPAQUE        = 0x02,
	TYPE_LIST          = 0x03,
	TYPE_DICTIONARY    = 0x04,
	TYPE_VARARG        = 0x05;
}
	
