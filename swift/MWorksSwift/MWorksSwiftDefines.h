//
//  MWorksSwiftDefines.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/6/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef MWorksSwiftDefines_h
#define MWorksSwiftDefines_h

#ifdef __cplusplus
#define MWORKSSWIFT_EXTERN extern "C"
#else
#define MWORKSSWIFT_EXTERN extern
#endif

#define MWORKSSWIFT_PUBLIC_CLASS __attribute__((visibility("default")))
#define MWORKSSWIFT_PUBLIC_EXTERN MWORKSSWIFT_EXTERN __attribute__((visibility("default")))

#endif /* MWorksSwiftDefines_h */
