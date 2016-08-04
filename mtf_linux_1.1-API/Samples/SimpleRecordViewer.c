#include <GL/glut.h>
#include <GL/gl.h>

#include <unistd.h> //usleep
#include <sys/time.h> //gettimeofday
#include <pthread.h>//pthread
#include <stdio.h>
#include <stdlib.h>

#include "../include/MTF_API.h"

#define TOTAL	2
#define DATA_AMPLITUDE  0
#define DATA_DEPTH      1
#define DATA_RGB        2

#define MTF_TYPE_RGB    1

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

typedef struct _FrameHeader
{
    int nWidth;
    int nHeight;
    int nFrames;
    int nDepthBufSize;
    int nRGBBufSize;
    int nDeviceType;
}_stFrameHeader;
_stFrameHeader m_stFrameHeader; //Frame Data

unsigned short	*m_pCameraBuf[2];
unsigned char   *m_pRGBCameraBuf;

unsigned int	m_nTexMapX;
unsigned int	m_nTexMapY;

int m_nDisplayData;

char m_szFileName[255];
FILE *m_pLoadFile;
bool m_bLoadFile;
int m_nMaxFrames;
int m_nLoadDeviceType;
int m_nDeviceType;
int m_nFrameIndex;

void System_Close(void)
{
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

    if(m_pTexMap != NULL)
    {
        delete[] m_pTexMap;
    }

}

void glut_Keyboard(unsigned char key, int /*x*/, int /*y*/)
{
    switch (key)
    {
    case 27://ESC
        if(m_bLoadFile == true)
            fclose(m_pLoadFile);

        exit (1);

    case '1'://1
        m_nDisplayData = DATA_AMPLITUDE;

        break;

    case '2'://2
        m_nDisplayData = DATA_DEPTH;
        break;

    case '3'://3
        if(m_nDeviceType == MTF_TYPE_RGB)
            m_nDisplayData = DATA_RGB;

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


void glut_Display()
{
    usleep(100);

    if(m_pLoadFile != NULL)
    {
        //1000ms / 30fps = 33.333ms
        usleep(33000);//30fps set 33ms

        //frame check

        if(m_nFrameIndex == m_nMaxFrames)
        {
            m_nFrameIndex = 0;
            fseek(m_pLoadFile, sizeof(m_stFrameHeader), SEEK_SET);//pos move fileHeader
        }
        m_nFrameIndex++;

        //read
        fread(m_pCameraBuf[DATA_AMPLITUDE], IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(unsigned short), 1, m_pLoadFile);
        fread(m_pCameraBuf[DATA_DEPTH], IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(unsigned short),  1, m_pLoadFile);
        if(m_nLoadDeviceType)
        {
            fread(m_pRGBCameraBuf, 640*480*3*sizeof(unsigned char), 1, m_pLoadFile);
        }
    }

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);


    RGB888Pixel* pTexRow = m_pTexMap + 0 * m_nTexMapX;


    //*********** Read Data Display ****************//

    register int x, y;

    //Amplitude display
    if(m_nDisplayData == DATA_AMPLITUDE)
    {
        unsigned char nGray8 = 0;

        for (y = 0; y < IMAGE_HEIGHT; ++y)
        {
            RGB888Pixel* pTex = pTexRow + 0;

            for (x = 0; x < IMAGE_WIDTH; ++x, ++pTex)
            {
                //Convert Gray
                Convert_To_GRAY8((float)m_pCameraBuf[0][y*IMAGE_WIDTH+x], &nGray8, 0.0f, 200.0f);
                pTex->r = pTex->g = pTex->b = nGray8;

            }//for
            pTexRow += m_nTexMapX;
        }//for
    }

    //Depth display
    else if(m_nDisplayData == DATA_DEPTH)
    {
        for (y = 0; y < IMAGE_HEIGHT; ++y)
        {
            RGB888Pixel* pTex = pTexRow + 0;

            for (x = 0; x < IMAGE_WIDTH; ++x, ++pTex)
            {
                //Convert RGB888 Data
                Convert_To_RGB24((float)m_pCameraBuf[1][y*IMAGE_WIDTH+x], pTex, 0.0f, 7500.0f);
            }//for
            pTexRow += m_nTexMapX;
        }//for
    }

    //Color RGB888 display
    else if(m_nDisplayData == DATA_RGB)
    {
        for (y = 0; y < 480; ++y)
        {
            RGB888Pixel* pTex = pTexRow;

            for (x = 0; x < 640; ++x, ++pTex)
            {
                pTex->r = m_pRGBCameraBuf[(y*640+x)*3+2];//R
                pTex->g = m_pRGBCameraBuf[(y*640+x)*3+1];//G
                pTex->b = m_pRGBCameraBuf[(y*640+x)*3+0];//B

            }//for

            pTexRow += m_nTexMapX;
        }//for
    }

    printf("Display Frame %d / %d\n", m_nFrameIndex, m_nMaxFrames);

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

int System_Init(void)
{
    //Initcode
    m_nDisplayData = DATA_DEPTH;

    //file name : *.mee
    sprintf(m_szFileName, "%s", "DataFile.mee");

    //buf init;
    int i;
    for(i=0; i<TOTAL; i++)
    {
        m_pCameraBuf[i] = NULL;
    }
    m_pRGBCameraBuf = NULL;

    m_pLoadFile = fopen(m_szFileName, "r+");//read onlay //get data


    /***********************
    *.mee file open example
    ************************/
    if(m_pLoadFile != NULL)
    {
        //head infomation
        //size_t readbyte;
        fseek(m_pLoadFile, 0, SEEK_SET);//pos move 0x00
        fread(&m_stFrameHeader, sizeof(m_stFrameHeader), 1, m_pLoadFile);

        m_nMaxFrames = m_stFrameHeader.nFrames;
        m_nLoadDeviceType = m_stFrameHeader.nDeviceType;

        //nDeviceType : 0 - Only Depth, 1 - RGB
        m_nDeviceType = m_stFrameHeader.nDeviceType;
        m_nFrameIndex = 0;

        m_bLoadFile = TRUE;

        printf("*.mee file open success, Data Display Start\n");

        printf("Key 'Esc': exit\n");
        printf("Key   '1': Depth 320x240 Data\n");
        printf("Key   '2': Amplitude 320x240 Data\n");
        printf("Key   '3': RGB 888 640x480 Data\n");
    }
    else
    {
        printf("*.mee file open fail, Confirm the path\n");
        m_bLoadFile = FALSE;
        return ERROR_FAIL;
    }

    // Texture map init
    m_nTexMapX = MIN_CHUNKS_SIZE(IMAGE_WIDTH, TEXTURE_SIZE);
    m_nTexMapY = MIN_CHUNKS_SIZE(IMAGE_HEIGHT, TEXTURE_SIZE);
    m_pTexMap = new RGB888Pixel[m_nTexMapX * m_nTexMapY];

    //buf allocate
    for(i=0; i<TOTAL; i++)
    {
        m_pCameraBuf[i]     = (unsigned short*)malloc(sizeof(unsigned short) * IMAGE_WIDTH * IMAGE_HEIGHT);
    }
    if(m_nLoadDeviceType == MTF_TYPE_RGB)//RGB Type
        m_pRGBCameraBuf		= (unsigned char *)malloc(640*480*3*sizeof(unsigned char));

    return ERROR_NO;
}

int main(int argc, char** argv)
{
    if(System_Init() != ERROR_NO)
        return 0;

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
    glutCreateWindow ("Simple MTF Viewer");

    initOpenGLHooks();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glutMainLoop();
}
