#include "scarab_mem.h"
#include <coptic.h>

void scarab_mem_init(void)
{
  cp_palloc_init();
  cp_thread_init();
}

void           *
scarab_mem_malloc(void *memory_context, int bytes)
{
	return cp_palloc((cp_pool *) memory_context, (size_t) bytes);
}

/**
 * Returns a new memory context that resides in the parent context
 */
void           *
scarab_mem_subcontext_new(void *parent_context)
{
	return cp_pool_create((cp_pool *) parent_context);
}

/**
 * Returns a new independent memory context
 */
void           *
scarab_memctx_new(void)
{
	return cp_pool_create(NULL);
}

/**
 * Frees a variable within a memory context
 */
void 
scarab_mem_free(void *memory_context, void *ptr)
{
}

/**
 * Frees an entire memory context
 */
void 
scarab_memctx_free(void *memory_context)
{
	cp_pool_destroy((cp_pool *) memory_context);
}
