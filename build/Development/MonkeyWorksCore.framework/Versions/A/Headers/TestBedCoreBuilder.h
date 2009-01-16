#ifndef __TEST_BED_CORE_BUILDER_H__
#define __TEST_BED_CORE_BUILDER_H__

/*
 *  TestBedCoreBuilder.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/25/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "StandardServerCoreBuilder.h"
namespace mw {

class TestBedCoreBuilder : public StandardServerCoreBuilder {

 private:
 
 public:
/*!
         * @function startInterpreters
         * @discussion Would normally start a Perl Interpreter, but in this
		 *			   stripped down version, it does not
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool startInterpreters();

};
}
#endif