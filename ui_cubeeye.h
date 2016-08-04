/********************************************************************************
** Form generated from reading UI file 'cubeeye.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CUBEEYE_H
#define UI_CUBEEYE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CubeEye
{
public:
    QPushButton *pushButton;
    QLabel *label;
    QTextEdit *textEdit;

    void setupUi(QWidget *CubeEye)
    {
        if (CubeEye->objectName().isEmpty())
            CubeEye->setObjectName(QStringLiteral("CubeEye"));
        CubeEye->resize(405, 351);
        pushButton = new QPushButton(CubeEye);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(0, 50, 85, 27));
        label = new QLabel(CubeEye);
        label->setObjectName(QStringLiteral("label"));
        label->setEnabled(true);
        label->setGeometry(QRect(9, 9, 382, 31));
        label->setMaximumSize(QSize(16777215, 602));
        textEdit = new QTextEdit(CubeEye);
        textEdit->setObjectName(QStringLiteral("textEdit"));
        textEdit->setGeometry(QRect(90, 50, 311, 31));

        retranslateUi(CubeEye);

        QMetaObject::connectSlotsByName(CubeEye);
    } // setupUi

    void retranslateUi(QWidget *CubeEye)
    {
        CubeEye->setWindowTitle(QApplication::translate("CubeEye", "CubeEye", 0));
        pushButton->setText(QApplication::translate("CubeEye", "Play", 0));
        label->setText(QApplication::translate("CubeEye", "<html><head/><body><p align=\"center\"><span style=\" font-size:18pt; font-weight:600;\">SmartCore - CubeEye</span></p></body></html>", 0));
    } // retranslateUi

};

namespace Ui {
    class CubeEye: public Ui_CubeEye {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CUBEEYE_H
