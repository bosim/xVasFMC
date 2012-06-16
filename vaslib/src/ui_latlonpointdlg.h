/********************************************************************************
** Form generated from reading UI file 'latlonpointdlg.ui'
**
** Created: Fri 15. Jun 19:19:19 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LATLONPOINTDLG_H
#define UI_LATLONPOINTDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_LatLonPointDlg
{
public:
    QGridLayout *gridLayout;
    QLineEdit *lon_edit;
    QLineEdit *lat_edit;
    QLabel *label_3;
    QLabel *label_2;
    QLineEdit *wpt_id_edit;
    QLabel *label;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *okButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *LatLonPointDlg)
    {
        if (LatLonPointDlg->objectName().isEmpty())
            LatLonPointDlg->setObjectName(QString::fromUtf8("LatLonPointDlg"));
        LatLonPointDlg->resize(233, 164);
        gridLayout = new QGridLayout(LatLonPointDlg);
#ifndef Q_OS_MAC
        gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        lon_edit = new QLineEdit(LatLonPointDlg);
        lon_edit->setObjectName(QString::fromUtf8("lon_edit"));

        gridLayout->addWidget(lon_edit, 2, 1, 1, 1);

        lat_edit = new QLineEdit(LatLonPointDlg);
        lat_edit->setObjectName(QString::fromUtf8("lat_edit"));

        gridLayout->addWidget(lat_edit, 1, 1, 1, 1);

        label_3 = new QLabel(LatLonPointDlg);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        label_2 = new QLabel(LatLonPointDlg);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        wpt_id_edit = new QLineEdit(LatLonPointDlg);
        wpt_id_edit->setObjectName(QString::fromUtf8("wpt_id_edit"));

        gridLayout->addWidget(wpt_id_edit, 0, 1, 1, 1);

        label = new QLabel(LatLonPointDlg);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        okButton = new QPushButton(LatLonPointDlg);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        hboxLayout->addWidget(okButton);

        cancelButton = new QPushButton(LatLonPointDlg);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        hboxLayout->addWidget(cancelButton);


        gridLayout->addLayout(hboxLayout, 3, 0, 1, 2);

        QWidget::setTabOrder(wpt_id_edit, lat_edit);
        QWidget::setTabOrder(lat_edit, lon_edit);
        QWidget::setTabOrder(lon_edit, okButton);
        QWidget::setTabOrder(okButton, cancelButton);

        retranslateUi(LatLonPointDlg);
        QObject::connect(okButton, SIGNAL(clicked()), LatLonPointDlg, SLOT(accept()));
        QObject::connect(cancelButton, SIGNAL(clicked()), LatLonPointDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(LatLonPointDlg);
    } // setupUi

    void retranslateUi(QDialog *LatLonPointDlg)
    {
        LatLonPointDlg->setWindowTitle(QApplication::translate("LatLonPointDlg", "LAT/LON Point", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("LatLonPointDlg", "Longitude:\n"
"(example: E016.32.0)", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("LatLonPointDlg", "Latitude:\n"
"(example: N48.16.2)", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("LatLonPointDlg", "Waypoint ID:", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("LatLonPointDlg", "OK", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("LatLonPointDlg", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class LatLonPointDlg: public Ui_LatLonPointDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LATLONPOINTDLG_H
