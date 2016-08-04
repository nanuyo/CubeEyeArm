#include "cubeeye.h"
#include "ui_cubeeye.h"
#include "MTF_API.h"
#include <unistd.h>//usleep


#define DATA_AMPLITUDE  0
#define DATA_DEPTH      1

#define IMAGE_WIDTH    320
#define IMAGE_HEIGHT   240

CubeEye::CubeEye(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CubeEye)
{
    QTimer *timer = new QTimer(this);
    ui->setupUi(this);

    m_nFGInit = 0;
    System_Init();
    m_nCubeEyeOpen = CubeEyeOpen();

    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  //  timer->start(1000);
      timer->start(33.3);
      t.start();
}

CubeEye::~CubeEye()
{
    System_Close();
    delete ui;
}

void CubeEye::on_pushButton_clicked()
{
// m_nCubeEyeOpen = CubeEyeOpen();
   update();
}

void CubeEye::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter;
    painter.begin(this);


    mtfReadFromDevice(m_hnDevice, (unsigned short**)m_pCameraBuf);


    if(m_pCameraBuf[DATA_DEPTH] != NULL)
    {
#if 0
        int k=0;

        for(int i=0 ; i < IMAGE_HEIGHT ; i++)
        {
            for(int j=0 ; j < IMAGE_WIDTH; j++)
            {
                painter.setPen(m_pCameraBuf[DATA_DEPTH][j+k]);
                painter.drawPoint(j+30, i+100);
                if (frameCnt < m_pCameraBuf[DATA_DEPTH][j+k])
                {
                frameCnt = m_pCameraBuf[DATA_DEPTH][j+k];
                qDebug("%d\n ", frameCnt);
                }
            }

            k += IMAGE_WIDTH;
        }
#else

        int imageWidth = IMAGE_WIDTH;
        int imageHeight = IMAGE_HEIGHT;
        int bytesPerPixel = 3; // 4 for RGBA, 3 for RGB
        unsigned short tData;
        memset(m_drawBuf, 0x00, sizeof(m_drawBuf));
                for(int i=0 ; i < IMAGE_HEIGHT ; i++)
                {
                    for(int j=0 ; j < IMAGE_WIDTH; j++)
                    {
                       tData = m_pCameraBuf[0][i*IMAGE_WIDTH + j];
                       //tData /=128;
                       if(tData>255) tData = 255;
                       m_drawBuf[i*IMAGE_WIDTH*3+j*3+0] = (uchar)tData;
                       m_drawBuf[i*IMAGE_WIDTH*3+j*3+1] = (uchar)tData;
                       m_drawBuf[i*IMAGE_WIDTH*3+j*3+2] = (uchar)tData;
                    }
                }

//        QImage image( (uchar*) m_pCameraBuf[DATA_DEPTH], imageWidth, imageHeight, imageWidth * bytesPerPixel, QImage::Format_RGB16, Q_NULLPTR, Q_NULLPTR);
        QImage image( m_drawBuf, imageWidth, imageHeight, imageWidth * bytesPerPixel, QImage::Format_RGB888, Q_NULLPTR, Q_NULLPTR);
        painter.drawImage(30,100, image);
        frameCnt ++;
        if(t.elapsed()>=1000)
        {
            qDebug("Number of Frames per 1 Second = %d, Data = %d\n", frameCnt, m_pCameraBuf[DATA_DEPTH][(IMAGE_HEIGHT/2)*IMAGE_WIDTH+(IMAGE_WIDTH/2)]);
            t.restart();
            frameCnt =0;
        }
#endif

    }

    painter.end();
}


int CubeEye::System_Init(void)
{
    if (m_nFGInit != 0)
    {
        return FALSE;
    }

    //Get device info
    mtfGetDeviceList(m_pDevInfo, &m_nDevCount);
    qDebug("Number of Device is %d\n", m_nDevCount);

    if(m_nDevCount < 1)
    {
        qDebug("Device is not connected\n");
        ui->textEdit->setPlainText("Device is not connected");
        return ERROR_FAIL;
    }

    ui->textEdit->setPlainText("CubeEye is connected");

    m_hnDevice = m_pDevInfo[0].mtfHnd; //device handle:0 ~ (max device-1)
    if(mtfDeviceOpen(m_hnDevice, 0) == ERROR_NO)
    {
        qDebug("Device Information");
        qDebug("Vendor Name = [%s]\n", m_pDevInfo[0].szVendor);
        qDebug("Device Name = [%s]\n", m_pDevInfo[0].szName);
        qDebug("Device Serial Number = [%s]\n", m_pDevInfo[0].szSerialNum);
        qDebug("Device Vendor ID =[%03x], Product ID = [%03x]\n", m_pDevInfo[0].nVendorId, m_pDevInfo[0].nProductId);
        qDebug("Device TYPE = [%d]\n", m_pDevInfo[0].nDeviceType);
        qDebug("Image Width=[%d], Image Height=[%d]\n", m_pDevInfo[0].nWidth, m_pDevInfo[0].nHeight);

        m_nFGInit = TRUE;
    }
    else
    {
        m_nFGInit = FALSE;
        qDebug("Device Initialize Fail.");
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
        memset(m_pCameraBuf[i], 0, sizeof(unsigned short) * IMAGE_WIDTH * IMAGE_HEIGHT);

    }

    return ERROR_NO;
}


void CubeEye::System_Close(void)
{

    if (!mtfDeviceIsOpen(m_hnDevice))
    {
        return;
    }

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

    qDebug("CubeEye Close\n");
}


#if 0
void CubeEye::Thread_Read(void)
{
   // QTime t;
  //  int i=0;

   // t.start();

    //while(1)
    {

        mtfReadFromDevice(m_hnDevice, (unsigned short**)m_pCameraBuf);

      /*  if(m_pCameraBuf[DATA_DEPTH] != NULL)
        {
            qDebug("Data : %d, Count : %d\n", m_pCameraBuf[DATA_DEPTH][(IMAGE_HEIGHT/2)*IMAGE_WIDTH+(IMAGE_WIDTH/2)], i++);
        }*/

//if(t.elapsed()>=1000)
  //  return;

     //   usleep(1);
    }
    //return (void*)arg;
}
#endif

int CubeEye::CubeEyeOpen(void)
{

    if (m_nFGInit == FALSE)
    {
        qDebug("CubeEys is not initialized\n");
        return FALSE;
    }

    if (m_hnDevice != NULL)
    {
        if (mtfDeviceIsOpen(m_hnDevice))
        {
            qDebug("CubeEye is already Open\n");
            return TRUE;
        }
    }

    if (m_nCubeEyeOpen == TRUE)
    {
        qDebug("Already Open\n");
        return TRUE;
    }


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

                //1.Read buffer init
                mtfReadBufInit(m_hnDevice);
                usleep(100);


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

                qDebug("Min Depth:%d, Max Depth:%d\n", nMinDepth, nMaxDepth);
                qDebug("Data Dispay Start\n");





    m_nCubeEyeOpen = TRUE;
    return m_nCubeEyeOpen;
}

