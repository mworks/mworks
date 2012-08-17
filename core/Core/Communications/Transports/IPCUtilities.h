//
//  IPCUtilities.h
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/17/12.
//
//

#ifndef MWorksCore_IPCUtilities_h
#define MWorksCore_IPCUtilities_h

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/cstdint.hpp>
#include <boost/interprocess/detail/utilities.hpp>

#include "MWorksMacros.h"


BEGIN_NAMESPACE(boost)
BEGIN_NAMESPACE(interprocess)
BEGIN_NAMESPACE(ipcdetail)


//
// Without the following template, the definitions of offset_ptr and message_queue in the message_queue_32_64
// namespace result in compilation failures (for both i386 and x86_64) identical to the one described at
// https://svn.boost.org/trac/boost/ticket/6147 .  Adding this variant of get_rounded_size eliminates the issue.
//

template<class SizeType, class OtherSizeType>
inline SizeType get_rounded_size(SizeType orig_size, OtherSizeType round_to)
{
    return get_rounded_size(orig_size, SizeType(round_to));
}


END_NAMESPACE(ipcdetail)
END_NAMESPACE(interprocess)
END_NAMESPACE(boost)


// We must include message_queue.hpp *after* our implementation of get_rounded_size
#include <boost/interprocess/ipc/message_queue.hpp>


BEGIN_NAMESPACE_MW
BEGIN_NAMESPACE(message_queue_32_64)


//
// The following typedefs define a message queue capable of passing messages between 32- and 64-bit processes
// (as described at https://svn.boost.org/trac/boost/ticket/5230 ).  The archive typedefs are included because
// not all archive types are portable between architectures; in particular, boost::archive::binary_oarchive
// and boost::archive::binary_iarchive are not, as noted at
// http://www.boost.org/doc/libs/1_50_0/libs/serialization/doc/archives.html#archive_models .
//

typedef boost::interprocess::offset_ptr<void, boost::int32_t, boost::uint64_t> offset_ptr;
typedef boost::interprocess::message_queue_t<offset_ptr> message_queue;
typedef boost::archive::text_iarchive iarchive;
typedef boost::archive::text_oarchive oarchive;


END_NAMESPACE(message_queue_32_64)
END_NAMESPACE_MW


#endif


























