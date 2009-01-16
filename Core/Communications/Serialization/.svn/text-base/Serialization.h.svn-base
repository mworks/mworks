/*
 *  Serialization.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 9/26/08.
 *  Copyright 2008 Harvard University. All rights reserved.
 *
 */
#ifndef _SERIALIZATION_H_
#define _SERIALIZATION_H_


#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/basic_binary_oprimitive.hpp>
#include <boost/archive/basic_binary_iprimitive.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/array.hpp>
#include "boost/serialization/serialization.hpp"

#include "ScarabServices.h"

namespace boost {
    namespace serialization {
        
        // These are some "shim" functions for serializing ScarabDatum structs
        // Ultimately, ScarabDatum's only exist for serialization, so if we 
        // move to an alternative serialization scheme (e.g. boost.serialization),
        // then there won't be any strong reason to keep Scarab around.  However,
        // best not to break everything at once.
        
        // ScarabDatum
        
        template<class Archive>
        void serialize(Archive & ar, ScarabDatum& d, const unsigned int version) 
        {
            split_free(ar, d, version);
        }
        
        template<class Archive>
        void save(Archive & ar, const ScarabDatum& d, const unsigned int version) 
        {
            ar << d.type;
            if(d.type == SCARAB_DICT){
                ar << *(d.data.dict);
            } else if(d.type == SCARAB_LIST){
                ar << *(d.data.list);
            } else if(d.type == SCARAB_OPAQUE){
                ar << d.data.opaque;
            } else if(d.type == SCARAB_INTEGER){
                ar << (long long)(d.data.integer);
            } else if(d.type == SCARAB_FLOAT){
                ar << (double)d.data.floatp;
            }
        }

        template<class Archive>
        void load(Archive & ar, ScarabDatum& d, const unsigned int version) 
        {
            ar >> d.type;
            if(d.type == SCARAB_DICT){
                ar >> *(d.data.dict);
            } else if(d.type == SCARAB_LIST){
                ar >> *(d.data.list);
            } else if(d.type == SCARAB_OPAQUE){
                ar >> d.data.opaque;
            } else if(d.type == SCARAB_INTEGER){
                long long val;
                ar >> val;
                d.data.integer = val;
            } else if(d.type == SCARAB_FLOAT){
                double val;
                ar >> val;
                d.data.floatp = val;
            }
        }
        
        
        // ScarabDict
        
        template<class Archive>
        void serialize(Archive & ar, ScarabDict& d, const unsigned int version){
            split_free(ar, d, version);
        }
        
        template<class Archive>
        void save(Archive & ar, const ScarabDict& d, const unsigned int version)
        {
            ar << d.size;
            ar << d.tablesize;
            
            for(int i = 0; i < d.size; i++){
                ar << d.keys[i];
                ar << d.values[i];
            }
        }

        template<class Archive>
        void load(Archive & ar, ScarabDict& d, const unsigned int version)
        {
            int dict_size, tablesize;
            shared_ptr< array<ScarabDatum *> > keys_array, values_array;
            ScarabDatum **keys, **values;
            
            ar >> dict_size;
            ar >> tablesize;
            
            keys = new ScarabDatum*[dict_size];
            values = new ScarabDatum*[dict_size];
            
            for(int i = 0; i < dict_size; i++){
                ScarabDatum *key;
                ScarabDatum *val;
                ar >> key;
                ar >> val;
                
                keys[i] = key;
                values[i] = val;
            }
            
            ScarabDatum *new_dict = scarab_dict_new(dict_size, &scarab_dict_times2);
            for(int i = 0; i < dict_size; i++){
                scarab_dict_put(new_dict, keys[i], values[i]);
            }
            
            d = *(new_dict->data.dict);
        }
        
        
        // ScarabList
        
        template<class Archive>
        void serialize(Archive & ar, ScarabList& d, const unsigned int version){
            split_free(ar, d, version);
        }
        
        template<class Archive>
        void save(Archive & ar, const ScarabList& d, const unsigned int version)
        {
            ar << d.size;
            for(int i = 0; i < d.size; i++){
                ar << d.values[i];
            }
        }
        
        template<class Archive>
        void load(Archive & ar, ScarabList& d, const unsigned int version)
        {
            int list_size;
            ScarabDatum **values;
            
            ar >> list_size;
            values = new ScarabDatum*[list_size];
            for(int i = 0; i < list_size; i++){
                ScarabDatum *value;
                ar >> value;
                values[i] = value;
            }
            
            ScarabDatum *new_list = scarab_list_new(list_size);
            for(int i = 0; i < list_size; i++){
                scarab_dict_put(new_list, scarab_new_integer(i), values[i]);
            }
            
            d = *(new_list->data.list);
        }
       
        // ScarabOpaque
        
        template<class Archive>
        void serialize(Archive & ar, ScarabOpaque& d, const unsigned int version){
            split_free(ar, d, version);
        }
        
        template<class Archive>
        void save(Archive & ar, const ScarabOpaque& d, const unsigned int version)
        {
            ar << d.size;
            ar << make_binary_object(d.data, d.size);
        }
        
        template<class Archive>
        void load(Archive & ar, ScarabOpaque& d, const unsigned int version)
        {
            int data_size;

            
            ar >> data_size;
            unsigned char *value = new unsigned char[data_size];
            binary_object value_object(value, data_size);
            
            ar >> value_object;
            
            d.size = data_size;
            d.data = value;
        }
        
        
        
    } // namespace serialization
} // namespace boost




#endif

