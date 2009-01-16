#ifndef SCARAB_LIST_H
#define SCARAB_LIST_H

/*
 * Scarab List
 *
 * Provides a simple list for ScarabDatum items.
 *
 * Each list has a fixed size providing indexes
 * between 0 and size - 1.
 */

FWD(struct,ScarabList);
DEF(struct,ScarabList)
{
	int			size;
	ScarabDatum	** values;
};


/*
 * Create a new list with capacity 'size'.
 */
extern ScarabDatum *scarab_list_new(int size);

/*
 * Place a 'val' at index 'idx'.
 * Replaces the existing value.
 * Returns the replaced value.
 */
extern ScarabDatum* scarab_list_put(ScarabDatum *list, int idx,
									ScarabDatum *val);

/*
 * Retrieve the value from index 'idx'.
 * Returns NULL if the index is out of bounds.
 */
extern ScarabDatum *scarab_list_get(ScarabDatum *list, int idx); 

/*
 * Delete the element at 'idx'.
 * Pushes all elements down and returns the
 * deleted element.
 */
// extern ScarabDatum *scarab_list_remove(ScarabDatum *list, int idx);

/*
 * Copy 'source' into 'dest' at position 'destpos'.
 * 'dest' must have enough room to fit 'source'.
 */
extern ScarabDatum *scarab_list_copy(ScarabDatum *dest, int destpos,
									 ScarabDatum *source, 
									 int sourcepos, int count);

#endif
