#include <scarab.h>

// TODO: does this work?
ScarabDatum*
scarab_list_copy(ScarabDatum *dest, int destpos, ScarabDatum *source, 
				  int sourcepos, int count) 
{
	int i;
	scarab_lock_datum(dest);
	scarab_lock_datum(source);
	
	ScarabList *dl = dest->data.list;
	ScarabList *sl = source->data.list;
	for (i = 0; i < count; i++)
	{
		dl->values[destpos + i] = sl->values[sourcepos + i];
	}
	
	scarab_unlock_datum(dest);
	scarab_unlock_datum(source);
	return dest;
}

ScarabDatum*
scarab_list_new(int initialsize) 
{
	int i;
	ScarabDatum *d = scarab_new_molecular();
	ScarabList *list;
	list = (ScarabList*)scarab_mem_malloc(sizeof(ScarabList));
	list->size = initialsize;
	list->values = (ScarabDatum**)scarab_mem_malloc(initialsize * sizeof(void*));
	for (i = 0; i < initialsize; i++)
	{
		list->values[i] = NULL;
	}
	d->data.list = list;
	d->type = SCARAB_LIST;
	return d;
}

ScarabDatum*
scarab_list_put(ScarabDatum *list, int idx, ScarabDatum *val) 
{
	scarab_lock_datum(list);
	scarab_lock_datum(val);
	ScarabList *l = list->data.list;
	ScarabDatum *oldval = l->values[idx];
	l->values[idx] = val;
	
	// increment the ref_count for the incoming datum
	if(val != NULL){
		val->ref_count++;
	}
	
	// decrement the ref_count for the outgoing datum
	// Actually, we're giving back a ref, so don't decrement...
	/*if(oldval != NULL){
		scarab_lock_datum(oldval);
		oldval->ref_count--;
		scarab_unlock_datum(oldval);
	}*/
	
	scarab_unlock_datum(val);
	scarab_unlock_datum(list);
	return oldval;
}

ScarabDatum* 
scarab_list_get(ScarabDatum *list, int idx) 
{
	scarab_lock_datum(list);
	ScarabList *l = list->data.list;
	ScarabDatum *value = l->values[idx];
	scarab_unlock_datum(list);
	return value;
}


// TODO ...  this won't work ... you could end up with a refcount of 0
//ScarabDatum*
//scarab_list_remove(ScarabDatum *list, int idx) 
//{
//	scarab_lock_datum(list);
//	ScarabList *l = list->data.list;
//	ScarabDatum *oldval = l->values[idx];
//	l->values[idx] = NULL;
//	
//	if(oldval != NULL){
//		scarab_lock_datum(oldval);
//		oldval->ref_count--;
//		scarab_unlock_datum(oldval);
//	}
//	scarab_unlock_datum(list);
//	return oldval;
//}
