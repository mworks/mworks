Designing an Experiment
=======================

For both new and experienced MWorks users, designing an experiment can be a daunting task.  This guide attempts to break down and explain the elements of experimental design, both to help you better understand the working of existing experiments and to provide you with a mental framework for creating your own.

*Note*: The examples included in and referred to by this guide all use `MWEL <MWEL>`, which is recommended for new experiments.  However, the ideas and tools described here are equally applicable to XML-based experiments created or modified in MWEditor.  Any example files referred to by name can be found in the directory ``/Library/Application Support/MWorks/Examples``.


Data Flow
---------


Variables
^^^^^^^^^

`Variables <Variable>` play several essential roles in MWorks experiments.

Like the variables you may know from scripting or programming languages, MWorks variables associate names with values.  You can use them to store experiment parameters and record results.  Every `assignment <Assign Variable>` to a variable generates an event that is recorded in the event file and can be `extracted for analysis <Analyzing Experimental Data>`.

In addition to their data storage role, variables also enable the flow of information between different MWorks `components <Components>`.  This is described in more detail in the next section.


I/O Devices and Filters
^^^^^^^^^^^^^^^^^^^^^^^

MWorks interacts with the outside world via `input/output <Input/Output>` (I/O) devices.

Each time an input device receives new data, it assigns the relevant value(s) to one or more variables.  The target variables may be read directly by other parts of the experiment, or they can serve as inputs to `filters <Filters>`, which perform additional data processing and output new values (again via variables).

Conversely, when an experiment needs to send data to or perform an action in the outside world, it assigns values to one or more variables associated with an output device.  The output device watches those variables, and whenever one of them is assigned a new value, it takes appropriate action based on that value.

As an example, consider the typical flow of eye-tracking data in an MWorks experiment.  Raw eye positions are received from an eye tracker (e.g. an `EyeLink <EyeLink Device>`) and assigned to variables::

    var eye_h_raw = 0
    var eye_v_raw = 0
    
    iodevice/eyelink eyelink (
        pupil_lx = eye_h_raw
        pupil_ly = eye_v_raw
        tracker_ip = '100.1.1.1'
        tracking_dist = 1024
        data_interval = 1ms
        )

Next, the raw positions are used as input to an `eye calibrator <Eye Calibrator>`, which applies a calibration and outputs the results::

    var eye_h_calibrated = 0
    var eye_v_calibrated = 0
    
    calibrator/standard_eye_calibrator eye_calibrator (
        eyeh_raw = eye_h_raw
        eyev_raw = eye_v_raw
        eyeh_calibrated = eye_h_calibrated
        eyev_calibrated = eye_v_calibrated
        )

Each component of the calibrated eye position then passes through a `boxcar filter <Box-car Filter>`, which computes a moving average of its input::

    var eye_h = 0
    var eye_v = 0
    
    filter/boxcar_filter_1d (
        in1 = eye_h_calibrated
        out1 = eye_h
        width_samples = 5
        )
    
    filter/boxcar_filter_1d (
        in1 = eye_v_calibrated
        out1 = eye_v
        width_samples = 5
        )

Finally, the averaged eye coordinates serve as input to two additional components: a `fixation point <Circular Fixation Point Stimulus>`, which reports whether the eye position lies within its target region, and an `eye monitor <Eye Monitor>`, which uses the coordinates to detect and report saccades::

    var eye_on_fixation_point = false
    
    stimulus/circular_fixation_point fixation_point (
        trigger_watch_x = eye_h
        trigger_watch_y = eye_v
        trigger_width = 2
        trigger_flag = eye_on_fixation_point
        x_size = 0.4
        )

    var eye_in_saccade = false
    
    filter/basic_eye_monitor (
        eyeh_calibrated = eye_h
        eyev_calibrated = eye_v
        eye_state = eye_in_saccade
        width_samples = 5
        saccade_entry_speed = 60
        saccade_exit_speed = 20
        )

The final outputs of the eye-tracking pipeline are the values of the variables ``eye_on_fixation_point`` and ``eye_in_saccade``.  These values, in turn, are used elsewhere in the experiment.  For example, they may be used in a `conditional transition <Conditional Transition>`::

    transition/conditional (
        target = 'Start fixation'
        condition = eye_on_fixation_point and (not eye_in_saccade)
        )


Control Flow
------------


Protocols
^^^^^^^^^

`Protocols <Protocol>` encompass all the runtime logic of an MWorks experiment.  To "run" an experiment really means to execute one or more of its protocols.

A protocol is a container for other components.  Its child components can be simple `actions <Actions>`, or they can be other container components (`blocks <Block>`, `trials <Trial>`, `task systems <Task System>`, etc.) with their own children.

The example experiment ``HelloWorld.mwel`` contains three protocols.  They range in complexity from extremely simple (a single action) to moderately complex (a task system and multiple trials).

*Note*: The components of an experiment that are defined outside of any protocol (variables, I/O devices, visual stimuli, etc.) are shared by *all* protocols in the experiment.


Blocks, Trials, and Lists
^^^^^^^^^^^^^^^^^^^^^^^^^

`Blocks <Block>`, `trials <Trial>`, and `lists <List>` provide high-level structure within a protocol.

Functionally, they are nearly identical.  All three are containers for other  components, and all support `selection <selection>`-based execution of their children.  They differ only in that entry to and exit from a block or trial is announced via a system variable (`announceBlock var` for blocks, `announceTrial var` for trials), whereas no such announcement is made for lists.

Despite their suggestive names, MWorks makes no assumptions about the content and usage of these components.  Your experiment may employ them in any way that makes sense to you.


Task Systems
^^^^^^^^^^^^

`Task systems <Task System>` are the most powerful and flexible tool that MWorks' provides for managing the flow of control within an experiment.  The core execution logic of a protocol is typically implemented as a task system.

A task system is a form of `finite state machine <https://en.wikipedia.org/wiki/Finite-state_machine>`_.  It is composed of one or more `states <Task System State>`, each of which contains both `actions <Actions>` and `transitions <Transitions>`.

Execution of a task system begins with its first state (the *start state*).  First, all of the state's associated actions are executed.  Next, the transitions contained in the state are evaluated one by one, repeatedly if needed, until one of them succeeds.  Finally, execution of the task system transfers to the successful transition's target state, and the process repeats.  The flow of control from state to state continues until a `yield transition <Exit Task System>` succeeds, at which point execution of the task system ends.

As an example, consider the task system in ``FindTheCircle.mwel``.  After displaying three colored squares on screen, the experiment enters a state called "Wait for selection".  This state contains a `Start Timer` action, followed by four transitions.  The first three transitions succeed when the subject selects the red, green, or blue square, respectively, while the fourth succeeds if the timer expires before any selection is made::

    state 'Wait for selection' {
        start_timer (
            timer = selection_timer
            duration = selection_timeout
            )

        // Handle each possible selection
        goto (
            target = 'Red selected'
            when = pointer_on_red_square
            )
        goto (
            target = 'Green selected'
            when = pointer_on_green_square
            )
        goto (
            target = 'Blue selected'
            when = pointer_on_blue_square
            )

        // If the timeout expires, go to state "No selection"
        goto (
            target = 'No selection'
            when = timer_expired(selection_timer)
            )
    }

If the subject selects a square, the task system proceeds to a state associated with the selected color.  This state records the selection in a variable, then chooses the next state based on whether the selection was correct.  For example::

    state 'Red selected' {
        red_selected = true

        goto (
            target = 'Correct selection'
            when = circle_x < 0
            )
        goto ('Incorrect selection')
    }

Finally, the states "Correct selection", "Incorrect selection", and "No selection" each record the corresponding outcome and play an appropriate sound before proceeding, unconditionally, to the next state::

    state 'Correct selection' {
        num_correct += 1
        play_sound (correct_sound)

        goto ('Reveal circle')
    }

    state 'Incorrect selection' {
        num_incorrect += 1
        play_sound (incorrect_sound)

        goto ('Reveal circle')
    }

    state 'No selection' {
        num_ignored += 1
        play_sound (ignored_sound)

        goto ('End trial')
    }


If, If/Else, and While
^^^^^^^^^^^^^^^^^^^^^^

For the most part, `actions <Actions>` are the "atoms" of an experiment's execution logic.  They perform a single task (e.g. `starting an I/O device <Start IO Device>` or `playing a sound <Play Sound>`) and have no child components.  However, a few actions defy these constraints and serve as tools of control flow.

An `if <Conditionally Execute Actions (if)>` action tests a condition expression.  If the expression evaluates to a true value (e.g. ``1``, ``true``), the ``if`` action then executues its child actions::

    if (red_selected) {
        report ('Subject chose red')
    }

An `if_else <Conditional Branching (if/else)>` action encloses one or more ``if`` actions and at most one `else <Unconditionally Execute Actions (else)>` action.  It tests its ``if`` actions one by one and executes the first whose condition is true.  If all the conditions are false, the ``if_else`` will execute its ``else`` if present; otherwise, it does nothing::

    if_else {
        if (red_selected) {
            report ('Subject chose red')
        }
        if (green_selected) {
            report ('Subject chose green')
        }
        if (blue_selected) {
            report ('Subject chose blue')
        }
        else {
            report ('Subject made no selection')
        }
    }

Finally, a `while <Repeat Actions (while)>` action is similar to an ``if`` in that it tests a condition and, if the condition is true, evaluates its child actions.  However, a ``while`` will repeat this process until its condition becomes false::

    index = 0
    while (index < num_images) {
        queue_stimulus (images[index])
        index += 1
    }


Visual Stimuli and Display Management
-------------------------------------

Throughout its history, MWorks' primary users have been researchers studying vision.  Because of this, MWorks provides a rich set of `visual stimuli <Stimuli>` and tools for controlling their presentation on a display.


Declaring Stimuli
^^^^^^^^^^^^^^^^^

Like variables and I/O devices, the stimuli in an MWorks experiment are declared outside of all protocols (and, therefore, are available to *all* protocols)::

    stimulus/circle red_circle (
        x_size = 2
        color = 1,0,0
        )

Stimuli can be declared individually, as above, or as members of a `stimulus group <Stimulus Group>`::

    stimulus_group circles {
        circle (
            x_size = 2
            x_position = -1
            color = 1,0,0  // Red
            )
        circle (
            x_size = 2
            x_position = 0
            color = 0,1,0  // Green
            )
        circle (
            x_size = 2
            x_position = 1
            color = 0,0,1  // Blue
            )
    }

A stimulus declared individually can be referred to by its tag, e.g. ``red_circle``.  A stimulus declared in a stimulus group can be referenced either via its own tag (if present) or by using the group's tag and a zero-based index, e.g. ``circles[0]``, ``circles[2]``.


Queuing and Dequeuing Stimuli
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To display a stimulus, you must first add it to the display queue via the `Queue Stimulus` action::

    queue_stimulus (red_circle)

To display multiple stimuli simultaneously, queue the stimuli in *back-to-front* order.  For example, the following will result in the red circle being drawn first, followed by the green circle, followed by the blue circle.  Since the red and green circles overlap, and the green circle is queued *after* the red circle, the green circle will partly cover the red one.  Similarly, the blue circle will partly cover the green one::

    queue_stimulus (circles[0])
    queue_stimulus (circles[1])
    queue_stimulus (circles[2])

Changing the queuing order also changes the drawing order.  For example, the following will result in the green circle partly covering *both* the red and blue ones::

    queue_stimulus (circles[0])
    queue_stimulus (circles[2])
    queue_stimulus (circles[1])

When all the desired stimuli are queued, you commit your changes and trigger a display update with the `Update Stimulus Display` action::

    update_display ()

To remove a stimulus from the display, you must first dequeue it with the `Dequeue Stimulus` action.  As with queuing, multiple stimuli can be dequeued at the same time, and ``update_display`` commits your changes.  For example, the following will remove the red and blue circles from the display, but the green one will still be visible::

    dequeue_stimulus (circles[0])
    dequeue_stimulus (circles[2])
    update_display ()

By combining queue and dequeue actions, you can both add and remove stimuli in a single display update::

    // Remove green circle and restore red and blue ones
    dequeue_stimulus (circles[1])
    queue_stimulus (circles[0])
    queue_stimulus (circles[2])
    update_display ()


.. _Understanding Display Updates:

Understanding Display Updates
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Computer displays typically update at a fixed `refresh rate <https://en.wikipedia.org/wiki/Refresh_rate>`_.  For example, a display with a 60Hz refresh rate will redraw itself approximately once every 16.67 milliseconds.

To optimize graphics performance and avoid visual artifacts like `screen tearing <https://en.wikipedia.org/wiki/Screen_tearing>`_, MWorks performs all stimulus-drawing operations in synchrony with the display's refresh cycle.  Specifically, all drawing code executes on a dedicated operating system `thread <https://en.wikipedia.org/wiki/Thread_(computing)>`_, independent of the thread on which the running protocol executes, and rendered frames are transferred to the display hardware only during the `vertical blanking interval <https://en.wikipedia.org/wiki/Vertical_blanking_interval>`_, which occurs once per refresh period.

Contrary to what you might expect, the completion of an `Update Stimulus Display` action does *not* signal that the display has actually been updated.  Rather, it indicates only that all stimulus drawing commands have been submitted to the graphics hardware, and that their effects will become visible during the next refresh of the display (which should begin less than one refresh period in the future).

Every time MWorks updates the display, it announces the update via the `stimDisplayUpdate var` system variable.  As with ``update_display``, this announcement is made *before* the display is actually updated.  The time stamp on the announcement event is the operating system’s best guess for when the rendered frame will start to appear on the display.  (More precisely, it is the operating system's estimate, based on past data, of when the next `vertical blank interrupt <https://en.wikipedia.org/wiki/Vertical_blank_interrupt>`_ will occur.  The display should begin redrawing itself shortly afterward.)

If you need access to this predicted time within your experiment, set the ``predicted_output_time`` parameter of ``update_display`` to the name of a variable in which to store the value::

    update_display(predicted_output_time = my_var)

Regardless of when or how you use this time stamp, remember that it is only a *prediction* of when a future display update will begin.  If you need to know precisely when a particular stimulus appears on screen, you must measure its onset time yourself (e.g. with a photodiode attached to the display).


Advanced Tools and Techniques
-----------------------------


Variable-Attached Actions
^^^^^^^^^^^^^^^^^^^^^^^^^

Although `actions <Actions>` normally reside inside a `protocol <Protocol>`, you can also attach them to `variables <Variable>`.

Actions that are attached to a variable execute every time the variable is assigned a value.  For example, by attaching a `report <Report / Message>` action to a variable, you can log a message every time the variable's value is set::

    var x = 0 {
        report ('x = $x')
    }

Variable-attached actions will execute *even if no protocol is running*.  If you load an experiment containing the above declarion of ``x`` and, before pressing the start button, assign the value 7 to ``x`` via MWClient's variables window, you will see the message ``x = 7`` in the console.

Be aware that attaching an `assignment <Assign Variable>` to the assignment's target variable will result in `deadlock <https://en.wikipedia.org/wiki/Deadlock>`_::

    var y = 0 {
        // Don't do this!
        y += 1
    }

Variable-attached actions are a powerful tool that enable a form of `event-driven programming <https://en.wikipedia.org/wiki/Event-driven_programming>`_ within  MWorks experiments.  They can even play a role similar to `subroutines <https://en.wikipedia.org/wiki/Subroutine>`_, where assignment to the parent variable "calls" the routine.  (However, MWEL `statement macros <statement macros>` are better suited to this task.)


Replicators
^^^^^^^^^^^

`Replicators` are confusing, difficult to use correctly, and should be avoided whenever possible.  However, a few experiment-construction tasks would be difficult or impossible to accomplish without replicators, so you should know the basics of how they work.

Most commonly, replicators are employed in the declaration of related stimuli.  For example, suppose you are implementing an experiment in which you will present 100 `image stimuli <Image Stimulus>`, all of the same size and at the same on-screen position.  In the absence of replicators, your experiment would include a large list of nearly-identical stimulus declarations, one for each image file::

    var img_size = 5
    var img_pos_x = 0
    var img_pos_y = 0

    stimulus_group images {
        image_file image1 (
            path = 'images/img1.png'
            x_size = img_size
            x_position = img_pos_x
            y_position = img_pos_y
            )

        image_file image2 (
            path = 'images/img2.png'
            x_size = img_size
            x_position = img_pos_x
            y_position = img_pos_y
            )

        ...

        image_file image100 (
            path = 'images/img100.png'
            x_size = img_size
            x_position = img_pos_x
            y_position = img_pos_y
            )
    }

Because each declaration differs only in the numeric index of the image, you can replace this long, redundant list with a single image declaration contained in a `range replicator <Range Replicator>`::

    var index = 0 (scope = local)

    stimulus_group images {
        range_replicator (
            variable = index
            from = 1
            to = 100
            step = 1
            ) {
            image_file image${index} (
                path = 'images/img${index}.png'
                x_size = img_size
                x_position = img_pos_x
                y_position = img_pos_y
                )
        }
    }

There are two important points to note here:

1. The replicator variable, ``index``, includes ``scope=local`` in its declaration.
2. Inside the replicator, the text ``${index}`` is replaced with the value of ``index`` for the current iteration.

Alternatively, to avoid the requirement that your image files be named with ascending numeric indices, you can use a `list replicator <List Replicator>` with a ``filenames`` directive::

    var filename (scope = local; type = string; default_value = not_a_file)

    stimulus_group images {
        list_replicator (
            variable = filename
            values = 'filenames(images/*.png)'
            ) {
            image_file ${filename} (
                path = '${filename}'
                x_size = img_size
                x_position = img_pos_x
                y_position = img_pos_y
                )
        }
    }

Replicators can also be used to create sets of related `protocols <Protocol>`, `blocks <Block>`, `trials <Trial>`, and `lists <List>`.  For example, suppose you want your experiment to contain 100 trials.  The trials will be identical, except each will present a different image.  You can avoid having a separate declaration for each trial by using a replicator::

    protocol {
        range_replicator (
            variable = index
            from = 0
            to = 99
            step = 1
            ) {
            trial {
                queue_stimulus (images[index])
                ...
            }
        }
    }

Notice that, in this case, the replicator variable is referred to simply by name (``index``) and *not* like it is in the image declaration (``${index}``).  In fact, if you try to use the latter syntax, you will get an "unknown variable" error at run time.  This is but one of many quirks and limitations that make replicators difficult to understand and use.


.. _selection:

Selection
^^^^^^^^^

In MWorks, *selection* is a mechanism for controlling the ordering and repetition of experimental tasks and parameters.  A *selectable object* is a container from which items are drawn ("selected") in sequential or random order.  After being drawn, selected items can be *accepted* (removed from the container permanently) or *rejected* (placed back in the container to be selected again).


Selection Parameters
""""""""""""""""""""

The behavior of selectable objects is controlled by three parameters:

selection
    The selection method, which controls the order in which samples are drawn.  Allowed values are ``sequential`` (aka ``sequential_ascending``), ``sequential_descending``, ``random_without_replacement``, and ``random_with_replacement``.

nsamples
    The number of samples that may be drawn before the selectable object is exhausted

sampling_method
    Determines what constitutes a sample.  ``cycles`` means that *all* possible selections must be made to complete one sample, whereas ``samples`` indicates that each individual selection counts as a sample.

`Protocols <Protocol>`, `blocks <Block>`, `trials <Trial>`, and `lists <List>` are all selectable objects.  The items that they contain and offer for selection are their immediate child components (i.e. `actions <Actions>` and other `paradigm components <Paradigm Components>`).  This is easiest to understand via an example.

Consider a block that contains three assignments to variable ``x``, whose initial value is 0::

    block {
        x = 10*x + 1
        x = 10*x + 2
        x = 10*x + 3
    }

When the block executes, it will perform each assignment exactly once, in order of appearance, after which the value of ``x`` will be 123.  This execution behavior results from the default values used for the block's selection parameters, which we can also specify explicitly::

    block (
        selection = sequential
        nsamples = 1
        sampling_method = cycles
        ) {
        ...
    }

Now, consider how the final value of ``x`` changes as we alter each selection parameter is turn.  Suppose we change the value of ``selection`` from ``sequential`` to ``sequential_descending``::

    block (
        selection = sequential_descending
        nsamples = 1
        sampling_method = cycles
        ) {
        ...
    }

This reverses the order in which the block executes its child actions, giving ``x`` a final value of 321.

Next, suppose we change ``sampling_method`` from ``cycles`` to ``samples``::

    block (
        selection = sequential_descending
        nsamples = 1
        sampling_method = samples
        ) {
        ...
    }

Execution of a single child action now constitutes a sample.  Because ``nsamples`` is 1, the block will perform just one action, after which all selections will be exhausted, and its execution will terminate.  Hence, the final value of ``x`` will be 3.

Finally, suppose we change ``nsamples`` to 5::

    block (
        selection = sequential_descending
        nsamples = 5
        sampling_method = samples
        ) {
        ...
    }

Now, the block will draw five samples from its child components.  Because the number of samples is greater than the number of children, after reaching the end of its child list, the block will loop back to the beginning.  The final value of ``x`` will be 32132.


Accepting and Rejecting Samples
"""""""""""""""""""""""""""""""

While the examples in the previous section illustrate the meaning of the different selection parameters, they are not typical of real experiments.  Most commonly, selection is used to exercise a set of experimental conditions, in random order, with each condition having associated acceptance criteria.

The example experiment ``RSVPDemo.mwel`` contains a protocol named "Eye Calibration", which calibrates the eye positions received from an eye tracker.  The protocol requires the subject to fixate on 49 different points on screen, which are presented in random order.  It is implemented with a `list <List>` (``calibration_list``), whose ``selection`` parameter is set to ``random_without_replacement``.  The list contains 49 `trials <Trial>`, generated with two nested `range replicators <Range Replicator>`, each of which presents the fixation point at a different location::

    list calibration_list (selection = random_without_replacement) {
        range_replicator (
            variable = cal_fix_pos_x
            from = -15
            to = 15
            step = 5
        ) {
            range_replicator (
                variable = cal_fix_pos_y
                from = -15
                to = 15
                step = 5
            ) {
                trial {
                    ...
                }
            }
        }
    }

Each trial requires the subject to fixate on the relevant point for a specified length of time.  If the subject never fixates or breaks fixation early, the trial must be repeated.  This is accomplished via the `Reject Selections` action::

    reject_selections (calibration_list)

This action tells the selectable object (``calibration_list``) to put the current selection (the executing trial) back in the sample pool, ready to be chosen (and, hence, executed) again on a later draw.

Conversely, if the subject does successfully fixate for the desired length of time, then the trial takes a calibration sample for the current screen location.  After this, there's no need for the trial to execute again, so it removes itself from the sample pool with the `Accept Selections` action::

    accept_selections (calibration_list)

The list will continue to execute, choosing trials at random from its pool of non-accepted children, until all trials have been accepted.


Selection Variables
"""""""""""""""""""

In all of the selection examples so far, the selectable object has been a `paradigm component <Paradigm Components>`.  However, MWorks also provides another, more flexible type of selectable object: the `Selection Variable`.

A selection variable is essentially a bag of user-specified values, to which MWorks' selection machinery is applied.  Unlike selection-capable paradigm components, selection variables do not advance through their sample lists automatically.  Instead, each subsequent selection must be made explicitly, via the `Next Selection` action.  Within expressions, selection variables can be referred to by name, like regular variables, and evaluate to their currently-selected value.

For a demonstration of selection variables in action, see the "RSVP" protocol in ``RSVPDemo.mwel``, which uses a selection variable (``RSVP_test_stim_index``) to draw images in random order from a `stimulus group <Stimulus Group>`.


Stimulus Animation
^^^^^^^^^^^^^^^^^^

Although some of MWorks' visual stimuli (such as `videos <Video Stimulus>` and `drifting gratings <Drifting Grating Stimulus>`) are inherently dynamic, most are  designed for static display, with changes to their color, size, position, etc. being made explicitly by the experiment.  However, with a little work, these normally-static stimuli can be animated, opening the door to user-defined dynamic stimulus presentations.

Animating a non-dynamic stimulus involves three steps:

1. Writing the stimulus parameters that you want to animate as time-varying expressions,
2. Arranging for the stimulus display to redraw itself on every display refesh, and
3. Ensuring that the stimulus' parameters are re-evaluated every time the stimulus is drawn.

Consider the following `circle stimulus <Ellipse Stimulus>` declaration::

    var start_time = 0

    circle ball (
        color = 1,0,0
        x_size = 5
        x_position = -15 * cos(2*pi() * (next_frame_time() - start_time) / 3s)
        )

The ``color`` and ``x_size`` parameters have simple, constant values.  However, the value of ``x_position`` is an expression that depends on ``next_frame_time``.  This function returns the predicted output time of the frame that the stimulus display is currently rendering.  (For more information, see `Understanding Display Updates`_.)  Every time the expression is evaluated, it will return a different value, varying sinusoidally with a period of three seconds.  (Including the ``start_time`` variable in the expression for ``x_position`` allows us to control the initial position of the ball.  While not really necessary in this example, the starting time is a crucial parameter in most real-world stimulus animations, so we illustrate its use here.)

Now that we have a stimulus parameter with a time-varying value (step 1), we must force the stimulus display to redraw during every refresh cycle (step 2).  There are two ways to accomplish this.  The first is to include a `Stimulus Display` device declaration in your experiment, and set its ``redraw_on_every_refresh`` parameter to ``true``::

    stimulus_display (
        background_color = 0,0,0
        redraw_on_every_refresh = true
        )

The second method entails "wrapping" the stimulus you want to animate in a `frame list <Frame List Stimulus>` that is configured to loop indefinitely::

    frame_list ball (
        stimulus_group = ball_frames
        loop = true
        autoplay = true
        )

    stimulus_group ball_frames {
        circle (
            color = 1,0,0
            x_size = 5
            x_position = -15 * cos(2*pi() * (next_frame_time() - start_time) / 3s)
            )
    }

This method is more complicated, but it has the advantage that the display is forced to refresh *only* while the animated stimulus is on screen.

Finally, to ensure that the stimulus' parameters are re-evaluated every time it is drawn (step 3), we simply `live queue <Live Queue Stimulus>` the stimulus::

    start_time = next_frame_time ()
    live_queue_stimulus (ball)
    update_display ()

For a demonstration of more complex stimulus animation, see the example experiment ``BouncingBall.mwel``.
