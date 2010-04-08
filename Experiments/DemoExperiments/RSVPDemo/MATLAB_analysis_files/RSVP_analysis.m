
% This is a sample script to show some of the methods for analyzing data

clear all
close all
addpath('/Library/Application Support/MWorks/Scripting/Matlab')
filename = '/Documents/MWorks/Data/rsvp_demo.mwk';

% get the variable codec map.  This structure maps the codec code to a
% variable name.
codecs=getCodecs(filename);

% get the codec codes for the '#stimDisplayUpdate' variable
codec = struct2cell(codecs.codec);
codec = codec(1:2,:);
stimDisplayUpdate_code = codec{1,find(strcmp('#stimDisplayUpdate',codec(2,:)))};


% let's find all the presentations of stimulus 7.

% first we need to get all of the #stimDisplayUpdate events.  This variable
% is set whenever the display is updated, and it contains a list of all of
% the stimuli that are currently on the screen

stimDisplayUpdateEvents = getEvents(filename, stimDisplayUpdate_code);

% stimulus #7 is named 'OSImage_8'
% let's loop through stimDisplayUpdateEvents and find the time-stamps of
% when this occured

stim7timestamps = [];
for display_update_index=1:length(stimDisplayUpdateEvents)
    for stimulus_index=1:length(stimDisplayUpdateEvents(display_update_index))
        if(length(stimDisplayUpdateEvents(display_update_index).data) > 1)
            if(strcmp('OSImage_8',stimDisplayUpdateEvents(display_update_index).data{stimulus_index}.name))
                stim7timestamps(end+1) = stimDisplayUpdateEvents(display_update_index).time_us; %#ok<AGROW>
            end
        end
    end
end



% now let's look at the spikes for 0-500ms after the stimulus presentations
% we need the 'spikes' code
spikes_code = codec{1,find(strcmp('spikes',codec(2,:)))};

for stim7presents_index = 1:length(stim7timestamps)
    
    % get the spike data
    spikes = getEvents(filename, spikes_code, stim7timestamps(stim7presents_index), stim7timestamps(stim7presents_index)+500000);
    figure
    % divide by 1000 to get into milliseconds
    hist(([spikes.time_us]-min([spikes.time_us]))/1000, [50 150 250 350 450 550]);
    xlabel('ms')
end

% it's not terribly interesting because it's fake spike data.



% we can look at the time series of the RSVPs, along with fixations

allstim_timestamps = [];
fixation_timestamps = [];
for display_update_index=1:length(stimDisplayUpdateEvents)
    if(length(stimDisplayUpdateEvents(display_update_index).data) > 1)
        for stimulus_index=1:length(stimDisplayUpdateEvents(display_update_index).data)
            % first the stimuli

            if(strncmp('OSImage_',stimDisplayUpdateEvents(display_update_index).data{stimulus_index}.name, 8))
                allstim_timestamps(end+1) = stimDisplayUpdateEvents(display_update_index).time_us; %#ok<AGROW>
            end
            if(strcmp('fixation_dot',stimDisplayUpdateEvents(display_update_index).data{stimulus_index}.name))
                fixation_timestamps(end+1) = stimDisplayUpdateEvents(display_update_index).time_us; %#ok<AGROW>
            end
        end
    end
end

% note, this shows every update, so you see the fixation on every refresh
figure
plot(fixation_timestamps, ones(size(fixation_timestamps))*1.01, 'r*');
hold on;
plot(allstim_timestamps, ones(size(allstim_timestamps)), 'b*');
axis([min([allstim_timestamps fixation_timestamps]) max([allstim_timestamps fixation_timestamps]) 0 3])

% here's the fixations with the eye movements
eye_h_code = codec{1,find(strcmp('eye_h',codec(2,:)))};
eye_h = getEvents(filename, eye_h_code);
eye_v_code = codec{1,find(strcmp('eye_v',codec(2,:)))};
eye_v = getEvents(filename, eye_v_code);

figure
plot([eye_v.time_us]/1000000, [eye_v.data], 'g-');
hold on
plot(fixation_timestamps/1000000, zeros(size(fixation_timestamps)), 'r*');
xlabel('seconds')
ylabel('vertical eve movement (degrees)')

figure
plot([eye_h.data], [eye_h.time_us]/1000000, 'b-');
hold on
plot(zeros(size(fixation_timestamps)), fixation_timestamps/1000000, 'r*');
ylabel('seconds')
xlabel('horizontal eve movement (degrees)')

