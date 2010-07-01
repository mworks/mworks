/*!
 * @header CoreBuilderForeman.h
 *
 * @discussion Directs teh building of the core.
 *
 * History:<BR/>
 * paul on 1/20/06 - Created.<BR/>
 * 
 * @copyright MIT
 * @updated 1/20/06
 * @version 1.0.0
 */

#ifndef CORE_BUILDER_FOREMAN_H__
#define CORE_BUILDER_FOREMAN_H__

#include "AbstractCoreBuilder.h"
namespace mw {
class CoreBuilderForeman {

    public:
        /*!
         * @function constructCoreStandardOrder
         * @discussion Uses the supplied builder to setup the core and get
         * it ready to run experiments.  The builder methods are called in
         * the following order:
         *      buildProcessWillStart()
         *      initializeEventBuffers()
         *      initializeRegistries()
         *      initializeGlobalParameters()
         *      loadPlugins()
         *      chooseRealtimeComponents()
         *      startInterpreters()
         *      startRealtimeServices()
         *      customInitialization()
         *      buildProcessWillEnd()
         * If any of the methods fail then this method will return false.
         *
         * @param builder An instance of a builder object
         * @result True if all methods completed successfully false otherwise.
         */
        static bool constructCoreStandardOrder(AbstractCoreBuilder * builder);
};
}
#endif