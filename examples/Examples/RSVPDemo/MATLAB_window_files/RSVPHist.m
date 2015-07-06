function [retval] = RSVPHist(data_struct, input)

if nargin == 1
    input.raster = struct([]);
    input.trialnum = ones(100,1);
    input.stimnumall = [];
%    fprintf(2,'no argument \n');
%    save ~/Desktop/RSVPHisttest.mat
end

% Find codes of relevant variables

codec = struct2cell(data_struct.event_codec);
codec = codec(1:2,:);
code.stimDisplayUpdate = codec{1,find(strcmp('#stimDisplayUpdate',codec(2,:)))};
code.spikes = codec{1,find(strcmp('spikes',codec(2,:)))};

% Get all the spikes
spkind = find([data_struct.events.event_code] == code.spikes);
spiketimes = [data_struct.events(spkind).time_us]./1000;%divide by 1000 to get spiketimes in milliseconds


% Get all the #stimDisplayUpdate events
updateind = find([data_struct.events.event_code] == code.stimDisplayUpdate);
updateevents = data_struct.events(updateind);



% Get all the stimuli start times using #updatestimulus display
stimstart = [];
stimnum = [];
stimnumall = input.stimnumall;
for ind = 1:length(updateevents)
    if length(updateevents(ind).data) > 1
        if strncmp('OSImage',updateevents(ind).data{1}.name,7)
            stimstart = [stimstart updateevents(ind).time_us./1000];%divide by 1000 to get spiketime in milliseconds
            %fprintf(2,'%s\n',updateevents(ind).data{1}.name);
            updateevents(ind).data{1}.name
            updateevents(ind).data{1}.pos_x
            updateevents(ind).data{1}.pos_y
            updateevents(ind).time_us./1000
            stimnum = [stimnum str2num(updateevents(ind).data{1}.name(findstr('_',updateevents(ind).data{1}.name)+1:end))];
            stimnumall = [stimnumall str2num(updateevents(ind).data{1}.name(findstr('_',updateevents(ind).data{1}.name)+1:end))];
        end
    end
end


if ((~ishandle(1)) | (nargin == 1))
    figure(1);
    for plotind = 1:100
        ax(plotind) = subplot(10,10,plotind,'nextplot','add');
        set(gca,'xticklabel',[]);
        if plotind ~= 91
            set(gca,'yticklabel',[]);
        end
        set(gca,'xlim',[-150 300]);
        %set(gca,'ylim',[-1 17]);
        set(gca,'xtick',[0 200]);
        %set(gca,'ytick',[0 15]);
        set(gca,'tickdir','out');
        set(gca,'ticklength',[0.03 0.025]);
        set(gca,'color','none');
        set(gca,'Position',get(gca,'Outerposition'));
        text(0.05,0.95,num2str(plotind-1),'units','normalized');

    end
    set(ax(91),'xticklabel',[0 200]);
    %set(ax(91),'yticklabel',[0 15]);
    xlabel(ax(91),'Time (ms)');
    ylabel(ax(91),'Trial #'); 
    linkaxes(ax,'y');
else
    ax = input.ax;
end

%find the spikes that occur whithin a time window (-100,500) around the onset of the
%stimulus

raster = input.raster;
trialnum = input.trialnum;
binwidth = 25;
timebins = [-150:binwidth:500];
for histind = 1:length(stimstart)    
    stimspks = spiketimes(find(spiketimes >= stimstart(histind)-150 & spiketimes <= (stimstart(histind)+500)))-stimstart(histind);
    %plot(ax(stimnum(histind)),stimspks,trialnum(stimnum(histind)).*ones(size(stimspks)),'k*','MarkerSize',[2]);
    raster(trialnum(stimnum(histind)),stimnum(histind)).spiketimes = stimspks;
    [N2,BIN] = histc(cast([raster(:,stimnum(histind)).spiketimes], 'double'),timebins);
    if isempty(N2)
        N2 = zeros(length(timebins));
    end
    FR2 = (N2*1000)./(trialnum(stimnum(histind))*binwidth)
    if trialnum(stimnum(histind)) > 1
        [N1,BIN] = histc(cast([raster(1:trialnum(stimnum(histind))-1,stimnum(histind)).spiketimes], 'double'),timebins);
        if isempty(N1)
            N1 = zeros(length(timebins));
        end
        FR1 = (N1*1000)./((trialnum(stimnum(histind))-1)*binwidth);
        axes(ax(stimnum(histind)));
        cla;
        plot(timebins+(binwidth./2),FR1,'r-');
        if max(FR1) > max(get(gca,'ylim'))
            axis tight;
        end
    end
    axes(ax(stimnum(histind)));
    plot(timebins+(binwidth./2),FR2,'k-');
    stimspks = [];
    trialnum(stimnum(histind)) = trialnum(stimnum(histind)) + 1;
    text(0.05,0.95,num2str(stimnum(histind)-1),'units','normalized');
    text(0.9,0.95,num2str(trialnum(stimnum(histind))),'units','normalized');
    clear N1 N2 FR1 FR2;
end




retval.stimnumall = stimnumall;
retval.trialnum  = trialnum
retval.raster = raster
retval.ax = ax

