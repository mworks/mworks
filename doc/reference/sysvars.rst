.. _sysvars:

System Variables
================

In addition to being a central component of user-created experiments, `variables <Variable>` play a crucial role in the overall configuration and function of MWorks.  The following system-defined variables are always present (even before an experiment has been loaded) and serve a variety of purposes.


.. _config vars:

Configuration
-------------


#allowAltFailover
^^^^^^^^^^^^^^^^^

Controls MWorks' behavior when an `I/O device <Input/Output>` fails to initialize but specifies an alternative, "failover" device in its ``alt`` parameter.  If this variable is set to a true value (e.g ``1`` or ``true``), MWorks will attempt to use the failover device in place of the failed one.  Otherwise, the failover device is ignored.


.. _mainScreenInfo var:

#mainScreenInfo
^^^^^^^^^^^^^^^

The value of this variable is a dictionary whose key/value pairs describe and configure the stimulus display.

The following keys are recognized.  If a key is omitted, the default value is used:

.. list-table::
   :widths: auto
   :header-rows: 1

   * - Key
     - Description
     - Default value
   * - ``display_to_use``
     - Integer indicating which physical display to use as the main stimulus display.  0 indicates the primary display (e.g. where the macOS dock is located), 1 or greater selects a secondary display, and a negative value indicates that only the mirror window should be used (useful for testing experiments on a laptop).
     - ``0``
   * - ``width``
     - Width of the main display (in arbitrary units)
     - 
   * - ``height``
     - Height of the main display (in the same units as ``width``)
     - 
   * - ``distance``
     - Distance from the center of the main display to the experimental subject's eye (in the same units as ``width`` and ``height``)
     - 
   * - ``refresh_rate_hz``
     - Refresh rate of the main display in hertz (required, but currently unused)
     - 
   * - ``always_display_mirror_window``
     - True or false, indicating whether the stimulus display mirror window should be enabled or disabled.  When enabled, the mirror window "mirrors" the main stimulus display, allowing you to observe the stimulus presentation even if the main display is in another room or otherwise not visible.
     - ``false``
   * - ``mirror_window_base_height``
     - Height of the mirror window in `points <https://developer.apple.com/library/content/documentation/Cocoa/Conceptual/CocoaDrawingGuide/Transforms/Transforms.html#//apple_ref/doc/uid/TP40003290-CH204-SW5>`_.  The width is chosen to match the aspect ratio specified by ``width`` and ``height``.
     - 
   * - ``announce_individual_stimuli``
     - True or false, indicating whether stimuli should be announced individually when presented (via `#announceStimulus`_ events), in addition to being announced in `#stimDisplayUpdate`_ events
     - ``true``
   * - ``use_color_management``
     - True or false, indicating whether the stimulus display pipeline should perform `color management <https://en.wikipedia.org/wiki/Color_management>`_.  If color management is enabled, stimuli are rendered in a "linearized" `sRGB color space <https://en.wikipedia.org/wiki/SRGB>`_, and each frame is converted to the target display's color space before being presented.  Otherwise, rendering takes place in the (typically non-linear) color space of the display, and no color conversion is performed.
     - ``true``
   * - ``set_display_gamma``
     - True or false, indicating if the display's `gamma function <https://en.wikipedia.org/wiki/Gamma_correction>`_ should be set using the values specified by ``red_gamma``, ``green_gamma``, and ``blue_gamma``.  Applies only when ``use_color_management`` is false.
     - ``false``
   * - ``red_gamma``
     - Inverse of red channel gamma (must be greater than zero)
     - 
   * - ``green_gamma``
     - Inverse of green channel gamma (must be greater than zero)
     - 
   * - ``blue_gamma``
     - Inverse of blue channel gamma (must be greater than zero)
     - 
   * - ``make_window_opaque``
     - True or false, indicating whether the stimulus display window should be opaque.  If the window is not opaque, and the experiment includes a `Stimulus Display` device whose ``background_alpha_multiplier`` parameter is less than 1, then the display background and/or application windows beneath the stimulus window will show through it.  Does not affect the mirror window, which is always opaque.
     - ``true``

The ``width``, ``height``, and ``distance`` values are used to compute the angular field of view covered by the main display.  The bounds of the field of view (in degrees) are reported by MWorks when the experiment is loaded, e.g.::

    Display bounds set to (-26.4631 left, 26.4631 right, 16.5394 top, -16.5394 bottom)

Within an experiment, the bounds can be obtained via the ``display_bounds`` function.  Visual stimuli are drawn within these bounds, with their position and size given in degrees.

Changes to this variable affect the next experiment loaded.


#realtimeComponents
^^^^^^^^^^^^^^^^^^^

The value of this variable is a dictionary, where each key identifies a core realtime service (clock, scheduler, or state system), and the corresponding value is the name of a plugin that provides that service.

The following keys and values are allowed.  If a key is omitted, the default value is used:

.. list-table::
   :widths: auto
   :header-rows: 1

   * - Key
     - Default value
     - Alternative value(s)
   * - ``clock``
     - ``HighPrecisionClock``
     - ``SimpleClock``
   * - ``scheduler``
     - ``ZenScheduler``
     - 
   * - ``state_system``
     - ``ZenStateSystem``
     - 

Changes to this varible take effect when the server is restarted.


#serverName
^^^^^^^^^^^

The value of this variable is a string containing an arbitrary, user-specified name for the system running MWServer.  This name is displayed by MWClient when connected to the server.


#stopOnError
^^^^^^^^^^^^

If this variable is set to a true value, a running experiment will stop automatically when an error is reported.


#warnOnSkippedRefresh
^^^^^^^^^^^^^^^^^^^^^

If this variable is set to a true value, a warning is issued whenever the stimulus presentation skips a display refresh cycle.


Experiment State
----------------


#announceAssertion
^^^^^^^^^^^^^^^^^^

When an `assertion <Assert>` fails, the value of this variable is set to a string containing the associated error message.


.. _announceBlock var:

#announceBlock
^^^^^^^^^^^^^^

The initial value of this variable is 0.  When the running experiment enters a `block <Block>`, the value is increased by 1.  When execution of the block is complete, the value is decreased by 1.  Hence, the value indicates the number of (nested) blocks currently executing.


#announceCalibrator
^^^^^^^^^^^^^^^^^^^

Used to announce changes to the state of a `calibrator <Filters>`, e.g. acquistion of a new sample or recalculation of fit parameters.  The value is a dictionary containing the name of the calibrator, the type of update, and the relevant sample data or parameter values.


#announceCurrentState
^^^^^^^^^^^^^^^^^^^^^

When the experiment is running, this variable contains the numeric identifier of the component (i.e. `paradigm component <Paradigm Components>` or `action <Actions>`) that is currently executing.  The component codec provides the mapping between numeric component identifiers and component names.


#announceMessage
^^^^^^^^^^^^^^^^

Used to log messages (generated by the system or the `user <Report / Message>`) to the event stream.  The value of this variable is a dictionary containing the message text, plus some metadata for internal use.


#announceSound
^^^^^^^^^^^^^^

Used to announce that a sound has started playing.  The value of this variable is a dictionary containing the name and parameters of the relevant `sound stimulus <Sounds>`.


#announceStimulus
^^^^^^^^^^^^^^^^^

Used to announce the display of a visual stimulus.  The value of this variable is a dictionary containing the name and parameters of the relevant `stimulus <Stimuli>`.  The time stamp of an ``#announceStimulus`` event is the operating system's best guess for when the rendered frame containing the stimulus will start to appear on the display.

*Note*: ``#announceStimulus`` and `#stimDisplayUpdate`_ report exactly the same information.  The only difference is that the latter reports details of *all* currently-displayed stimuli in a single value.  To eliminate this redundancy (and reduce the size of the event file), set the ``announce_individual_stimuli`` key of `#mainScreenInfo`_ to ``false``.


.. _announceTrial var:

#announceTrial
^^^^^^^^^^^^^^

The initial value of this variable is 0.  When the running experiment enters a `trial <Trial>`, the value is increased by 1.  When execution of the trial is complete, the value is decreased by 1.  Hence, the value indicates the number of (nested) trials currently executing.


#loadedExperiment
^^^^^^^^^^^^^^^^^

Used to record the source file(s) of the currently-loaded experiment to the event stream and event file.

If the current experiment was loaded from an XML source file, the value of this variable is a string containing the file's content.

If the current experiment was loaded from an `MWEL <MWEL>` source file, the value of this variable is a dictionary.  The keys in the dictionary are file paths, and the values are strings containing the corresponding file's content.  The dictionary includes entries for both the primary source file and any files it `includes <includes>` (either directly or indirectly).


#state_system_mode
^^^^^^^^^^^^^^^^^^

Used to announce the execution state of the current experiment.  The value of this variable is an integer, corresponding to one of four possible states:

.. list-table::
   :widths: auto
   :header-rows: 1

   * - Value
     - State
     - Description
   * - ``0``
     - Idle
     - Experiment is not executing
   * - ``1``
     - Stopping
     - Experiment execution is ending.  `I/O devices <Input/Output>` and other components should perform shutdown and cleanup tasks as needed.
   * - ``2``
     - Running
     - Experiment is currently executing
   * - ``3``
     - Paused
     - Experiment is executing, but execution is currently paused.  `Stimulus <Stimuli>` presentation and `sound <Sounds>` playback are paused, but `I/O devices <Input/Output>` continue to operate normally.


.. _stimDisplayUpdate var:

#stimDisplayUpdate
^^^^^^^^^^^^^^^^^^

Used to announce updates to the visual stimulus presentation.

The value of this variable is a list.  Each element in the list is a dictionary containing the name and parameters of a `stimulus <Stimuli>` that is currently being displayed.  The order of the list matches the draw order of the stimuli.

The time stamp of a ``#stimDisplayUpdate`` event is the operating system's best guess for when the rendered frame containing the reported stimuli will start to appear on the display.


Internal
--------


#experimentLoadProgress
^^^^^^^^^^^^^^^^^^^^^^^

Used to provide MWClient with an estimate of the fraction of an experiment that has been loaded.


#requestCalibrator
^^^^^^^^^^^^^^^^^^

Used by MWClient's eye calibrator window to request updates to a calibrator's parameters.


.. _system event codes:

System Event Codes
^^^^^^^^^^^^^^^^^^

The following event codes have no associated variables but do appear in the event stream and event file:

.. list-table::
   :widths: auto
   :header-rows: 1

   * - Code
     - Internal name
     - Description
   * - 0
     - RESERVED_CODEC_CODE
     - Used to announce the variable codec (i.e. the mapping from event codes to variables)
   * - 1
     - RESERVED_SYSTEM_EVENT_CODE
     - Used for internal communication between MWServer and MWClient
   * - 2
     - RESERVED_COMPONENT_CODEC_CODE
     - Used to announce the component codec (i.e. the mapping from numeric component identifiers to component names)
   * - 3
     - RESERVED_TERMINATION_CODE
     - Last event sent to the event stream or recorded to the event file before the stream or file is closed.  Has no associated value.
