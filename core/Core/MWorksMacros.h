/*
 *  MWorksMacros.h
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/23/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#ifndef _MWORKS_MACROS_H
#define _MWORKS_MACROS_H


//
// Use of the following prevents syntax-aware editors from indenting everything in a namespace block
//

#ifndef BEGIN_NAMESPACE
#define BEGIN_NAMESPACE(name)   namespace name {
#endif

#ifndef END_NAMESPACE
#define END_NAMESPACE(name)     }
#endif

#ifndef BEGIN_NAMESPACE_MW
#define BEGIN_NAMESPACE_MW      BEGIN_NAMESPACE(mw)
#endif

#ifndef END_NAMESPACE_MW
#define END_NAMESPACE_MW        END_NAMESPACE(mw)
#endif


//
// MW_NOEXCEPT and MW_OVERRIDE expand to the C++11 "noexcept" and "override" keywords, respectively, if the compiler
// and current language standard support them.  Otherwise, they expand to nothing but still serve to document the
// programmer's intent.
//

#ifdef __clang__
#  if __has_feature(cxx_noexcept)
#    define MW_NOEXCEPT noexcept
#  endif
#  if __has_feature(cxx_override_control)
#    define MW_OVERRIDE override
#  endif
#endif  // __clang__

#ifndef MW_NOEXCEPT
#  define MW_NOEXCEPT
#endif
#ifndef MW_OVERRIDE
#  define MW_OVERRIDE
#endif


#endif  // _MWORKS_MACROS_H


























