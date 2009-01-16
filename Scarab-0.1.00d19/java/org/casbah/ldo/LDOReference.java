package org.casbah.ldo;

class LDOReference {
    long refid;
    long realid;
    Object referent;

    LDOReference(long id) {
	refid=id;
	realid=-1;
    }
}
