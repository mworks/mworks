#include <stdlib.h>
#include <scarab.h>
#include <scarab_utilities.h>

int 
scarab_dict_times2(ScarabDict *d)
{
	return d->tablesize * 2;
}

static ScarabDict*
newdict(int initialsize, ScarabDictExpFun f) 
{
	int i=0;
	ScarabDict *dict;
	
	dict = (ScarabDict*)scarab_mem_malloc(sizeof(ScarabDict));  
	
	dict->size = 0;
	dict->tablesize = initialsize;

	dict->keys = (ScarabDatum**)scarab_mem_malloc(
		initialsize * sizeof(ScarabDatum*));
	dict->values = (ScarabDatum**)scarab_mem_malloc(
		initialsize * sizeof(ScarabDatum*));

	dict->expansion_function = f;
	for (i=0; i < initialsize; i++)
	{
		dict->keys[i] = dict->values[i] = NULL;
	}
	
	return dict;
}

static void
rehash(ScarabDatum *dictionary) 
{
	int i = 0;
	
	
	ScarabDict *olddict = dictionary->data.dict;
	int newsize = olddict->expansion_function(olddict);
	ScarabDict *new_dict = newdict(newsize, olddict->expansion_function);
	ScarabDatum	**keys;
	ScarabDatum	**values;
	ScarabDatum *existing_val;

	keys = olddict->keys;
	values = olddict->values;

	dictionary->data.dict = new_dict;
	
	for(i = 0; i < olddict->tablesize; i++)
	{
		if (keys[i])
		{
			existing_val = scarab_dict_put(dictionary, keys[i], values[i]);
			if(existing_val) {
				/* badness...there shouldn't be anything here */
			}
			
			scarab_free_datum(keys[i]);
			scarab_free_datum(values[i]);
		}
	}
	
	scarab_mem_free(keys);
	scarab_mem_free(values);
	scarab_mem_free(olddict);
}

ScarabDatum*
scarab_dict_new(int initialsize, ScarabDictExpFun f) 
{
	ScarabDatum *d = scarab_new_molecular();
	
	if(initialsize < 1) {initialsize = 1; }
	
	ScarabDict *dict = newdict(initialsize, f);
	d->data.dict = dict;
	d->type = SCARAB_DICT;
	return d;
}

ScarabDatum*
scarab_dict_put(ScarabDatum *dictionary, ScarabDatum *_key,
				 ScarabDatum *_val) 
{
	scarab_lock_datum(dictionary);
	ScarabDatum *key = scarab_copy_datum(_key);
	ScarabDatum *val = scarab_copy_datum(_val);
	
	ScarabDict *dict = dictionary->data.dict;
	ScarabDatum *oldval;
	int i = 0;
	int hashval;
	int h = scarab_hash(key, dict->tablesize, 0);

	if (dict->keys[h] != NULL 
		&& !scarab_equals(dict->keys[h], key))
		h = scarab_hash(key, dict->tablesize, 1);
	
	if (dict->keys[h] != NULL
		&& !scarab_equals(dict->keys[h], key))
		h = scarab_hash(key, dict->tablesize, 2);
	
	if (dict->keys[h] != NULL
		&& !scarab_equals(dict->keys[h], key))
	{
		hashval = h;
		h = -1;
		for (i = hashval + 1; i < hashval + 1 + dict->tablesize; i++)
		{
			if (dict->keys[i % dict->tablesize] == NULL
				|| scarab_equals(dict->keys[i % dict->tablesize],key))
			{
				h = i % dict->tablesize;
				break;
			}
		}
		if (h == -1)
		{
			
			scarab_unlock_datum(dictionary);
			rehash(dictionary);
			
			oldval =  scarab_dict_put(dictionary, key, val);
			
			/* need to reduce the reference count of these datums because
				they were incremented in the last put */			
			scarab_free_datum(key);
			scarab_free_datum(val);

			return oldval;
		}
	}
	
	oldval = dict->values[h];
	if (dict->keys[h] == NULL) {
		++dict->size;
		dict->keys[h] = key;
	} else {
		scarab_free_datum(key);
	}
	
	dict->values[h] = val;
		
	scarab_unlock_datum(dictionary);
	return oldval;
}


ScarabDatum*
scarab_dict_get(ScarabDatum *dictionary, ScarabDatum *key) 
{
	int i = 0;
	int h = 0;
	int hashval = 0;
	
	scarab_lock_datum(dictionary);

	ScarabDict *dict = dictionary->data.dict;

	h = scarab_hash(key, dict->tablesize, 0);

	if (!scarab_equals(dict->keys[h], key))
		h = scarab_hash(key, dict->tablesize, 1);

	if (!scarab_equals(dict->keys[h], key))

		h = scarab_hash(key, dict->tablesize, 2);

	if (!scarab_equals(dict->keys[h], key))
	{
		hashval = h;
		h = -1;
		for (i = hashval + 1; i < hashval + 1 + dict->tablesize; i++)
		{
			if (scarab_equals(dict->keys[i % dict->tablesize],key))
			{
				h=i % dict->tablesize;
				break;
			}
		}
		if (h == -1){
			scarab_unlock_datum(dictionary);
			return NULL;
		}
	}
	
	ScarabDatum *result = dict->values[h];

	scarab_unlock_datum(dictionary);
	return result;
}

ScarabDatum*
scarab_dict_remove(ScarabDatum *dictionary, ScarabDatum *key) 
{
	
	scarab_lock_datum(dictionary);
	
	ScarabDict *dict = dictionary->data.dict;
	ScarabDatum *oldval;
	int i = 0;
	int h = scarab_hash(key, dict->tablesize, 0);
	int hashval = h;

	if (!scarab_equals(dict->keys[h], key))
		h = scarab_hash(key, dict->tablesize, 1);

	if (!scarab_equals(dict->keys[h], key))
		h = scarab_hash(key, dict->tablesize, 2);

	if (!scarab_equals(dict->keys[h], key))
	{
		hashval = h;
		h = -1;
		for (i = hashval + 1; i < hashval + 1 + dict->tablesize; i++)
		{
			if (scarab_equals(dict->keys[i % dict->tablesize],key))
			{
				h = i % dict->tablesize;
				break;
			}
		}
		if (h == -1){ 
			scarab_unlock_datum(dictionary);
			return NULL;
		}
	}
	if (dict->keys[h] != NULL) {
		--dict->size;
	}
	
	oldval = scarab_copy_datum(dict->values[h]);

	scarab_free_datum(dict->keys[h]);
	dict->keys[h] = NULL;
	scarab_free_datum(dict->values[h]);	
	dict->values[h] = NULL;
	
	scarab_unlock_datum(dictionary);
	return oldval;
}

ScarabDatum ** scarab_dict_keys(ScarabDatum * dictionary) {
    //scarab_lock_datum(dictionary);
	ScarabDict * dict = dictionary->data.dict;
    ScarabDatum **keys = dict->keys;
	//scarab_unlock_datum(dictionary);
	return keys;
}

ScarabDatum ** scarab_dict_values(ScarabDatum * dictionary) {
    //scarab_lock_datum(dictionary);
	ScarabDict * dict = dictionary->data.dict;
    ScarabDatum **values = dict->values;
	//scarab_unlock_datum(dictionary);
	return values;
}

int scarab_dict_number_of_elements(ScarabDatum * dictionary) {
    //scarab_lock_datum(dictionary);
	ScarabDict * dict = dictionary->data.dict;
    int size = dict->size;
	//scarab_unlock_datum(dictionary);
	return size;
}
