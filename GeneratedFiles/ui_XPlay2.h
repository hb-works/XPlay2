/********************************************************************************
** Form generated from reading UI file 'XPlay2.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_XPLAY2_H
#define UI_XPLAY2_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>
#include <XVideoWidget.h>

QT_BEGIN_NAMESPACE

class Ui_XPlay2Class
{
public:
    XVideoWidget *openGLWidget;

    void setupUi(QWidget *XPlay2Class)
    {
        if (XPlay2Class->objectName().isEmpty())
            XPlay2Class->setObjectName(QStringLiteral("XPlay2Class"));
        XPlay2Class->resize(1043, 827);
        openGLWidget = new XVideoWidget(XPlay2Class);
        openGLWidget->setObjectName(QStringLiteral("openGLWidget"));
        openGLWidget->setGeometry(QRect(50, 50, 800, 600));

        retranslateUi(XPlay2Class);

        QMetaObject::connectSlotsByName(XPlay2Class);
    } // setupUi

    void retranslateUi(QWidget *XPlay2Class)
    {
        XPlay2Class->setWindowTitle(QApplication::translate("XPlay2Class", "XPlay2", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class XPlay2Class: public Ui_XPlay2Class {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_XPLAY2_H
