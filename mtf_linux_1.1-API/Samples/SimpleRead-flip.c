#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>//usleep
#include <pthread.h>//pthread

#include "../include/MTF_API.h"

#define TOTAL	2
#define TRUE	1
#define FALSE	0
#define DATA_AMPLITUDE  0
#define DATA_DEPTH      1

#define IMAGE_WIDTH    320
#define IMAGE_HEIGHT   240


mtfHandle m_hnDevice;
mtfDeviceInfo m_pDevInfo[MAX_DEVICE];

int m_nDevCount = 0;
int m_nFGInit = FALSE;

//buf
unsigned short *m_pCameraBuf[TOTAL];

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
    mtfGetDeviceList(m_pDevInfo, &m_nDevCount);
    if(m_nDevCount < 1)
    {
        printf("Device not connection\n");
        return ERROR_FAIL;
    }

    m_hnDevice = m_pDevInfo[0].mtfHnd; //device handle:0 ~ (max device-1)
    if(mtfDeviceOpen(m_hnDevice, 0) == ERROR_NO)
    {
        printf("Device Name = [%s]\n", m_pDevInfo[0].szName);
        printf("Device Serial Number = [%s]\n", m_pDevInfo[0].szSerialNum);
        m_nFGInit = TRUE;
    }
    else
    {
        m_nFGInit = FALSE;
        printf("Device Initialize Fail.");
        return ERROR_OPEN;
    }

    //buf init;
    int i;
    for(i=0; i<TOTAL; i++)
    {
        m_pCameraBuf[i]	= NULL;
    }

    //buf allocate
    for(i=0; i<TOTAL; i++)
    {
        m_pCameraBuf[i]	= (unsigned short*)malloc(sizeof(unsigned short) * IMAGE_WIDTH * IMAGE_HEIGHT);
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

}

static void *Thread_Read(void *arg)
{
    while(m_nEnableCameraThread)
    {
        mtfReadFromDevice(m_hnDevice, (unsigned short**)m_pCameraBuf);

        if(m_pCameraBuf[DATA_DEPTH] != NULL)
        {
            printf("Data : %d\n", m_pCameraBuf[DATA_DEPTH][(IMAGE_HEIGHT/2/2)*IMAGE_WIDTH+(IMAGE_WIDTH/2/2)]);
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

            //2. read thread stop
            m_nEnableCameraThread = FALSE;
            break;
        }

        //-----FilpTest Key----//
        else if(getkey == '1')
        {
            mtfSetFlipVertical(m_hnDevice, FALSE);
            printf("vertical off\n");

        }
        else if(getkey == '2')
        {
            mtfSetFlipVertical(m_hnDevice, TRUE);
            printf("vertical on\n");

        }
        else if(getkey == '3')
        {
            mtfSetFlipHorizontal(m_hnDevice, FALSE);
            printf("horizontal off\n");
        }
        else if(getkey == '4')
        {
            mtfSetFlipHorizontal(m_hnDevice, TRUE);
            printf("horizontal on\n");
        }
        //-----FilpTest Key----//

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
