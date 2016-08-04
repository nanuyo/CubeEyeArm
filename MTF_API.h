/*****************************************************************************
*
* Copyright (C) 2013 meerecompany Ltd.

* MTF_API.h : MTF_API DLL's default header file.
* Meere ToF SDK(Windows Shared DLL Version)
* Copyright(c) Meerecompany, All rights reserved.
* Update: 2015/11/26 - YKW
*
*****************************************************************************/

#ifndef MTF_API_H
#define MTF_API_H

#define MAX_DEVICE 10
typedef int mtfHandle;

//Error List
typedef enum mtfError
{
    ERROR_NO        = 1,
    ERROR_FAIL      = 0,
    ERROR_HANDLE    = -1,
    ERROR_OPEN      = -2,
    ERROR_TIME_OUT  = -3,
    ERROR_PARAM     = -4,
    ERROR_READ_DATA = -5,
}mtfError;

//Filtering Type
typedef enum mtfFilter
{
    FILTER_NO           = 0,
    FILTER_SMOOTHING    = 1,
    FILTER_EDGE         = 2,
}mtfFilter;



//Device Information
typedef struct mtfDeviceInfo
{
    mtfHandle mtfHnd;           //Device Handle
    char szDevPath[256];        //Device Path
    char szVendor[256];         //Vendor Name
    char szName[256];           //Name
    char szSerialNum[256];      //Serial Number
    unsigned short nVendorId;   //Vendor ID
    unsigned short nProductId;  //Product ID
    unsigned short nDeviceType; //Device Type(0:Only Depth, 1:Depth+RGB)
    int nWidth;                //Image Width
    int nHeight;               //Image Height
}mtfDeviceInfo;

#ifdef __cplusplus
extern "C" {
#endif

/*
*	@description	get the connected device info. list <mtfDeviceInfo[0] ~ mtfDeviceInfo[9]>
*	@param			*hnd - device handle list
                    *nDevCount - device count
*	@return			success(1) or false(0)
*/
int mtfGetDeviceList(mtfDeviceInfo *pDevInfo, int *nDevCount);


/*
*	@description	device Open
*	@param			hnd - device handle number
*	@return			success(1) or false(0)
*/
int mtfDeviceOpen(mtfHandle hnd, unsigned char nRGBCam_No);


/*
*	@description	check device open
*	@param			hnd - device handle number
*	@return			open(1) or close(0)
*/
int mtfDeviceIsOpen(mtfHandle hnd);


/*
*	@description	device close
*	@param			hnd - device handle number
*	@return			success(1) or false(0)
*/
int mtfDeviceClose(mtfHandle hnd);


/*
*	@description	set Integration time (all set function is applied during reading thread run(thread continuously call 'ReadFromDevice��function)
*	@param			hnd - device handle number
                    nIntegrationTime - Intgrationtime value(1000 ~ 5000 [unit:us])
*	@return			success(1) or false(0)
*/
int mtfSetIntegrationTime(mtfHandle hnd, int nIntegrationTime);


/*
*	@description	get Integration time
*	@param			hnd - device handle number
*	@return			current Intgrationtime value
*/
int  mtfGetIntegrationTime(mtfHandle hnd);


/*
*	@description	set Module Frequency (all set function is applied during reading thread run.(thread continuously call 'ReadFromDevice��function)
*	@param			hnd - device handle number
                    nModuleFrequency - Module Frequency value(LDC Type: 10 [unit:Mhz], MDC Type: 20 [unit:Mhz]) - don't change value
*	@return			success(1) or false(0)
*/
int mtfSetModuleFrequency(mtfHandle hnd, int nModuleFrequency);


/*
*	@description	get Module Frequency
*	@param			hnd - device handle number
*	@return			current Module Frequency
*/
int  mtfGetModuleFrequency(mtfHandle hnd);


/*
*	@description	set distance offset(all set function is applid during reading thread run.(thread continuously call 'ReadFromDevice��function)
*	@param			hnd - device handle number
                    nOffset - Offset value(0 ~ MaxDistance [unit:mm])
*	@return			success(1) or false(0)
*/
int mtfSetOffset(mtfHandle hnd, int nOffset);


/*
*	@description	get distance offset
*	@param			hnd - device handle number
*	@return			current offset
*/
int  mtfGetOffset(mtfHandle hnd);


/*
*	@description	set depth signal check threshold
*	@param			hnd - device handle number
                    nThreshold - nThreshold value(0 ~ 255)
*	@return			success(1) or false(0)
*/
int mtfSetCheckThreshold(mtfHandle hnd, int nThreshold);


/*
*	@description	get depth signal check threshold (all set function is applied during reading thread run.(thread continuously call 'ReadFromDevice��function)
*	@param			hnd - device handle number
*	@return			current depth signal check threshold
*/
int  mtfGetCheckThreshold(mtfHandle hnd);


/*
*	@description	read buffer init (call this function once before 'mtfReadFromDevice' function thread run)
*	@param			hnd - device handle number
                    wRecvData - [0]:Amplitude data, [1]:Depth data
*	@return			void
*/
int mtfReadBufInit(mtfHandle hnd);


/*
*	@description	grap start (call this function once after reading thread run.(thread continuously call 'ReadFromDevice��function)
*	@param			hnd - device handle number
*	@return			success(1) or false(0)
*/
int mtfGrabStart(mtfHandle hnd);


/*
*	@description	grap stop (call this function once before reading thread run.(thread continuously call 'ReadFromDevice��function)
*	@param			hnd - device handle number
*	@return			success(1) or false(0)
*/
int mtfGrabStop(mtfHandle hnd);


/*
*	@description	get depth data from device
*	@param			hnd - device handle number
                    wRecvData - [0]:Amplitude data(320 x 240), [1]:Depth data(320 x 240)
*	@return			void
*/
int mtfReadFromDevice(mtfHandle hnd, unsigned short** wRecvData);


/*
*	@description	get rgb data from device
*	@param			hnd - device handle number
                    RGBData - RGB 888 data(640 x 480 x 3)
*	@return			void
*/
int mtfReadFromDevice_RGB888(mtfHandle hnd, unsigned char *nRGBData);


/*
*	@description	set depth range(out of depth value is zero)
*	@param			hnd - device handle number
                    nMinDepth - min depth
                    nMaxDepth - max dpeth
*	@return			success(1) or false(0)
*/
int mtfSetDepthRange(mtfHandle hnd, int nMinDepth, int nMaxDepth);


/*
*	@description	get depth range
*	@param			hnd - device handle number
                    nMinDepth - min depth
                    nMaxDepth - max dpeth
*	@return			success(1) or false(0)
*/
int mtfGetDepthRange(mtfHandle hnd, int *nMinDepth, int *nMaxDepth);


/*
*	@description	set Flip Horizontal for frame
*	@param			hnd - device handle number
                    bEnable - true to enable, false to disable Flip Horizontal
*	@return			success(1) or false(0)
*/
int mtfSetFlipHorizontal(mtfHandle hnd, int nEnable);


/*
*	@description	get Flip Horizontal status
*	@param			hnd - device handle number
*	@return			true if 'Flip Horizontal' is currently enabled, false otherwise.
*/
int mtfGetFlipHorizontal(mtfHandle hnd);


/*
*	@description	set Flip Vertical for frame
*	@param			hnd - device handle number
                    bEnable - true to enable, false to disable Flip Vertical
*	@return			success(1) or false(0)
*/
int mtfSetFlipVertical(mtfHandle hnd, int nEnable);


/*
*	@description	get Flip Vertical status
*	@param			hnd - device handle number
*	@return			true if 'Flip Vertical' is currently enabled, false otherwise.
*/
int mtfGetFlipVertical(mtfHandle hnd);


/*
*	@description	get device sensor temperature
*	@param			hnd - device handle number
*	@return			temperature
*/
double mtfGetTemperature(mtfHandle hnd);


/*
*	@description	set Image Filtering(default: FILTER_NO)
*	@param			hnd - device handle number
                    nFilterType - filtering type
*	@return			success(1) or false(0)
*/
int mtfSetFiltering(mtfHandle hnd, mtfFilter nFilterType);


/*
*	@description	get Image Filtering
*	@param			hnd - device handle number
*	@return			applied filtering type
*/
int mtfGetFiltering(mtfHandle hnd);


/*
*	@date			2015. 11. 26.
*	@description	set Multi Sync mode status
*	@param			hnd - device handle number
                    bEnable - true to enable
*	@return			success(1) or false(0)
*/
int mtfSetMultiSyncMode(mtfHandle hnd, int nEnable);


/*
*	@date			2015. 11. 26.
*	@description	get Multi Sync mode status
*	@param			hnd - device handle number
*	@return			success(1) or false(0)
*/
int mtfGetMultiSyncMode(mtfHandle hnd);


/*
*	@date			2015. 10. 22.
*	@description	set FPS Delay
*	@param			hnd - device handle number
                    nDelay - Delay time
*	@return			success(1) or false(0)
*/
int mtfSetFPSDelay(mtfHandle hnd, int nDelay);


/*
*	@date			2015. 10. 22.
*	@description	get FPS Delay
*	@param			hnd - device handle number
*	@return			current FPS Delay
*/
int mtfGetFPSDelay(mtfHandle hnd);


/*
*	@description	Correcting lens distortion
*	@param			hnd - device handle number
                    bEnable - true to enable, false to disable remove lens distortion
                    fK1 - radial distortion value   = -0.23f
                    fFx - focal length x value      = 302.0f
                    fFy - focal length y value      = 302.0f
                    fCx - principal point x value   = 160.0f
                    fCy - principal point y value   = 120.0f
*	@return			success(1) or false(0)
*/
int mtfSetUndistortion(mtfHandle hnd, int nEnable, float fK1, float fFx, float fFy, float fCx, float fCy);

#ifdef __cplusplus
}
#endif

#endif // MTF_API_H
