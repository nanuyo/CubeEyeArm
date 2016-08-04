#ifndef CUBEEYE_H
#define CUBEEYE_H

#include <QWidget>
#include <QPainter>
#include "MTF_API.h"
#include <QtWidgets>

#define TOTAL	2
#define TRUE	1
#define FALSE	0

namespace Ui {
class CubeEye;
}

class CubeEye : public QWidget
{
    Q_OBJECT



protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void on_pushButton_clicked();

private:
    Ui::CubeEye *ui;

public:
    explicit CubeEye(QWidget *parent = 0);
    ~CubeEye();

    mtfHandle m_hnDevice;
    mtfDeviceInfo m_pDevInfo[MAX_DEVICE];

    int m_nDevCount;
    int m_nFGInit=0;
    int m_nCubeEyeOpen=FALSE;
    QTime t;
    int frameCnt = 0;

    //buf
    unsigned short *m_pCameraBuf[TOTAL];
    //uchar *m_pCameraBuf[TOTAL];
    uchar m_drawBuf[320*240*3];

    //Image Parameter
    float m_fDistMaxRange;
    float m_fDistMinRange;
    float m_fAmpMaxRange;
    float m_fAmpMinRange;

    //thread
    int m_nEnableCameraThread;

    int System_Init(void);
    void System_Close(void);

    int CubeEyeOpen(void);

    //void Thread_Read(void);

};

bool checkCameraAvailability();


#endif // CUBEEYE_H
