#ifndef GHN_LIB_Layer2Connection_h__
#define GHN_LIB_Layer2Connection_h__

#include "console.h"


#ifdef _WIN32
#define DllExport   __declspec( dllexport ) 
#else
#define DllExport
#endif

#ifdef __cplusplus
extern "C" {
#endif

DllExport eGHN_LIB_STAT Open_Layer2_Connection(	sConnection_Information*		xi_Connection,
												UINT16							xi_etype,
												Layer2Connection*				xo_layer2Connection,
												char*							xo_ErrorDescription);

DllExport eGHN_LIB_STAT Close_Layer2_Connection(Layer2Connection* layer2Connection);

#ifdef __cplusplus
}
#endif

#endif // GHN_LIB_Layer2Connection_h__
