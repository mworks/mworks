#ifndef	SCARAB_MEM_H
#define	SCARAB_MEM_H


/*
 * Memory abstraction API for Scarab-C.
 *
 * Scarab will call these methods, which can be implemented by a number
 * of memory management systems, including memory pools, or standard
 * malloc.
 *
 * The basic interface is to construct a memory context, and place other
 * contexts inside of them.  Freeing a context releases everything in it,
 * including all sub-contexts.  This is basically identical to a pool system.
 */

/*
 * Initializes the implementing memory subsystem
 */
extern void scarab_mem_init(void);

/*
 * Returns a pointer to 'bytes' bytes of memory created within the 
 * given memory context.  Alignment suitable for either a void* or
 * a double must be guarented by the implementor.
 */
extern void *scarab_mem_malloc(int bytes);

/*
 * Frees a single allocation within a memory context.
 */
extern void scarab_mem_free(void *ptr);


#endif


