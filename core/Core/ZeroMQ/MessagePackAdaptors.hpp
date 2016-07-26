//
//  MessagePackAdaptors.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 7/28/16.
//
//

#ifndef MessagePackAdaptors_hpp
#define MessagePackAdaptors_hpp

#include <msgpack.hpp>

#include "GenericData.h"


//
// Relevant msgpack documentation: https://github.com/msgpack/msgpack-c/wiki/v2_0_cpp_adaptor
//


namespace msgpack {
    MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
        namespace adaptor {
            
            
            template <>
            struct pack<mw::Datum> {
                template <typename Stream>
                msgpack::packer<Stream>& operator()(msgpack::packer<Stream> &packer, const mw::Datum &datum) const {
                    switch (datum.getDataType()) {
                            
                        case mw::M_INTEGER:
                            packer.pack(datum.getInteger());
                            break;
                            
                        case mw::M_FLOAT:
                            packer.pack(datum.getFloat());
                            break;
                            
                        case mw::M_BOOLEAN:
                            packer.pack(datum.getBool());
                            break;
                            
                        case mw::M_STRING: {
                            auto &str = datum.getString();
                            if (datum.stringIsCString()) {
                                packer.pack(str);
                            } else {
                                packer.pack_bin(str.size());
                                packer.pack_bin_body(str.data(), str.size());
                            }
                            break;
                        }
                            
                        case mw::M_LIST:
                            packer.pack(datum.getList());
                            break;
                            
                        case mw::M_DICTIONARY:
                            packer.pack(datum.getDict());
                            break;
                            
                        default:
                            packer.pack_nil();
                            break;
                            
                    }
                    
                    return packer;
                }
            };
            
            
            template <>
            struct convert<mw::Datum> {
                const msgpack::object& operator()(const msgpack::object &object, mw::Datum &datum) const {
                    switch (object.type) {
                            
                        case msgpack::type::NIL:
                            datum = mw::Datum();
                            break;
                            
                        case msgpack::type::BOOLEAN:
                            datum = mw::Datum(object.via.boolean);
                            break;
                            
                        case msgpack::type::POSITIVE_INTEGER:
                        case msgpack::type::NEGATIVE_INTEGER:
                            // object.as() will throw if the value won't fit in a long long
                            datum = mw::Datum(object.as<long long>());
                            break;
                            
                        case msgpack::type::FLOAT:
                            datum = mw::Datum(object.via.f64);
                            break;
                            
                        case msgpack::type::STR: {
                            auto &str = object.via.str;
                            datum = mw::Datum(str.ptr, str.size);
                            break;
                        }
                            
                        case msgpack::type::BIN: {
                            auto &bin = object.via.bin;
                            datum = mw::Datum(bin.ptr, bin.size);
                            break;
                        }
                            
                        case msgpack::type::ARRAY: {
                            auto &array = object.via.array;
                            mw::Datum::list_value_type list;
                            list.reserve(array.size);
                            for (std::size_t i = 0; i < array.size; i++) {
                                list.push_back(array.ptr[i].as<mw::Datum>());
                            }
                            datum = mw::Datum(std::move(list));
                            break;
                        }
                            
                        case msgpack::type::MAP: {
                            auto &map = object.via.map;
                            mw::Datum::dict_value_type dict;
                            dict.reserve(map.size);
                            for (std::size_t i = 0; i < map.size; i++) {
                                auto &kv = map.ptr[i];
                                dict[kv.key.as<mw::Datum>()] = kv.val.as<mw::Datum>();
                            }
                            datum = mw::Datum(std::move(dict));
                            break;
                        }
                            
                        default:
                            throw msgpack::type_error();
                            
                    }
                    
                    return object;
                }
            };
            
            
        }
    }
}


#endif /* MessagePackAdaptors_hpp */


























