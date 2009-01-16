/*!
 * @header StandardClientCoreBuilder.h
 *
 * @discussion 
 *
 * History:<BR/>
 * paul on 2/7/06 - Created.<BR/>
 * 
 * @copyright MIT
 * @updated 2/7/06
 * @version 1.0.0
 */

#ifndef _STANDARD_CLIENT_CORE_BUILDER_H__
#define _STANDARD_CLIENT_CORE_BUILDER_H__

#include "AbstractCoreBuilder.h"
namespace mw {
class StandardClientCoreBuilder : public AbstractCoreBuilder {
    public:
        /*!
         * @function StandardClientCoreBuilder
         * @discussion Constructor does nothing.
         */
        StandardClientCoreBuilder();

        /*!
         * @function ~StandardClientCoreBuilder
         * @discussion Destructor
         */
        virtual ~StandardClientCoreBuilder();

    // AbstractCoreBuilder methods
    public:
        /*!
         * @function buildProcessWillStart
         * @discussion Does nothing.
         * 
         * @result True if everything went OK false otherwise.
         */
        virtual bool buildProcessWillStart();

        /*!
         * @function initializeEventBuffers
         * @discussion 
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool initializeEventBuffers();

        /*!
         * @function initializeRegistries
         * @discussion 
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool initializeRegistries();

        /*!
         * @function initializeGlobalParameters
         * @discussion 
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool initializeGlobalParameters();

        /*!
         * @function loadPlugins
         * @discussion 
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool loadPlugins();

        /*!
         * @function chooseRealtimeComponents
         * @discussion 
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool chooseRealtimeComponents();

        /*!
         * @function startInterpreters
         * @discussion 
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool startInterpreters();

        /*!
         * @function startRealtimeServices
         * @discussion 
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool startRealtimeServices();

        /*!
         * @function customInitialization
         * @discussion Ignores SIGPIPE.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool customInitialization();

        /*!
         * @function buildProcessWillEnd
         * @discussion Does nothing.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool buildProcessWillEnd();

};
}
#endif

