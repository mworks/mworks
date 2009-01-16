/*!
 * @header StandardServerCoreBuilder.h
 *
 * @discussion 
 *
 * History:<BR/>
 * paul on 1/20/06 - Created.<BR/>
 * 
 * @copyright MIT
 * @updated 1/20/06
 * @version 1.0.0
 */

#ifndef _STANDARD_SERVER_CORE_BUILDER_H__
#define _STANDARD_SERVER_CORE_BUILDER_H__

#include "AbstractCoreBuilder.h"
namespace mw {
class StandardServerCoreBuilder : public AbstractCoreBuilder {
    public:
        /*!
         * @function StandardServerCoreBuilder
         * @discussion Constructor does nothing.
         */
        StandardServerCoreBuilder();

        /*!
         * @function ~StandardServerCoreBuilder
         * @discussion Destructor
         */
        virtual ~StandardServerCoreBuilder();

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
         * @discussion Initializes buffers to 10,000
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool initializeEventBuffers();

        /*!
         * @function initializeRegistries
         * @discussion Calls initializeServiceRegistries.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool initializeRegistries();

        /*!
         * @function initializeGlobalParameters
         * @discussion Initializes standard variables and also initializes the
         * global scarab memory context.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool initializeGlobalParameters();

        /*!
         * @function loadPlugins
         * @discussion Loads the ZenScheduler, MachClock and 
         * ZenStateSystem plugins
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool loadPlugins();

        /*!
         * @function chooseRealtimeComponents
         * @discussion Chooses the default scheduler, clock, and state system.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool chooseRealtimeComponents();

        /*!
         * @function startInterpreters
         * @discussion Starts the global perl interpreter.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool startInterpreters();

        /*!
         * @function startRealtimeServices
         * @discussion Starts the global clock.
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