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


mtfHandle m_hnDevice[2];
mtfDeviceInfo m_pDevInfo[MAX_DEVICE];

int m_nDev;
int m_nDevCount;

int m_nFGInit = FALSE;

//buf
unsigned short *m_pCameraBuf1[TOTAL];
unsigned short *m_pCameraBuf2[TOTAL];

//Image Parameter
float m_fDistMaxRange;
float m_fDistMinRange;
float m_fAmpMaxRange;
float m_fAmpMinRange;

//thread
int m_nEnableCameraThread[2];

int System_Init(void)
{
    //Get device info
    mtfGetDeviceList(m_pDevInfo, &m_nDevCount);
    if(m_nDevCount < 1)
    {
        printf("Device not connection\n");
        return ERROR_FAIL;
    }

    if(m_nDevCount == 1)
    {
        //2. Set control device handle
        m_hnDevice[0] = m_pDevInfo[0].mtfHnd; //Get device 0 handle
        if(mtfDeviceOpen(m_hnDevice[0], 0) == ERROR_NO)
        {
            printf("Device Name = [%s]\n", m_pDevInfo[0].szName);
            printf("Device Serial Number = [%s]\n", m_pDevInfo[0].szSerialNum);
            m_nFGInit = TRUE;
        }
        else
        {
            m_nFGInit = FALSE;
            printf("Device Initialize Fail.");
            return ERROR_FAIL;
        }
    }
    else if(m_nDevCount == 2)//multi
    {
        //Device 1 open handle
        m_hnDevice[0] = m_pDevInfo[0].mtfHnd; //Get device 0 handle

        //3.Open Device
        if(mtfDeviceOpen(m_hnDevice[0], 0) == ERROR_NO)
        {
            printf("Device Name = [%s]\n", m_pDevInfo[0].szName);
            printf("Device Serial Number = [%s]\n", m_pDevInfo[0].szSerialNum);
        }
        else
        {
            printf("Device Initialize Fail.");
            return ERROR_FAIL;
        }

        //Device 2 open handle
        m_hnDevice[1] = m_pDevInfo[1].mtfHnd; //Get device 0 handle

        //3.Open Device
        if(mtfDeviceOpen(m_hnDevice[1], 1) == ERROR_NO)
        {
            printf("Device Name = [%s]\n", m_pDevInfo[1].szName);
            printf("Device Serial Number = [%s]\n", m_pDevInfo[1].szSerialNum);
        }
        else
        {
            printf("Device Initialize Fail.");
            return ERROR_FAIL;
        }
    }

    //buf init & allocate;
    int i;
    for(i=0; i<TOTAL; i++)
    {
        m_pCameraBuf1[i]		= NULL;
        m_pCameraBuf2[i]		= NULL;
        m_pCameraBuf1[i]		= (unsigned short*)malloc(sizeof(unsigned short) * IMAGE_WIDTH * IMAGE_HEIGHT);
        m_pCameraBuf2[i]		= (unsigned short*)malloc(sizeof(unsigned short) * IMAGE_WIDTH * IMAGE_HEIGHT);
    }

    m_nEnableCameraThread[0] = FALSE;
    m_nEnableCameraThread[1] = FALSE;

    return ERROR_NO;
}

void System_Close(void)
{
    int i;
    //Device close
    for(i=0; i<m_nDevCount; i++)
    {
        if(mtfDeviceIsOpen(m_hnDevice[i]))
        {
            mtfDeviceClose(m_hnDevice[i]);//Device close
        }
    }

    for(i=0; i<TOTAL; i++)//buf free
    {
        if(m_pCameraBuf1[i] != NULL)
        {
            free(m_pCameraBuf1[i]);
            m_pCameraBuf1[i] = NULL;
        }
        if(m_pCameraBuf2[i] != NULL)
        {
            free(m_pCameraBuf2[i]);
            m_pCameraBuf2[i] = NULL;
        }
    }
}

static void *Thread_Read1(void *arg)
{
    while(m_nEnableCameraThread[0])
    {
        mtfReadFromDevice(m_hnDevice[0], (unsigned short**)m_pCameraBuf1);

        if(m_pCameraBuf1[DATA_DEPTH] != NULL)
        {
            printf("Data1 : %d\n", m_pCameraBuf1[DATA_DEPTH][(IMAGE_HEIGHT/2)*IMAGE_WIDTH+(IMAGE_WIDTH/2)]);
        }
        usleep(1);
    }
    return (void*)arg;
}

static void *Thread_Read2(void *arg)
{
    while(m_nEnableCameraThread[1])
    {
        mtfReadFromDevice(m_hnDevice[1], (unsigned short**)m_pCameraBuf2);

        if(m_pCameraBuf2[DATA_DEPTH] != NULL)
        {
            printf("Data2 : %d\n", m_pCameraBuf2[DATA_DEPTH][(IMAGE_HEIGHT/2)*IMAGE_WIDTH+(IMAGE_WIDTH/2)]);
        }
        usleep(1);
    }
    return (void*)arg;
}


int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    pthread_t tidRead1, tidRead2;
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
    printf("s : device 0/1 data Read Start\n");

    while(1)
    {
        getkey = getchar();

        //Reading end Process
        if(getkey == 'q')
        {
            //1. Grab Stop
            //mtfGrabStop(m_hnDevice[0]);

            //2. read thread stop
            m_nEnableCameraThread[0] = FALSE;
            m_nEnableCameraThread[1] = FALSE;
            break;
        }

        //single Reading Start Process
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

            if(m_nEnableCameraThread[0] == FALSE)//if reading first? => reading thread start
            {
                //1.Read buffer init
                mtfReadBufInit(m_hnDevice[0]);
                usleep(100);

                //2. Read thread start
                m_nEnableCameraThread[0] = TRUE;
                nRet = pthread_create(&tidRead1, NULL, Thread_Read1, NULL);
                if(nRet<0)
                {
                    m_nEnableCameraThread[0] = FALSE;
                    printf("Thread1 Read False.\n");
                    return 0;
                }

                //3. Grab Start
                mtfGrabStart(m_hnDevice[0]);

                //4.Setting parameter
                //Set integration time
                mtfSetIntegrationTime(m_hnDevice[0], 4000);//mtfSetIntegrationTime(m_hnDevice[0], 5000);
                //Set amplitude check threshold
                mtfSetCheckThreshold(m_hnDevice[0], 0);//mtfSetCheckThreshold(m_hnDevice[0], 20);
                //Check depth range
                int nMinDepth, nMaxDepth;
                mtfGetDepthRange(m_hnDevice[0], &nMinDepth, &nMaxDepth);
                //Set depth range(0 ~ max depth)
                mtfSetDepthRange(m_hnDevice[0], nMinDepth, nMaxDepth);

                printf("Min Depth:%d, Max Depth:%d\n", nMinDepth, nMaxDepth);
                printf("Data Dispay Start\n");
            }

            if(m_nDevCount == 2)
            {
                if(m_nEnableCameraThread[1] == FALSE)//device1reading thread start
                {
                    //1.Read buffer init
                    mtfReadBufInit(m_hnDevice[1]);
                    usleep(100);

                    //2. Read thread start
                    m_nEnableCameraThread[1] = TRUE;
                    nRet = pthread_create(&tidRead2, NULL, Thread_Read2, NULL);
                    if(nRet<0)
                    {
                        m_nEnableCameraThread[1] = FALSE;
                        printf("Thread2 Read False.\n");
                        return 0;
                    }

                    //3. Grab Start
                    mtfGrabStart(m_hnDevice[1]);

                    //4.Setting parameter
                    //Set integration time
                    mtfSetIntegrationTime(m_hnDevice[1], 4000);//mtfSetIntegrationTime(m_hnDevice[0], 5000);
                    //Set amplitude check threshold
                    mtfSetCheckThreshold(m_hnDevice[1], 0);//mtfSetCheckThreshold(m_hnDevice[0], 20);
                    //Check depth range
                    int nMinDepth, nMaxDepth;
                    mtfGetDepthRange(m_hnDevice[1], &nMinDepth, &nMaxDepth);
                    //Set depth range(0 ~ max depth)
                    mtfSetDepthRange(m_hnDevice[1], nMinDepth, nMaxDepth);

                    printf("dev1 Min Depth:%d, Max Depth:%d\n", nMinDepth, nMaxDepth);
                    printf("dev1 Data Dispay Start\n");
                }
            }

        }//getkey 's'
    }//while
	
    //Reading end Process
    if(m_nEnableCameraThread[0])
        pthread_join(tidRead1, (void**)&nStatus);

    if(m_nDevCount == 2 && m_nEnableCameraThread[1])
        pthread_join(tidRead2, (void**)&nStatus);

    System_Close();
    printf("program End\n");
    return 0;
}
