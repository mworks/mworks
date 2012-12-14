//
//  Converters.h
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/14/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#ifndef __MATLABTools__Converters__
#define __MATLABTools__Converters__

#include <MWorksCore/GenericData.h>

#include "Utilities.h"


BEGIN_NAMESPACE_MW_MATLAB


ArrayPtr convertDatumToArray(const Datum &datum);


END_NAMESPACE_MW_MATLAB


#endif  // !defined(__MATLABTools__Converters__)
