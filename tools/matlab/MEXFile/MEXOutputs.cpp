//
//  MEXOutputs.cpp
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/17/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#include "MEXOutputs.h"


BEGIN_NAMESPACE_MW_MATLAB


void MEXOutputs::needMoreOutputs() {
    throwMATLABError("MWorks:NotEnoughOutputs", "Not enough output arguments");
}


END_NAMESPACE_MW_MATLAB
