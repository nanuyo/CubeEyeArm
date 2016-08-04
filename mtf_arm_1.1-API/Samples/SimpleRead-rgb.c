#include <stdio.h>
#include <stdlib.h>
#include <string.h>//strstr
#include <unistd.h>//usleep
#include <pthread.h>//pthread

#include "../include/MTF_API.h"

#define TOTAL	2
#define TRUE	1
#define FALSE	0
#define DATA_AMPLITUDE  0
#define DATA_DEPTH      1

#define DEPTH_WIDTH    320
#define DEPTH_HEIGHT   240

#define RGB_WIDTH    640
#define RGB_HEIGHT   480
#define RGB888       3


mtfHandle m_hnDevice;
mtfDeviceInfo m_stDevInfo[MAX_DEVICE];

int m_nDevCount = 0;
int m_nDeviceTypeRGB = 0;
int m_nFGInit = FALSE;

//buf
unsigned short *m_pCameraBuf[TOTAL];
unsigned char *m_pRGBCameraBuf;

//Image Parameter
float m_fDistMaxRange;
float m_fDistMinRange;
float m_fAmpMaxRange;
float m_fAmpMinRange;

//thread
int m_nEnableCameraThread;

int System_Init(void)
{
    //Get device info
    mtfGetDeviceList(m_stDevInfo, &m_nDevCount);
    if(m_nDevCount < 1)
    {
        printf("Device not connection\n");
        return ERROR_FAIL;
    }

    m_hnDevice = m_stDevInfo[0].mtfHnd; //device handle:0 ~ (max device-1)

    if(strstr(m_stDevInfo[m_hnDevice].szName, "MTF-MDC040") ||
       strstr(m_stDevInfo[m_hnDevice].szName, "MTF-LDC040" ) )
    {
        m_nDeviceTypeRGB = TRUE;
    }
    else//Not RGB Module
    {
        m_nDeviceTypeRGB = FALSE;
	printf("Device not rgb module.\n");
	return -1;
    }

    if(mtfDeviceOpen(m_hnDevice, 0) != ERROR_NO)
    {
        m_nFGInit = FALSE;
        printf("Device Initialize Fail.");
        return ERROR_OPEN;
    }

    printf("Device Name = [%s]\n", m_stDevInfo[0].szName);
    printf("Device Serial Number = [%s]\n", m_stDevInfo[0].szSerialNum);
    m_nFGInit = TRUE;

    //buf init;
    int i;
    for(i=0; i<TOTAL; i++)
    {
        m_pCameraBuf[i]	= NULL;
    }
    m_pRGBCameraBuf = NULL;


    //buf allocate
    for(i=0; i<TOTAL; i++)
    {
        m_pCameraBuf[i]	= (unsigned short*)malloc(sizeof(unsigned short) * DEPTH_WIDTH * DEPTH_HEIGHT);
    }
    if(m_nDeviceTypeRGB)
    {
        m_pRGBCameraBuf	= (unsigned char*)malloc(sizeof(unsigned char) * RGB_WIDTH * RGB_HEIGHT * RGB888);
    }

    return ERROR_NO;
}

void System_Close(void)
{
    mtfDeviceClose(m_hnDevice);//Device close
    
	int i;
    for(i=0; i<TOTAL; i++)//buf free
    {
        if(m_pCameraBuf[i] != NULL)
        {
            free(m_pCameraBuf[i]);
            m_pCameraBuf[i] = NULL;
        }   
    }
    if(m_pRGBCameraBuf != NULL)
    {
        free(m_pRGBCameraBuf);
        m_pRGBCameraBuf = NULL;
    }
}

static void *Thread_Read(void *arg)
{
    while(m_nEnableCameraThread)
    {
//        if(m_nDeviceTypeRGB)
//        {
//            mtfReadFromDevice_RGB888(m_hnDevice, (unsigned short**)m_pCameraBuf, m_pRGBCameraBuf);

//            if(m_pCameraBuf[DATA_DEPTH] != NULL)
//            {
//                printf("Depth:%d, R:%d, G:%d, B:%d\n",m_pCameraBuf[DATA_DEPTH][(DEPTH_HEIGHT/2)*DEPTH_WIDTH+(DEPTH_WIDTH/2)]
//                                                     ,m_pRGBCameraBuf[((RGB_HEIGHT/2)*1920)+((RGB_WIDTH/2)*3)+2]
//                                                     ,m_pRGBCameraBuf[((RGB_HEIGHT/2)*1920)+((RGB_WIDTH/2)*3)+1]
//                                                     ,m_pRGBCameraBuf[((RGB_HEIGHT/2)*1920)+((RGB_WIDTH/2)*3)+0]
//                                                     );
//            }
//        }
//        else
//        {
//            mtfReadFromDevice(m_hnDevice, (unsigned short**)m_pCameraBuf);

//            if(m_pCameraBuf[DATA_DEPTH] != NULL)
//            {
//                printf("Data : %d\n", m_pCameraBuf[DATA_DEPTH][(DEPTH_HEIGHT/2)*DEPTH_WIDTH+(DEPTH_WIDTH/2)]);
//            }
//        }

        mtfReadFromDevice(m_hnDevice, (unsigned short**)m_pCameraBuf);
        mtfReadFromDevice_RGB888(m_hnDevice, m_pRGBCameraBuf);
        if(m_pCameraBuf[DATA_DEPTH] != NULL && m_pRGBCameraBuf != NULL)
        {
            printf("Depth:%d, R:%d, G:%d, B:%d\n",m_pCameraBuf[DATA_DEPTH][(DEPTH_HEIGHT/2)*DEPTH_WIDTH+(DEPTH_WIDTH/2)]
                                                                 ,m_pRGBCameraBuf[((RGB_HEIGHT/2)*1920)+((RGB_WIDTH/2)*3)+2]
                                                                 ,m_pRGBCameraBuf[((RGB_HEIGHT/2)*1920)+((RGB_WIDTH/2)*3)+1]
                                                                 ,m_pRGBCameraBuf[((RGB_HEIGHT/2)*1920)+((RGB_WIDTH/2)*3)+0]
                                                                 );
        }



        usleep(1);
    }
    return (void*)arg;
}


int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    pthread_t tidRead;
    int nRet, nStatus;
    char getkey;
	
    //InIt
    nRet = System_Init();
    if(nRet < 1)
    {
        printf("System Error\n");
        return -1;
    }

    printf("select operation\n");
    printf("--------Key Menu--------\n");
    printf("Key Menu\n");
    printf("q : Exit\n");
    printf("s : data Read Start\n");

    while(1)
    {
        getkey = getchar();

        //Reading end Process
        if(getkey == 'q')
        {
            //1. Grab Stop
            mtfGrabStop(m_hnDevice);
            usleep(100);

            //2. read thread stop
            m_nEnableCameraThread = FALSE;
            break;
        }

        //Reading Start Process
        else if(getkey == 's')
        {
            /* Get Data, Set Param process
             * 1. mtfGetDeviceList
             * 2. mtfReadBufInit
             * 3. mtfReadFromDevice
             * 4. mtfGrabStart
             * 5. Parameter Setting
             *
             * mtfGrabStart --> Parameter Setting --> setup
             * Parameter Setting --> mtfGrabStart --> setup fail
             * */

            if(m_nEnableCameraThread == FALSE)
			{
            	//1.Read buffer init
                mtfReadBufInit(m_hnDevice);

           		usleep(100);

            	//2. Read thread start
            	m_nEnableCameraThread = TRUE;
            	nRet = pthread_create(&tidRead, NULL, Thread_Read, NULL);
            	if(nRet<0)
            	{
                	m_nEnableCameraThread = FALSE;
                	printf("Thread Read False.\n");
                	return 0;
            	}

            	//3. Grab Start
                mtfGrabStart(m_hnDevice);

                //4.Setting parameter
                //Set integration time
                mtfSetIntegrationTime(m_hnDevice, 4000);//mtfSetIntegrationTime(m_hnDevice, 5000);
                //Set amplitude check threshold
                mtfSetCheckThreshold(m_hnDevice, 0);//mtfSetCheckThreshold(m_hnDevice, 20);
                //Check depth range
                int nMinDepth, nMaxDepth;
                mtfGetDepthRange(m_hnDevice, &nMinDepth, &nMaxDepth);
                //Set depth range(0 ~ max depth)
                mtfSetDepthRange(m_hnDevice, nMinDepth, nMaxDepth);

                printf("Min Depth:%d, Max Depth:%d\n", nMinDepth, nMaxDepth);
                printf("Data Dispay Start\n");
			}
        }
    }//while
	
    //Reading end Process
    if(m_nEnableCameraThread)
        pthread_join(tidRead, (void**)&nStatus);

    System_Close();
    printf("program End\n");
    return 0;
}
