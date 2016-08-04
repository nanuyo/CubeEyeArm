#include <GL/glut.h>
#include <GL/gl.h>

#include <unistd.h> //usleep
#include <sys/time.h> //gettimeofday
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>//pthread

#include "../include/MTF_API.h"

#define TOTAL	2

#define DATA_DEPTH_1        1
#define DATA_AMPLITUDE_1    2
#define DATA_DEPTH_2        3
#define DATA_AMPLITUDE_2    4

#define IMAGE_WIDTH     320
#define IMAGE_HEIGHT    240

#define GL_WIN_SIZE_X   1280
#define GL_WIN_SIZE_Y   1024
#define TEXTURE_SIZE    512

#define TRUE    1
#define FALSE   0

#define MIN_NUM_CHUNKS(data_size, chunk_size)	((((data_size)-1) / (chunk_size) + 1))
#define MIN_CHUNKS_SIZE(data_size, chunk_size)	(MIN_NUM_CHUNKS(data_size, chunk_size) * (chunk_size))

typedef struct _RGB888Pixel
{
    /* Red value of this pixel. */
    unsigned char r;
    /* Green value of this pixel. */
    unsigned char g;
    /* Blue value of this pixel. */
    unsigned char b;
}RGB888Pixel;
RGB888Pixel*	m_pTexMap;

mtfHandle		m_hnDevice[2];             //control device handle
mtfDeviceInfo	m_pDevInfo[MAX_DEVICE];	//device info


unsigned short	*m_pCameraBuf[2];
unsigned short 	*m_pDisplayBuf[2];
unsigned short 	*m_pTriDisplayBuf[2];

unsigned short	*m_pCameraBuf2[2];
unsigned short 	*m_pDisplayBuf2[2];
unsigned short 	*m_pTriDisplayBuf2[2];

unsigned int	m_nTexMapX;
unsigned int	m_nTexMapY;

int m_nDevCount;
int m_nDisplayData;
int nStatus;
int m_nEnableCameraThread;

bool	m_bDisplayBufAvailable[2];
pthread_t tidRead;

//time
double m_dCameraTimeVal, m_dPreCameraTimeVal;
double m_dCameraTime, m_dFCameraTime;
double m_dDisplayTimeVal, m_dPreDisplayTimeVal;
double m_dDisplayTime, m_dFDisplayTime;
int m_nCameraCount, m_nDisplayCount;

double checkClock()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec + (tv.tv_usec/1000000.0));
}

void CountDisplayTime()
{
    m_dDisplayTimeVal = checkClock();
    m_dDisplayTime +=( m_dDisplayTimeVal - m_dPreDisplayTimeVal );
    m_dPreDisplayTimeVal = m_dDisplayTimeVal;
    m_nDisplayCount++;
    if(m_nDisplayCount >= 10)
    {
        m_dFDisplayTime = m_dDisplayTime;
        m_nDisplayCount = 0;
        m_dDisplayTime = 0;
    }
}

void System_Close(void)
{
    mtfDeviceClose(m_hnDevice[0]);//Device close
    mtfDeviceClose(m_hnDevice[1]);//Device close

    int i;
    for(i=0; i<TOTAL; i++)//buf free
    {
        if(m_pCameraBuf[i] != NULL)
        {
            free(m_pCameraBuf[i]);
            m_pCameraBuf[i] = NULL;
        }
        if(m_pDisplayBuf[i] != NULL)
        {
            free(m_pDisplayBuf[i]);
            m_pDisplayBuf[i] = NULL;
        }
        if(m_pTriDisplayBuf[i] != NULL)
        {
            free(m_pTriDisplayBuf[i]);
            m_pTriDisplayBuf[i] = NULL;
        }

        if(m_pCameraBuf2[i] != NULL)
        {
            free(m_pCameraBuf2[i]);
            m_pCameraBuf2[i] = NULL;
        }
        if(m_pDisplayBuf2[i] != NULL)
        {
            free(m_pDisplayBuf2[i]);
            m_pDisplayBuf2[i] = NULL;
        }
        if(m_pTriDisplayBuf2[i] != NULL)
        {
            free(m_pTriDisplayBuf2[i]);
            m_pTriDisplayBuf2[i] = NULL;
        }

    }
    if(m_pTexMap != NULL)
    {
        delete[] m_pTexMap;
    }
}

void glut_Keyboard(unsigned char key, int /*x*/, int /*y*/)
{
    switch (key)
    {
    case 27://esc

        if(m_nEnableCameraThread == true)
        {
            //1.Grab stop
            mtfGrabStop(m_hnDevice[0]);
            mtfGrabStop(m_hnDevice[1]);

            //2.Thread stop
            m_nEnableCameraThread = false;

            //Reading end Process
            if(m_nEnableCameraThread)
                pthread_join(tidRead, (void**)&nStatus);

        }
        System_Close();//Close
        exit (1);

    case '1'://1
        m_nDisplayData = DATA_AMPLITUDE_1;
        break;

    case '2'://2
        m_nDisplayData = DATA_DEPTH_1;
        break;

    case '3'://3
        m_nDisplayData = DATA_AMPLITUDE_2;
        break;

    case '4'://4
        m_nDisplayData = DATA_DEPTH_2;
        break;
    }

}

int Convert_To_GRAY8(float fValue, unsigned char* nGrayData, float fMinValue, float fMaxValue)
{
    if(fValue < fMinValue) *nGrayData = 0;
    else if(fValue > fMaxValue) *nGrayData = 255;
    else
    {
        *nGrayData = (unsigned char)(255.0f * (fValue-fMinValue)/(fMaxValue-fMinValue));
    }
    return true;
}

int Convert_To_RGB24( float fValue, RGB888Pixel *nRGBData, float fMinValue, float fMaxValue)
{
    if(fValue == 0) //Invalide Pixel
    {
        nRGBData->r = 0;//R
        nRGBData->g = 0;//G
        nRGBData->b = 0;//B
    }
    else if(fValue < fMinValue)
    {
        nRGBData->r = 255;//R
        nRGBData->g = 0;//G
        nRGBData->b = 0;//B
    }
    else if(fValue > fMaxValue)
    {
        nRGBData->r = 255;//R
        nRGBData->g = 0;//G
        nRGBData->b = 255;//B
    }
    else
    {
        float fColorWeight;
        fColorWeight = (fValue-fMinValue) / (fMaxValue-fMinValue);

        if( (fColorWeight <= 1.0f) && (fColorWeight > 0.8f) )
        {
            nRGBData->r = (unsigned char)(255 * ((fColorWeight - 0.8f) / 0.2f));//값에 따라 증가
            nRGBData->g = 0;
            nRGBData->b = 255;
        }
        else if( (fColorWeight <= 0.8f) && (fColorWeight > 0.6f) )
        {
            nRGBData->r = 0;
            nRGBData->g = (unsigned char)(255 * (1.0f - (fColorWeight - 0.6f) / 0.2f));//값에 따라 감소
            nRGBData->b = 255;
        }
        else if( (fColorWeight <= 0.6f) && (fColorWeight > 0.4f) )
        {
            nRGBData->r = 0;
            nRGBData->g = 255;
            nRGBData->b = (unsigned char)(255 * ((fColorWeight - 0.4f) / 0.2f));//값에 따라 증가
        }
        else if( (fColorWeight <= 0.4f) && (fColorWeight > 0.2f) )
        {
            nRGBData->r = (unsigned char)(255 * (1.0f - (fColorWeight - 0.2f) / 0.2f));//값에 따라 감소
            nRGBData->g = 255;
            nRGBData->b = 0;
        }
        else if( (fColorWeight <= 0.2f) && (fColorWeight >= 0.0f) )
        {
            nRGBData->r = 255;
            nRGBData->g = (unsigned char)(255 * ((fColorWeight - 0.0f) / 0.2f));//값에 따라 증가
            nRGBData->b = 0;
        }
        else
        {
            nRGBData->r = 0;
            nRGBData->g = 0;
            nRGBData->b = 0;
        }
    }

    return true;
}

int DisplayBufferFlip(unsigned short **pBuf1, unsigned short **pBuf2, int bPrimaryBuffer)
{
    bool bFlipped = FALSE;
    {
        if (bPrimaryBuffer || m_bDisplayBufAvailable[0])
        {

            unsigned short *pTmp[2];

            pTmp[0] = *pBuf1;
            pTmp[1] = *pBuf2;

            *pBuf1 = m_pTriDisplayBuf[0];
            *pBuf2 = m_pTriDisplayBuf[1];

            m_pTriDisplayBuf[0] = pTmp[0];
            m_pTriDisplayBuf[1] = pTmp[1];

            bFlipped = TRUE;
        }
        m_bDisplayBufAvailable[0] = bPrimaryBuffer;
    }
    return bFlipped;
}

int DisplayBufferFlip2(unsigned short **pBuf1, unsigned short **pBuf2, int bPrimaryBuffer)
{
    bool bFlipped = FALSE;
    {
        if (bPrimaryBuffer || m_bDisplayBufAvailable[1])
        {

            unsigned short *pTmp[2];

            pTmp[0] = *pBuf1;
            pTmp[1] = *pBuf2;

            *pBuf1 = m_pTriDisplayBuf2[0];
            *pBuf2 = m_pTriDisplayBuf2[1];

            m_pTriDisplayBuf2[0] = pTmp[0];
            m_pTriDisplayBuf2[1] = pTmp[1];

            bFlipped = TRUE;
        }
        m_bDisplayBufAvailable[1] = bPrimaryBuffer;
    }
    return bFlipped;
}

void glut_Display()
{
    usleep(100);

    if(DisplayBufferFlip(&m_pDisplayBuf[0], &m_pDisplayBuf[1], FALSE) &&
       DisplayBufferFlip2(&m_pDisplayBuf2[0], &m_pDisplayBuf2[1], FALSE) )
    {
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);



        RGB888Pixel* pTexRow = m_pTexMap + 0 * m_nTexMapX;


        //*********** Read Data Display ****************//

        register int x, y;

        //Amplitude display
        if(m_nDisplayData == DATA_AMPLITUDE_1)
        {
            unsigned char nGray8 = 0;
            for (y = 0; y < IMAGE_HEIGHT; ++y)
            {
                RGB888Pixel* pTex;
                pTex = pTexRow + 0;

                for (x = 0; x < IMAGE_WIDTH; ++x, ++pTex)
                {
                    //Convert RGB888 Data
                    Convert_To_GRAY8((float)m_pDisplayBuf[0][y*IMAGE_WIDTH+x], &nGray8, 0.0f, 200.0f);
                    pTex->r = pTex->g = pTex->b = nGray8;

                }//for
                pTexRow += m_nTexMapX;
            }//for
        }

        //1-Depth display
        if(m_nDisplayData == DATA_DEPTH_1)
        {
            for (y = 0; y < IMAGE_HEIGHT; ++y)
            {
                RGB888Pixel* pTex = pTexRow + 0;

                for (x = 0; x < IMAGE_WIDTH; ++x, ++pTex)
                {
                    //Convert RGB888 Data
                    Convert_To_RGB24((float)m_pDisplayBuf[1][y*IMAGE_WIDTH+x], pTex, 0.0f, 7500.0f);
                }//for
                pTexRow += m_nTexMapX;
            }//for
        }

        //Amplitude display
        if(m_nDisplayData == DATA_AMPLITUDE_2)
        {
            unsigned char nGray8 = 0;
            for (y = 0; y < IMAGE_HEIGHT; ++y)
            {
                RGB888Pixel* pTex;
                pTex = pTexRow + 0;

                for (x = 0; x < IMAGE_WIDTH; ++x, ++pTex)
                {
                    //Convert RGB888 Data
                    Convert_To_GRAY8((float)m_pDisplayBuf2[0][y*IMAGE_WIDTH+x], &nGray8, 0.0f, 200.0f);
                    pTex->r = pTex->g = pTex->b = nGray8;

                }//for
                pTexRow += m_nTexMapX;
            }//for
        }

        //2-Depth display
        else if(m_nDisplayData == DATA_DEPTH_2)
        {
            for (y = 0; y < IMAGE_HEIGHT; ++y)
            {
                RGB888Pixel* pTex = pTexRow + 0;

                for (x = 0; x < IMAGE_WIDTH; ++x, ++pTex)
                {
                    //Convert RGB888 Data
                    Convert_To_RGB24((float)m_pDisplayBuf2[1][y*IMAGE_WIDTH+x], pTex, 0.0f, 7500.0f);
                }//for
                pTexRow += m_nTexMapX;
            }//for
        }

        //******************************************//

        //opengl fuction
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_nTexMapX, m_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pTexMap);

        // Display the OpenGL texture map
        glColor4f(1,1,1,1);

        glBegin(GL_QUADS);

        // upper left
        glTexCoord2f(0, 0);
        glVertex2f(0, 0);
        // upper right
        glTexCoord2f((float)IMAGE_WIDTH/(float)m_nTexMapX, 0);
        glVertex2f(GL_WIN_SIZE_X, 0);
        // bottom right
        glTexCoord2f((float)IMAGE_WIDTH/(float)m_nTexMapX, (float)IMAGE_HEIGHT/(float)m_nTexMapY);
        glVertex2f(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
        // bottom left
        glTexCoord2f(0, (float)IMAGE_HEIGHT/(float)m_nTexMapY);
        glVertex2f(0, GL_WIN_SIZE_Y);

        glEnd();

        // Swap the OpenGL display buffers
        glutSwapBuffers();

        //FPS Measurment
        CountDisplayTime();

        double Displayfps = 1.0/m_dFDisplayTime*10.0;
        printf("[DisplayTime:%3.1f]\n", Displayfps);
    }//if

}

void glut_Idle()
{
    glutPostRedisplay();//update
}

void initOpenGLHooks()
{
    glutKeyboardFunc(glut_Keyboard);
    glutDisplayFunc(glut_Display);
    glutIdleFunc(glut_Idle);
}

static void *Thread_Read(void *arg)
{
    while(m_nEnableCameraThread)
    {
        DisplayBufferFlip(&m_pCameraBuf[0], &m_pCameraBuf[1], true);
        mtfReadFromDevice(m_hnDevice[0], (unsigned short**)m_pCameraBuf);
        usleep(30000);//Delay MultiSync

        DisplayBufferFlip2(&m_pCameraBuf2[0], &m_pCameraBuf2[1], true);
        mtfReadFromDevice(m_hnDevice[1], (unsigned short**)m_pCameraBuf2);
        usleep(30000);//Delay MultiSync
    }
    return (void*)arg;
}

int System_Init(void)
{
    //Get device info
    mtfGetDeviceList(m_pDevInfo, &m_nDevCount);
    if(m_nDevCount < 1)
    {
        printf("Device not connection\n");
        return ERROR_FAIL;
    }
    if(m_nDevCount != 2)
    {
        printf("Connect the two devices.");
        exit(0);
    }

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


    //buf init;
    int i;
    for(i=0; i<TOTAL; i++)
    {
        m_pCameraBuf[i]     = NULL;
        m_pDisplayBuf[i]    = NULL;
        m_pTriDisplayBuf[i] = NULL;
        m_pCameraBuf2[i]    = NULL;
        m_pDisplayBuf2[i]   = NULL;
        m_pTriDisplayBuf2[i]= NULL;

    }

    //buf allocate
    for(i=0; i<TOTAL; i++)
    {
        m_pCameraBuf[i]     = (unsigned short*)malloc(sizeof(unsigned short) * IMAGE_WIDTH * IMAGE_HEIGHT);
        m_pDisplayBuf[i]	= (unsigned short*)malloc(sizeof(unsigned short) * IMAGE_WIDTH * IMAGE_HEIGHT);
        m_pTriDisplayBuf[i]	= (unsigned short*)malloc(sizeof(unsigned short) * IMAGE_WIDTH * IMAGE_HEIGHT);
        m_pCameraBuf2[i]    = (unsigned short*)malloc(sizeof(unsigned short) * IMAGE_WIDTH * IMAGE_HEIGHT);
        m_pDisplayBuf2[i]	= (unsigned short*)malloc(sizeof(unsigned short) * IMAGE_WIDTH * IMAGE_HEIGHT);
        m_pTriDisplayBuf2[i]= (unsigned short*)malloc(sizeof(unsigned short) * IMAGE_WIDTH * IMAGE_HEIGHT);
    }

    //Initcode
    m_nDisplayData = DATA_DEPTH_1;

    // Texture map init
    m_nTexMapX = MIN_CHUNKS_SIZE(IMAGE_WIDTH, TEXTURE_SIZE);
    m_nTexMapY = MIN_CHUNKS_SIZE(IMAGE_HEIGHT, TEXTURE_SIZE);
    m_pTexMap = new RGB888Pixel[m_nTexMapX * m_nTexMapY];

    //********** Data Acquire process *****************//

    //1.Read buffer init
    mtfReadBufInit(m_hnDevice[0]);
    mtfReadBufInit(m_hnDevice[1]);

    usleep(100);

    //2.Read thread start
    int nRet;
    nRet = pthread_create(&tidRead, NULL, Thread_Read, NULL);
    if(nRet<0)
    {
        printf("Thread Read False.\n");
        return 0;
    }

    m_nEnableCameraThread = TRUE;

    mtfGrabStart(m_hnDevice[0]);
    mtfGrabStart(m_hnDevice[1]);

    //*** SetMultiSyncMode ***//
    mtfSetMultiSyncMode(m_hnDevice[0], TRUE);
    mtfSetMultiSyncMode(m_hnDevice[1], TRUE);

    printf("Data Dispay Start\n");
    printf("Key 'Esc': exit\n");
    printf("Key   '1':  1-Depth Data\n");
    printf("Key   '2':  1-Amplitude Data\n");
    printf("Key   '3':  2-Depth Data\n");
    printf("Key   '4':  2-Amplitude Data\n");
    //*****************************************************//

    return ERROR_NO;
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    System_Init();

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
    glutCreateWindow ("Simple MTF Viewer");

    initOpenGLHooks();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glutMainLoop();
}
