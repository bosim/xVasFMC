/********************************************************************************
** Form generated from reading UI file 'pbdpointdlg.ui'
**
** Created: Fri 15. Jun 19:19:19 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PBDPOINTDLG_H
#define UI_PBDPOINTDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_PbdPointDlg
{
public:
    QGridLayout *gridLayout;
    QDoubleSpinBox *distance_spinbox;
    QSpinBox *bearing_spinbox;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QLabel *label;
    QLabel *label_3;
    QLabel *label_2;
    QLineEdit *wpt_id_edit;

    void setupUi(QDialog *PbdPointDlg)
    {
        if (PbdPointDlg->objectName().isEmpty())
            PbdPointDlg->setObjectName(QString::fromUtf8("PbdPointDlg"));
        PbdPointDlg->resize(236, 164);
        gridLayout = new QGridLayout(PbdPointDlg);
#ifndef Q_OS_MAC
        gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        distance_spinbox = new QDoubleSpinBox(PbdPointDlg);
        distance_spinbox->setObjectName(QString::fromUtf8("distance_spinbox"));
        distance_spinbox->setDecimals(1);
        distance_spinbox->setMaximum(999);

        gridLayout->addWidget(distance_spinbox, 2, 1, 1, 1);

        bearing_spinbox = new QSpinBox(PbdPointDlg);
        bearing_spinbox->setObjectName(QString::fromUtf8("bearing_spinbox"));
        bearing_spinbox->setMaximum(359);

        gridLayout->addWidget(bearing_spinbox, 1, 1, 1, 1);

        hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        okButton = new QPushButton(PbdPointDlg);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        hboxLayout->addWidget(okButton);

        cancelButton = new QPushButton(PbdPointDlg);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        hboxLayout->addWidget(cancelButton);


        gridLayout->addLayout(hboxLayout, 3, 0, 1, 2);

        label = new QLabel(PbdPointDlg);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_3 = new QLabel(PbdPointDlg);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        label_2 = new QLabel(PbdPointDlg);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        wpt_id_edit = new QLineEdit(PbdPointDlg);
        wpt_id_edit->setObjectName(QString::fromUtf8("wpt_id_edit"));

        gridLayout->addWidget(wpt_id_edit, 0, 1, 1, 1);

        QWidget::setTabOrder(wpt_id_edit, bearing_spinbox);
        QWidget::setTabOrder(bearing_spinbox, distance_spinbox);
        QWidget::setTabOrder(distance_spinbox, okButton);
        QWidget::setTabOrder(okButton, cancelButton);

        retranslateUi(PbdPointDlg);
        QObject::connect(okButton, SIGNAL(clicked()), PbdPointDlg, SLOT(accept()));
        QObject::connect(cancelButton, SIGNAL(clicked()), PbdPointDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(PbdPointDlg);
    } // setupUi

    void retranslateUi(QDialog *PbdPointDlg)
    {
        PbdPointDlg->setWindowTitle(QApplication::translate("PbdPointDlg", "PBD Point", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("PbdPointDlg", "OK", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("PbdPointDlg", "Cancel", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("PbdPointDlg", "Waypoint ID:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("PbdPointDlg", "Distance (NM):", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("PbdPointDlg", "Magnetic Bearing:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class PbdPointDlg: public Ui_PbdPointDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PBDPOINTDLG_H
