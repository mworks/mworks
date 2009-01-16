/*!
 * AbstractCoreBuilder.h
 *
 * Discussion Abstract base class for subclasses to implement so that they
 * can initialize the core.  Should think perhaps about adding a reference 
 * to some sort of object of structure to return error information in.
 *
 * NOTE: The order in which the functions are declared is the order in which
 * they should be called.  Ill make a foreman to do all the calling though.
 *
 * Histroy:
 * paul on 1/20/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#ifndef _ABSTRACT_CORE_BUILDER_H__
#define _ABSTRACT_CORE_BUILDER_H__

#include "ComponentRegistry_new.h"

namespace mw {

class AbstractCoreBuilder {
    public:
        /*!
         * @function AbstractCoreBuilder
         * @discussion Constructor does nothing.
         */
        AbstractCoreBuilder();

        /*!
         * @function buildProcessWillStart
         * @discussion Allows object to do any pre-processing before any other
         * methods are called.  Use this method to initialize any objets you
         * may need during building or for sending events... or for nothing.
         * 
         * @result True if everything went OK false otherwise.
         */
        virtual bool buildProcessWillStart()        =0;

        /*!
         * @function initializeEventBuffers
         * @discussion Use this method to initialize any event buffers in 
         * the system.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool initializeEventBuffers()       =0;

        /*!
         * @function initializeRegistries
         * @discussion Use this method to initialize any service registries.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool initializeRegistries()         =0;

        /*!
         * @function initializeGlobalParameters
         * @discussion Use this method to initialize any global parameters 
         * that are needed in the system.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool initializeGlobalParameters()   =0;

        /*!
         * @function loadPlugins
         * @discussion Load any plugins that the system wants to use.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool loadPlugins()                  =0;

        /*!
         * @function chooseRealtimeComponents
         * @discussion Use this method to choose the realtime components
         * that you want the system to use.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool chooseRealtimeComponents()     =0;

        /*!
         * @function startInterpreters
         * @discussion Start any language interpreters.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool startInterpreters()            =0;

        /*!
         * @function startRealtimeServices
         * @discussion Start any realtime services.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool startRealtimeServices()        =0;

        /*!
         * @function customInitialization
         * @discussion Use this to initialize any other components that you
         * may have added to the core.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool customInitialization()         =0;

        /*!
         * @function buildProcessWillEnd
         * @discussion Use this method to inform any interested parties that
         * the core is setup and ready to go.
         *
         * @result True if everything went OK false otherwise.
         */
        virtual bool buildProcessWillEnd()          =0;
        
        /*!
         * @function ~AbstractCoreBuilder
         * @discussion Destructor.
         */
        virtual ~AbstractCoreBuilder();
};
}
#endif