

#define XPLM430
#define XPLM420
#define XPLM411
#define XPLM400

#include <stdio.h>
#include <string.h>
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"


static float	FlightDataRecorderCallback(
                                   float                inElapsedSinceLastCall,    
                                   float                inElapsedTimeSinceLastFlightLoop,    
                                   int                  inCounter,    
                                   void *               inRefcon);    


PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	strcpy(outName, "Flight Data Recorder");
	strcpy(outSig, "conconlolo.FlightDataRecorder");
	strcpy(outDesc, "A plugin that allows data to be recorded and written to a GeoJSON file.");

	/* Register our callback for once a second.  Positive intervals
	 * are in seconds, negative are the negative of sim frames.  Zero
	 * registers but does not schedule a callback for time. */
	XPLMRegisterFlightLoopCallback(		
			FlightDataRecorderCallback,	/* Callback */
			1.0,					/* Interval */
			NULL);					/* refcon not used. */
			
	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	/* Unregister the callback */
	XPLMUnregisterFlightLoopCallback(FlightDataRecorderCallback, NULL);
}

PLUGIN_API void XPluginDisable(void)
{
	
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(
					XPLMPluginID	inFromWho,
					int				inMessage,
					void *			inParam)
{
}

float	FlightDataRecorderCallback(
                                   float                inElapsedSinceLastCall,    
                                   float                inElapsedTimeSinceLastFlightLoop,    
                                   int                  inCounter,    
                                   void *               inRefcon)
{
	/* Do something with callback */
	
	/* Return 1.0 to indicate that we want to be called again in 1 second. */
	return 1.0;
}                                   


