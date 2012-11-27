#include <assert.h>
#include <string.h>

#include <scarab.h>


ScarabDatum  *
scarab_new_molecular() 
{
	return scarab_new_atomic();
}


ScarabDatum  *
scarab_new_atomic() 
{
	ScarabDatum *datum;
	datum=(ScarabDatum*)scarab_mem_malloc(sizeof(ScarabDatum));
	datum->type=SCARAB_NULL;
	datum->ref_count = 1;
#ifdef THREAD_SAFE_SCARAB_DATUM
	datum->mutex = (pthread_mutex_t *)scarab_mem_malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(datum->mutex, NULL);
#endif
	return datum;
}


ScarabDatum *scarab_new_integer(long long num) 
{
	ScarabDatum *d=scarab_new_atomic();
	d->type=SCARAB_INTEGER;
	d->data.integer=num;
	return d;
}


ScarabDatum *scarab_new_float( double num) 
{
	ScarabDatum *d=scarab_new_atomic();
	d->type=SCARAB_FLOAT;
	d->data.floatp=num;
	return d;
}


ScarabDatum *scarab_new_string(const char* opaque) 
{
    assert(opaque != NULL);
    return scarab_new_opaque(opaque, strlen(opaque) + 1);
}


ScarabDatum *scarab_new_opaque(const char* opaque, int size) 
{
	ScarabDatum *d = scarab_new_atomic();
	d->type = SCARAB_OPAQUE;
    
    if (size < 0) {
        size = 0;
    }
    
	d->data.opaque.size = size;

    // Allocate space for size+1 bytes, and set the extra byte to NUL to protect against
    // code that blindly assumes the data is NUL-terminated
	d->data.opaque.data = (unsigned char *)scarab_mem_malloc((size + 1) * sizeof(unsigned char));
    d->data.opaque.data[size] = '\0';
    
    if (opaque != NULL) {
        memcpy(d->data.opaque.data, opaque, size);
    }
    
	return d;
}


void scarab_lock_datum(ScarabDatum *datum){
#ifdef THREAD_SAFE_SCARAB_DATUM
    if(datum != NULL){
        pthread_mutex_lock(datum->mutex);
    }
#endif
}


void scarab_unlock_datum(ScarabDatum *datum){
#ifdef THREAD_SAFE_SCARAB_DATUM
    if(datum != NULL){
        pthread_mutex_unlock(datum->mutex);
    }
#endif
}


ScarabDatum * scarab_copy_datum(ScarabDatum *datum){
	
	if(datum != NULL){
		scarab_lock_datum(datum);
		datum->ref_count++;
		scarab_unlock_datum(datum);
	}
	
	return datum;
}

void scarab_free_datum(ScarabDatum *d){
	
	int i;
	
	if(d == NULL) return;
	
#ifdef THREAD_SAFE_SCARAB_DATUM
	pthread_mutex_lock(d->mutex);
#endif

	// One fewer reference to this object
	d->ref_count--;
	
	// If the thing is no longer referenced, kill it
	if((d->ref_count) == 0){
	
		switch (d->type) 
		{
			case SCARAB_DICT: 
				// free sub-data
				for(i = 0; i < d->data.dict->tablesize; i++){
					scarab_free_datum(d->data.dict->keys[i]);
					scarab_free_datum(d->data.dict->values[i]);
				}
				
				// Free the parts
				scarab_mem_free(d->data.dict->keys);
				scarab_mem_free(d->data.dict->values);
				
				scarab_mem_free(d->data.dict);
					
				break;
			case SCARAB_LIST:
				// free sub-data
				for(i = 0; i < d->data.list->size; i++){
					scarab_free_datum(d->data.list->values[i]);
				}

				// free parts
				scarab_mem_free(d->data.list->values);
				scarab_mem_free(d->data.list);

				break;
			case SCARAB_OPAQUE:
				
				// free opaque data
				if(d->data.opaque.data != NULL){
					scarab_mem_free(d->data.opaque.data );
				}
				break;
			default:
				
				break;
		}
		
#ifdef THREAD_SAFE_SCARAB_DATUM
		// Take the lock out of the ScarabDatum, we want to hang onto it
		// until we're done
		pthread_mutex_t *lock = d->mutex;
#endif
        
		scarab_mem_free(d);
		
#ifdef THREAD_SAFE_SCARAB_DATUM
		// now, release the lock and destroy it
		pthread_mutex_unlock(lock);
		pthread_mutex_destroy(lock);
		scarab_mem_free(lock);
#endif
		
		return;
	}	
	
#ifdef THREAD_SAFE_SCARAB_DATUM
	pthread_mutex_unlock(d->mutex);
#endif
}


// This won't work until you fix copying lists and dictionaries
//ScarabDatum * scarab_deep_copy_datum(ScarabDatum *datum){
//  
//  ScarabDatum *return_datum;
//  int i;
//  
//  if(datum == NULL){
//    return NULL;
//  }
//  
//  
//  //scarab_lock_datum(datum);
//
//  switch(datum->type){
//    
//  case SCARAB_LIST:
//    
//    return_datum = scarab_list_new(datum->data.list->size);
//    
//    for(i = 0; i < datum->data.list->size; i++){
//      
//      // recurse
//      ScarabDatum *subdatum = scarab_deep_copy_datum(scarab_list_get(datum, i));
//      
//      if(subdatum != NULL){
//			scarab_list_put(return_datum, i, subdatum);
//      } else {
//	/* fprintf(stderr,
//	       "Tried to copy null element in ScarabListDatum (%d of %d)\n",
//	       i, datum->data.list->size); */
//      }
//    }
//    break;
//    
//  case SCARAB_DICT:
//    
//    return_datum = scarab_dict_new(
//				   datum->data.dict->tablesize, 
//				   &scarab_dict_times2);
//    ScarabDatum **keys = scarab_dict_keys(datum);
//    ScarabDatum **values = scarab_dict_values(datum);
//    
//    for(i = 0; i < datum->data.dict->tablesize; i++){
//      
//      if(keys[i] != NULL && values[i] != NULL){
//	// recurse
//		scarab_dict_put(return_datum, 
//			scarab_deep_copy_datum(keys[i]),
//			scarab_deep_copy_datum(values[i]) );
//      } else {
//	/*fprintf(stderr,
//	       "Tried to copy null element in ScarabDictDatum (%d of %d)\n",
//	       i, datum->data.dict->tablesize); */
//      }
//      
//    }
//    
//    break;
//    
//  case SCARAB_OPAQUE:
//    
//    return_datum = (ScarabDatum *)calloc(1, sizeof(ScarabDatum));
//    *return_datum = *datum;
//    
//	scarab_lock_datum(datum);
//    // copy the char *
//    return_datum->data.opaque.data = 
//      (unsigned char *)calloc(datum->data.opaque.size, 
//			      sizeof(unsigned char));
//    
//    memcpy(return_datum->data.opaque.data, datum->data.opaque.data,
//	   datum->data.opaque.size );
//	scarab_unlock_datum(datum);
//    break;
//  default:
//    
//	scarab_lock_datum(datum);
//    return_datum = (ScarabDatum *)calloc(1, sizeof(ScarabDatum));
//    *return_datum = *datum;
//	scarab_unlock_datum(datum);
//    break;
//  }
//  
//  
//  //scarab_unlock_datum(datum);
//
//  return return_datum;
//  
//}



