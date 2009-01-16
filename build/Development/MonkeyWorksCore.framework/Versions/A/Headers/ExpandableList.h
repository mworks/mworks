/**
 * ExpandableList.h
 *
 * Description: A list that contains anything.  Expands when needed.  Please
 * note that any object that is put into the list will have its copy
 * constructor called.  If an object has data members that are pointers to
 * objects then you must make sure that either A) you define a copy 
 * constructor for that object or B) do not define a destructor.  The
 * expandable list will try to free any memory it has allocated when it
 * is destroyed and errors will occur if you have pointer data in your
 * object and the object has a defined destructor but not a defined copy
 * constructor.
 *
 * History:
 * David Cox on ??/??/?? - Created.
 * Paul Jankunas on 05/18/05 - Added this header.  Changed implicit call to
 *                      copy constructor to be explicit to make it clearer as
 *                      to what is happening.  Wrote a destructor and copy
 *                      constructor.  Made the expand functions private.
 *                      added private init() function.
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#ifndef EXPANDABLE_LIST_H
#define EXPANDABLE_LIST_H

#include <stdlib.h>
#include <stdio.h>
#include "Lockable.h"
#include "Exceptions.h"

#define CRAZY_BIG_NUMBER 10000
#define DEFAULT_ELEMENT_INCREMENT 10
#define M_THREAD_SAFE_EXPANDABLE_LISTS	1

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
namespace mw {
	using namespace boost;

template <class Class> 
class ExpandableList : public Lockable {
    protected:
	shared_ptr<Class> *list;
		//mClass ** list;
        int elementincrement;
        int nelements;
        int nactualelements;

    private:
        // NULLS the list.  Does not free any memory.  Only call from
        // a constructor.
        void init() {
            for(int i = 0; i < nelements; i++) {
//                list[i] = NULL;
            }
        }
        
        // NULL out a list of size 'size'.
        void init(Class ** nList, int size) {
            for(int i = 0; i < size; i++) {
            //    nList[i] = NULL;
            }            
        }
        
        // Expands the list by added 'elementsincrement' to its size.
        void expandList() {
            if(elementincrement < 1){
				elementincrement = DEFAULT_ELEMENT_INCREMENT;
			}
			
			shared_ptr<Class> * newlist = 
			new shared_ptr<Class>[nelements + elementincrement];
            //if(newlist == NULL) {
                // throw an error here out of memory TODO
               // return;
            //}
			
			
			
            //init(newlist, nelements + elementincrement);
			
            for(int i = 0; i < nactualelements; i++) {
                newlist[i] = list[i];
            }
            delete [] list;
            list = newlist;
            nelements = nelements + elementincrement;
        }
        
        // Expands the list to be nitems long.  Be sure that nitems is a
        // reasonable number when calling this fucntion.  I made it private
        // for that reason.
        void expandList(int nitems) {
            shared_ptr<Class> * newlist = new shared_ptr<Class>[nitems];
            if(newlist == NULL) {
                // throw an error here out of memory TODO
                throw SimpleException("Out of memory");
				return;
            }
                
            //init(newlist, nitems);
            
            for(int i = 0; i < nactualelements; i++) {
                newlist[i] = list[i];
            }
            delete [] list;
            list = newlist;
            nelements = nitems;
        }

	public:
        
        /**
         * Default Constructor.  Creates a ten element list.  Sets the 
         * element increment size to ten meaning that on expanding the list
         * ten new elements will be added.
         */
        ExpandableList() {
            elementincrement = DEFAULT_ELEMENT_INCREMENT;
            nelements = elementincrement;
            nactualelements = 0;
            list = new shared_ptr<Class>[nelements];
            init();
        }
        
        /**
         * Constructor.  Creates a list of size 'arraysize'.  Will throw an
         * error if arraysize is less than zero or greater than 10,000.  Sets
         * the element increment size to 'arraysize' meaning that on 
         * expanding the list 'arraysize' elements will be added.
         */
        ExpandableList(int arraysize) {
            nelements = arraysize;
			if(elementincrement > 0){
				elementincrement = arraysize;
            } else {
				elementincrement = 1;
			}
			nactualelements = 0;
            if(nelements == 0) {
                nelements = 1;
            }
            if(nelements < 0) {
                throw("Invalid array size in constructing an ExpandableList");
            }
            if(nelements > CRAZY_BIG_NUMBER) {
                throw("Error: enormous freak'n Expandable list.  Did you mean to do that?");
            }
            list = new shared_ptr<Class>[nelements];
            init();
        }
        
        /**
         * Copy Constructor.
         */
        ExpandableList(const ExpandableList<Class>& listRef){
            // copy the integer things
            nactualelements = listRef.nactualelements;
            nelements = listRef.nelements;
            elementincrement = listRef.elementincrement;
            
            // make the new list
            list = new shared_ptr<Class>[nelements];
            for(int i = 0; i < nelements; i++) {
                //if(listRef[i] == NULL) {
//                    list[i] = shared_ptr<Class>();
//                    continue;
//                }
//                Class * tmp = listRef[i];
                list[i] = listRef.list[i];
            }
        }
        
        /**
         * Destructor.
         */
        ~ExpandableList() {
            // free all the memory for the objects
            // at each spot in the list if there is an object there
            for(int i = 0; i < nactualelements; i++) {
                if(list[i]) {
                    //delete list[i];
                    //list[i] = NULL;
                }
            }
            // free the list
            delete [] list;
            list = NULL;
        }
        
		
		/** 
		* Release the elements of the list
		* This should be called on a list that contains only references
		* to objects which are owned by someone else
		* Calling this prevents double frees of the list elements
		**/
		
		void releaseElements(){
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				lock();
			#endif
			
			for(int i = 0; i < nelements; i++){
				//list[i] = NULL;  // be free!
				list[i] = shared_ptr<Class>();
			}
			
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				unlock();
			#endif
		}
		
		
        /**
         * Returns the total amount of storage sapce in the list.  This may
         * be different from the number of elements in the list.
         */
        int getListCapacity() {
            return nelements;
        }
        
        /**
         * Returns the number of elements a list will add on expansion.
         */
        int getExpansionFactor() {
            return elementincrement;
        }
        
        /**
         * Adds the element to the list by making a copy of it.  Returns the
         * index in the array where the object was addded.
         */
int addElement(shared_ptr<Class> objtoadd) {
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				lock();
			#endif
			if(nactualelements >= nelements) {
                expandList();
            }
			
            if(objtoadd == NULL) {
                list[nactualelements] = shared_ptr<Class>();
            } else {
                //mClass * copy = new Class(*objtoadd);
                list[nactualelements] = objtoadd;
            }
            int returnval = nactualelements++;
            
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				unlock();
			#endif
			
			return returnval;
        }

        /**
         * Adds the object to the list by calling addElement(Class *).
         */
        int addElement(Class thingtoadd) {
			shared_ptr<Class> p(new Class((const Class&)thingtoadd));
            return addElement(p);
        }
        
        /**
         * Adds an object to the list at a given index.  If index is greater
         * than the number of elements in the list, the list will be expanded.
         * Note that after the expansion the reported actual number of objects
         * will be reported as the greatest index where an object is located.
         * Returns the index where the object was stored.  Returns -1 if
         * index is less than zero or larger than CRAZY_BIG_NUMBER
         */
int addElement(int index, shared_ptr<Class> objtoadd) {
            #if M_THREAD_SAFE_EXPANDABLE_LISTS
				lock();
			#endif
			
			// add a new object at a predefined index 
            if((index < 0) || (index > CRAZY_BIG_NUMBER)) {
                // should we throw an error here? TODO
                return -1;
            }
            if(index >= nelements) {
                // expand list to contain "index" elements
                // index+1 because we want to place at item at index
                // so there needs to be index+1 items because we count
                // from zero.
                expandList(index+1);
            }
            
            nactualelements = index+1;
			
            // delete the object that is already there.
			//mClass *item_to_delete = list[index];
			//if(item_to_delete != NULL) {
            //    delete item_to_delete;  
            //    list[index] = NULL;
            //}
            
			if(objtoadd == NULL) {
                list[index] = shared_ptr<Class>();
            } else {
                //mClass * copy = new Class(*objtoadd);
                //list[index] = copy;
				list[index] = objtoadd;
            }
            
			int returnval = nactualelements;
			
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				unlock();
			#endif
			
			return returnval;
        }
        
		
        /**
         * Adds the object by calling addElement(Class *).
         */
        int addElement(int index, Class thingtoadd) {
			shared_ptr<Class> p(new Class((const Class&)thingtoadd));
            return addElement(index, p);
        }


		// Add a pointer to the object
		// DON'T COPY IT
		// This is necessary because all of that copying ruins polymorphism
		// (e.g. an mw::Protocol cast as an State gets copied as an State, so
		// everything goes to shit)
		
int addReference(shared_ptr<Class> objtoadd){
			return addElement(objtoadd);
		}
		
/*		int addReference(Class *objtoadd) {
            #if M_THREAD_SAFE_EXPANDABLE_LISTS
				lock();
			#endif
			
			if(nactualelements >= nelements) {
                expandList();
            }
            if(objtoadd == NULL) {
                list[nactualelements] = NULL;
            } else {
                list[nactualelements] = objtoadd;
            }
            int returnval = nactualelements;
			nactualelements++;
            
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				unlock();
			#endif
			
			return returnval;
        }*/
		
int addReference(int index, shared_ptr<Class> objtoadd){
			return addElement(index, objtoadd);
		}
		
        /*int addReference(int index, Class *objtoadd) {
            #if M_THREAD_SAFE_EXPANDABLE_LISTS
				lock();
			#endif
			
			// add a new object at a predefined index 
            if((index < 0) || (index > CRAZY_BIG_NUMBER)) {
                // should we throw an error here? TODO
				#if M_THREAD_SAFE_EXPANDABLE_LISTS
					unlock();
				#endif    
				return -1;
            }
            if(index >= nelements) {
                // expand list to contain "index" elements
                // index+1 because we want to place at item at index
                // so there needs to be index+1 items because we count
                // from zero.
                expandList(index+1);
            }
            
            nactualelements = index+1;
            
			// TODO: double free?
			// delete the object that is already there.
            if(list[index] != NULL) {
                delete list[index];	
                list[index] = NULL;
            }
            if(objtoadd == NULL) {
                list[index] = NULL;
            } else {
                list[index] = objtoadd;
            }
			
		
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				unlock();
			#endif
			
            return index;
        }*/
		
		

        /** 
         * Removes any empty items from the list making the list size
         * equeal to the number of elements.
         */
        void trimList() {
            #if M_THREAD_SAFE_EXPANDABLE_LISTS
				lock();
			#endif
			
			// dont trim the list if it is full
            if(nactualelements == nelements) {
                #if M_THREAD_SAFE_EXPANDABLE_LISTS
					unlock();
				#endif
				return;
            }
            
			shared_ptr<Class> * newlist = 
				new shared_ptr<Class>[nactualelements];
            
			for(int i = 0; i < nactualelements; i++) {
                // make deep copies
                if(list[i] != NULL) {
                    // copy the old one then delete it.
                    //newlist[i] = new Class(list[i]);
                    //delete list[i];
					newlist[i] = list[i];
					
                } else {
                    newlist[i] = NULL;
                }
            }
            delete [] list;
            list = newlist;
            nelements = nactualelements;
			
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				unlock();
			#endif
		}
        
		Class *getRawElement(int i){
			return (getElement(i)).get();
		}
		
		
        /**
         * Returns a pointer to the object at index 'i'.
         */
shared_ptr<Class> getElement(int i) {
            #if M_THREAD_SAFE_EXPANDABLE_LISTS
				lock();
			#endif
			
			if(i >= nelements) { // FIX THIS PROBABLY
                // TODO dependency problem
				fprintf(stderr, "Error: attempt to access index (%d) greater than nelements (%d) in an ExpandableList structure",
                                                                i, nelements);
                fflush(stderr);
                i = nelements - 1;
            }
            if(i < 0) {
                // TODO dependency problem
				fprintf(stderr, "Error: attempt to access negative index on an expandable list (%d)", i);
                fflush(stderr); 
                i = 0;
            }
			
			shared_ptr<Class> return_pointer = list[i];
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				unlock();
			#endif
            return return_pointer;
        }
        
		/**
         * Places a copy of the object at index 'i' into the supplied memory
		 * location.  Should be used with care.
		 * Return false on failure, true on success
         */
		bool getElement(int i, Class *target) {
            #if M_THREAD_SAFE_EXPANDABLE_LISTS
				lock();
			#endif
			
			if(i >= nelements) { // FIX THIS PROBABLY
                // TODO dependency problem
				fprintf(stderr, "Error: attempt to access index (%d) greater than nelements (%d) in an ExpandableList structure",
                                                                i, nelements);
                fflush(stderr);
                i = nelements - 1;
            }
            if(i < 0) {
                // TODO dependency problem
				fprintf(stderr, "Error: attempt to access negative index on an expandable list (%d)", i);
                fflush(stderr); 
                i = 0;
            }
			
			shared_ptr<Class> return_pointer = list[i];
			
			if(return_pointer == NULL){
				#if M_THREAD_SAFE_EXPANDABLE_LISTS
					unlock();
				#endif
				return false;
			}
			
			*target = *return_pointer;
			
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				unlock();
			#endif
            return true;
        }
       
		
        /**
         * Sets the elements at index 'i' to the new value.  The old value
         * is deleted and the new value copied.
         */
        void setElement(int i, const Class value) {
			shared_ptr<Class> copied(new Class(value));
			setElement(i, copied);
		}
		 
		 // TODO: I'm not convinced that this method works properly yet...
		void setElement(int i, shared_ptr<Class> value) {
            #if M_THREAD_SAFE_EXPANDABLE_LISTS
				lock();
			#endif
			
			if((i < 0)) {
                // TODO dependency problem
				fprintf(stderr, "bad index in set element, should i throw an error?\n");
                fflush(stderr); 
                #if M_THREAD_SAFE_EXPANDABLE_LISTS
					unlock();
				#endif
				
				return;
            }
			if(i >= nelements) {
				expandList(i+10);
				nactualelements = i+1;
			}
		
            if(list[i] != NULL) {
                //delete (list[i]);
                //list[i] = NULL;
            }
            if(value == NULL) {
                //list[i] = NULL;
            } else {
                //mClass * copy = new Class(*value);
                //list[i] = copy;
				list[i] = value;
            }
			
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				unlock();
			#endif
        }

        /** 
         * Overloaded array access operator
         */
shared_ptr<Class> operator[](int i) {
            return list[i];
        }

        /**
         * Returns the number of actual objects stored in the list.
         */
        int getNElements() {
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				lock();
			#endif
			
			int returnval = nactualelements;
			
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				unlock();
			#endif
			
            return returnval;
        }

        /**
         * Deletes all the items in the list.
         */
        void clear() {
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				lock();
			#endif
			
            for(int i = 0; i < nactualelements; i++) {
                if(list[i] != NULL) {
					
					//shared_ptr<Class> value = list[i];
                    //delete list[i];
                    list[i] = shared_ptr<Class>();
                }
            }
            nactualelements = 0;
			
			#if M_THREAD_SAFE_EXPANDABLE_LISTS
				unlock();
			#endif
		}
};
}
#endif

 
