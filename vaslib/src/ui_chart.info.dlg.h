/********************************************************************************
** Form generated from reading UI file 'chart.info.dlg.ui'
**
** Created: Fri 15. Jun 19:19:19 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHART_H
#define UI_CHART_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDateEdit>
#include <QtGui/QDialog>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ChartInfoDlg
{
public:
    QVBoxLayout *vboxLayout;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout;
    QLabel *label_6;
    QLineEdit *chartname_edit;
    QDoubleSpinBox *magvar_spinbox;
    QDateEdit *effdate_edit;
    QLabel *label_4;
    QLabel *label_5;
    QGroupBox *groupBox;
    QGridLayout *gridLayout1;
    QLabel *label;
    QLabel *label_3;
    QLineEdit *airport_icao_edit;
    QLabel *label_2;
    QLineEdit *ctry_edit;
    QLineEdit *city_edit;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *okButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *ChartInfoDlg)
    {
        if (ChartInfoDlg->objectName().isEmpty())
            ChartInfoDlg->setObjectName(QString::fromUtf8("ChartInfoDlg"));
        ChartInfoDlg->resize(230, 300);
        QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ChartInfoDlg->sizePolicy().hasHeightForWidth());
        ChartInfoDlg->setSizePolicy(sizePolicy);
        ChartInfoDlg->setModal(true);
        vboxLayout = new QVBoxLayout(ChartInfoDlg);
#ifndef Q_OS_MAC
        vboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        groupBox_3 = new QGroupBox(ChartInfoDlg);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(0));
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy1);
        gridLayout = new QGridLayout(groupBox_3);
#ifndef Q_OS_MAC
        gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_6 = new QLabel(groupBox_3);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 0, 0, 1, 1);

        chartname_edit = new QLineEdit(groupBox_3);
        chartname_edit->setObjectName(QString::fromUtf8("chartname_edit"));

        gridLayout->addWidget(chartname_edit, 0, 1, 1, 2);

        magvar_spinbox = new QDoubleSpinBox(groupBox_3);
        magvar_spinbox->setObjectName(QString::fromUtf8("magvar_spinbox"));
        magvar_spinbox->setDecimals(1);
        magvar_spinbox->setMaximum(180);
        magvar_spinbox->setMinimum(-180);

        gridLayout->addWidget(magvar_spinbox, 2, 2, 1, 1);

        effdate_edit = new QDateEdit(groupBox_3);
        effdate_edit->setObjectName(QString::fromUtf8("effdate_edit"));

        gridLayout->addWidget(effdate_edit, 1, 2, 1, 1);

        label_4 = new QLabel(groupBox_3);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 1, 0, 1, 2);

        label_5 = new QLabel(groupBox_3);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 2, 0, 1, 2);


        vboxLayout->addWidget(groupBox_3);

        groupBox = new QGroupBox(ChartInfoDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        sizePolicy1.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy1);
        gridLayout1 = new QGridLayout(groupBox);
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout1->addWidget(label, 0, 0, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout1->addWidget(label_3, 2, 0, 1, 1);

        airport_icao_edit = new QLineEdit(groupBox);
        airport_icao_edit->setObjectName(QString::fromUtf8("airport_icao_edit"));

        gridLayout1->addWidget(airport_icao_edit, 0, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout1->addWidget(label_2, 1, 0, 1, 1);

        ctry_edit = new QLineEdit(groupBox);
        ctry_edit->setObjectName(QString::fromUtf8("ctry_edit"));

        gridLayout1->addWidget(ctry_edit, 1, 1, 1, 1);

        city_edit = new QLineEdit(groupBox);
        city_edit->setObjectName(QString::fromUtf8("city_edit"));

        gridLayout1->addWidget(city_edit, 2, 1, 1, 1);


        vboxLayout->addWidget(groupBox);

        hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        okButton = new QPushButton(ChartInfoDlg);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        hboxLayout->addWidget(okButton);

        cancelButton = new QPushButton(ChartInfoDlg);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        hboxLayout->addWidget(cancelButton);


        vboxLayout->addLayout(hboxLayout);

        QWidget::setTabOrder(chartname_edit, effdate_edit);
        QWidget::setTabOrder(effdate_edit, magvar_spinbox);
        QWidget::setTabOrder(magvar_spinbox, airport_icao_edit);
        QWidget::setTabOrder(airport_icao_edit, ctry_edit);
        QWidget::setTabOrder(ctry_edit, city_edit);
        QWidget::setTabOrder(city_edit, okButton);
        QWidget::setTabOrder(okButton, cancelButton);

        retranslateUi(ChartInfoDlg);
        QObject::connect(okButton, SIGNAL(clicked()), ChartInfoDlg, SLOT(accept()));
        QObject::connect(cancelButton, SIGNAL(clicked()), ChartInfoDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(ChartInfoDlg);
    } // setupUi

    void retranslateUi(QDialog *ChartInfoDlg)
    {
        ChartInfoDlg->setWindowTitle(QApplication::translate("ChartInfoDlg", "Chart Info Dialog", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("ChartInfoDlg", "Chart Data", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("ChartInfoDlg", "Chart Name:", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("ChartInfoDlg", "Effective Date:", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("ChartInfoDlg", "Magnetic Variation:", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("ChartInfoDlg", "Airport Data", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ChartInfoDlg", "Airport ICAO:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ChartInfoDlg", "City:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("ChartInfoDlg", "Country:", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("ChartInfoDlg", "OK", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("ChartInfoDlg", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ChartInfoDlg: public Ui_ChartInfoDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHART_H
