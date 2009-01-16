#ifndef FILTERS_H
#define FILTERS_H

#include "Buffers.h"

namespace mw {
class Filter{


	public:
	
		// given a ring buffer object and pointer to one of its sub-buffers, compute the filter
		virtual float filter(RingBuffer *bufferobj, float *subbuffer){return 0;};
};


class BoxCarFilter : public Filter {


	protected:
	
		int width;  // how many samples to average over
		bool timewindow; // samples (false) or msec (true)?
		
		
	public:
	
		BoxCarFilter(int _width, bool _timewindow = false){
			width = _width;			
			timewindow = _timewindow;
		}



		virtual float filter(TimeStampedRingBuffer *bufferobj, float *subbuffer){
		
			float running_sum = 0;
			
			int index = bufferobj->currentIndex();
			
			if(timewindow){ 		// if it's a time window, we'll scroll back through samples on the basis of their times
			
				int time_sofar = 0;
				MonkeyWorksTime *time = bufferobj->getTime();
				MonkeyWorksTime current_time = time[index];
				
				while(time_sofar < width){
					running_sum += subbuffer[index];
					index = bufferobj->previousIndex(index);
					time_sofar = current_time - time[index];
				}
			
			
			} else {  			// this object is specified in samples
			
				for(int i = 0; i < width; i++){
					running_sum += subbuffer[index];
					index = bufferobj->previousIndex(index);
				}
			}
                        
                        return running_sum;
		}

};

}

#endif
