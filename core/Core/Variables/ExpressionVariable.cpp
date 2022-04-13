/*
 *  ExpressionVariable.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 2/20/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "ExpressionVariable.h"


BEGIN_NAMESPACE_MW


ParsedExpressionVariable::ParsedExpressionVariable(const stx::ParseTree &expression_tree) :
    expression_tree(expression_tree)
{
    //
    // We want to pre-evaluate the expression, in order to catch fatal errors (e.g. references to
    // unknown variables) at load time.  However, load-time evaluation may produce non-fatal errors,
    // because the expression is being evaluated outside of its context in the experiment.  (For
    // example, the expression may try to retrieve the first element of a list, but the contents
    // of the list aren't set until run time, leading to a load-time "index out of bounds" error
    // that never happens when the experiment runs.)  Therefore, we silence all messages on the
    // current thread while pre-evaluating the expression.
    //
    SilenceMessages sm;
    getValue();
}


END_NAMESPACE_MW
