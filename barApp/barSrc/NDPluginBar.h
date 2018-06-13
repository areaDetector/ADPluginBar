#ifndef NDPlufinBar_H
#define NDPluginBar_H

#include "NDPluginDriver.h"

#define BAR_VERSION      0
#define BAR_REVISION     0
#define BAR_MODIFICATION 0

//Here I will define all of the output data types once the database is written

//class that does barcode readings
class NDPluginBar : public NDPluginDriver {

	public:
		NDPluginBar(const char *portName, int queueSize, int blockingCallbacks,
			const char* NDArrayPort, int NDArrayAddr, int maxBuffers,
			size_t maxMemory, int priority, int stackSize);
		void processCallBacks(NDArray *pArray);

	protected:

		//in this section, once i define the database values, I will have to define them here

	private:

};

#endif
