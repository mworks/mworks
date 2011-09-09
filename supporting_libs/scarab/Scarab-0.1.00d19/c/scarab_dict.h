#ifndef	SCARAB_DICT_H
#define	SCARAB_DICT_H

/*
 * Scarab dictionary simple implementation.
 */

FWD(struct,ScarabDict);

/*
 * Given the dictionary, returns the best size for a rehash-on-filled.
 */
typedef int (*ScarabDictExpFun) (ScarabDict *dict);

DEF(struct,ScarabDict)
{
	int					size;
	int					tablesize;
	ScarabDatum			**keys;
	ScarabDatum			**values;
	ScarabDictExpFun	expansion_function;
};


/*
 * Creates a new (empty) dict.
 */
extern ScarabDatum *scarab_dict_new(int initialsize,
									ScarabDictExpFun f);

/*
 * Place a value at 'key'.
 * Returns the old value of 'key' (or NULL).
 */
extern ScarabDatum *scarab_dict_put(ScarabDatum *dict, ScarabDatum *key,
									ScarabDatum *val);

/*
 * Get the value of 'key'.
 * Returns NULL if 'key' does not exist.
 */
extern ScarabDatum *scarab_dict_get(ScarabDatum *dict, ScarabDatum *key);

/*
 * Remove the key 'key'.
 * Returns NULL if 'key' does not exist.
 * Returns the old value of 'key' if it existed before the remove.
 */
extern ScarabDatum *scarab_dict_remove(ScarabDatum *dict, ScarabDatum *key);

/*
 * expansion functions
 */
extern int scarab_dict_times2(ScarabDict *dict);

//<EDIT>
/**
 * Returns a pointer to an array of keys in the dictionary.
 */
extern ScarabDatum ** scarab_dict_keys(ScarabDatum * dict);

/**
 * Returns a pointer to an array of values in the dictionary. 
 */
extern ScarabDatum ** scarab_dict_values(ScarabDatum * dict);

/**
 * Returns the number of elements in the dictionary.
 */
extern int scarab_dict_number_of_elements(ScarabDatum * dict);
//</EDIT>

#endif
