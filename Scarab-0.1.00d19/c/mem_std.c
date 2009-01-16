#include <scarab.h>
#include <stdlib.h>
//#include <malloc.h>

FWD(struct,scarab_mem_reg);
FWD(struct,scarab_mem_ctx);

DEF(struct,scarab_mem_reg)
{
	scarab_mem_reg	*next;
};

DEF(struct,scarab_mem_ctx)
{
	scarab_mem_ctx	*parent;
	scarab_mem_ctx	*prev;
	scarab_mem_ctx	*next;
	scarab_mem_ctx	*children;
	scarab_mem_reg	*regions;
};


void scarab_mem_init(void)
{
}
  
void *scarab_mem_malloc(void *memory_context, int bytes) 
{
	scarab_mem_reg	*reg;
	scarab_mem_ctx	*ctx;

	ctx = (scarab_mem_ctx*)memory_context;
	reg = (scarab_mem_reg*)malloc(sizeof(scarab_mem_reg) + bytes); // ***
	if (!reg)
	{
		return NULL;
	}

	reg->next = ctx->regions;
	ctx->regions = reg;

	return reg+1;
}

/**
 * Returns a new memory context that resides in the parent context
 */
void *scarab_mem_subcontext_new(void *parent_context) 
{
	scarab_mem_ctx	*ctx;
	scarab_mem_ctx	*par;

	par = (scarab_mem_ctx*)parent_context;
	ctx = (scarab_mem_ctx*)calloc(1, sizeof(scarab_mem_ctx));
	
	if (!ctx)
	{
		return NULL;
	}
	if (par)
	{
		ctx->parent = par;
		ctx->prev = NULL;
		ctx->next = par->children;
		if (ctx->next)
		{
			ctx->next->prev = ctx;
		}
		par->children = ctx;
	} else
	{
		ctx->parent = NULL;
		ctx->prev = NULL;
		ctx->next = NULL;
	}

	ctx->children = NULL;
	ctx->regions = NULL;
	
	return ctx;
}

/**
 * Returns a new independent memory context
 */
void *scarab_memctx_new(void)
{
	return scarab_mem_subcontext_new(NULL);
}

/**
 * Frees a variable within a memory context
 */
void scarab_mem_free(void *memory_context, void *ptr) 
{ 
	scarab_mem_ctx	*ctx;
	scarab_mem_reg	*reg;
	scarab_mem_reg	*r;
	scarab_mem_reg	*p;
	
	if(memory_context == NULL){
		if(ptr != NULL){
			free(ptr);
		}
		return;
	}

	ctx = (scarab_mem_ctx*)memory_context;
	reg = ((scarab_mem_reg*)ptr) - 1;
	
	for(p = NULL, r = ctx->regions; r; p = r, r = r->next)
	{
		if (r == reg)
		{
			if (!p)
			{
				ctx->regions = reg->next;
			} else
			{
				p->next = reg->next;
			}
			free(reg);
			return;
		}
	}
}

/**
 * Frees an entire memory context
 */
void scarab_memctx_free(void *memory_context)
{
	scarab_mem_ctx	*ctx;
	scarab_mem_ctx	*cr;
	scarab_mem_ctx	*cn;
	scarab_mem_reg	*rr;
	scarab_mem_reg	*rn;

	ctx = (scarab_mem_ctx*)memory_context;
	
	if(!ctx){
		return;
	}
	
	for(cr = ctx->children; cr; cr = cn)
	{
		/*
		 * This keeps us from crashing when the child
		 * region is deleted.  We also set its parent
		 * reference to NULL so it won't waste time
		 * deleting itself from our list.
		 */
		cn = cr->next;
		cr->parent = NULL;
		scarab_memctx_free(cr);
	}

	for(rr = ctx->regions; rr; rr = rn)
	{
		rn = rr->next;
		free(rr);
	}

	if (ctx->parent)
	{
		if (ctx->prev)
		{
			ctx->prev->next = ctx->next;
		} else
		{
			ctx->parent->children = ctx->next;
		}

		if (ctx->next)
		{
			ctx->next->prev = ctx->prev;
		}
	}
	
}
