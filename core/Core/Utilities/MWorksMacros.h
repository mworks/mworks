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


#if defined(__GNUC__) || defined(__clang__)
#  define MW_SYMBOL_PUBLIC __attribute__((visibility("default")))
#else
#  define MW_SYMBOL_PUBLIC
#endif


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
#define BEGIN_NAMESPACE_MW      namespace mw MW_SYMBOL_PUBLIC {
#endif

#ifndef END_NAMESPACE_MW
#define END_NAMESPACE_MW        }
#endif


//
// MW_NOEXCEPT and MW_OVERRIDE expand to the C++11 "noexcept" and "override" keywords, respectively, if the compiler
// and current language standard support them.  Otherwise, they expand to nothing but still serve to document the
// programmer's intent.
//

#ifdef __clang__
#  if __has_feature(cxx_noexcept)
#    define MW_HAVE_NOEXCEPT
#  endif
#  if __has_feature(cxx_override_control)
#    define MW_HAVE_OVERRIDE
#  endif
#endif  // __clang__

#ifdef MW_HAVE_NOEXCEPT
#  define MW_NOEXCEPT noexcept
#else
#  define MW_NOEXCEPT
#endif

#ifdef MW_HAVE_OVERRIDE
#  define MW_OVERRIDE override
#else
#  define MW_OVERRIDE
#endif


#endif  // _MWORKS_MACROS_H


























