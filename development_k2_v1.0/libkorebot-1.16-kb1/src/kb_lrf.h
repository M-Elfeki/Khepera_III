
/* ***************************************************************************************** */
/* ** LRF library                                                                         ** */
/* ***************************************************************************************** */
extern unsigned short kb_lrf_DistanceData[121];		// Array contains distance data after
							// successfull call of LRF_GetDistances

/* ***************************************************************************************** */
/* ** Prototypes                                                                          ** */
/* ***************************************************************************************** */

extern int kb_lrf_Init(char *LRF_DeviceName);		// returns LRF_DeviceHandle
                                  
extern int kb_lrf_GetDistances(int LRF_DeviceHandle);	// return <0 on failure, 0 on success

extern void kb_lrf_Close(int LRF_DeviceHandle);		// no return value

