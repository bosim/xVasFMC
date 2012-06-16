/********************************************************************************
** Form generated from reading UI file 'waypointchoosedlg.ui'
**
** Created: Fri 15. Jun 19:19:19 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WAYPOINTCHOOSEDLG_H
#define UI_WAYPOINTCHOOSEDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTableWidget>

QT_BEGIN_NAMESPACE

class Ui_WaypointChooseDlg
{
public:
    QGridLayout *gridLayout;
    QTableWidget *fixlist;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *okButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *WaypointChooseDlg)
    {
        if (WaypointChooseDlg->objectName().isEmpty())
            WaypointChooseDlg->setObjectName(QString::fromUtf8("WaypointChooseDlg"));
        WaypointChooseDlg->resize(624, 223);
        WaypointChooseDlg->setModal(true);
        gridLayout = new QGridLayout(WaypointChooseDlg);
#ifndef Q_OS_MAC
        gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        fixlist = new QTableWidget(WaypointChooseDlg);
        fixlist->setObjectName(QString::fromUtf8("fixlist"));
        fixlist->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        fixlist->setEditTriggers(QAbstractItemView::NoEditTriggers);
        fixlist->setAlternatingRowColors(true);
        fixlist->setSelectionMode(QAbstractItemView::SingleSelection);
        fixlist->setSelectionBehavior(QAbstractItemView::SelectRows);
        fixlist->setGridStyle(Qt::DashLine);

        gridLayout->addWidget(fixlist, 0, 0, 1, 1);

        hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        okButton = new QPushButton(WaypointChooseDlg);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        hboxLayout->addWidget(okButton);

        cancelButton = new QPushButton(WaypointChooseDlg);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        hboxLayout->addWidget(cancelButton);


        gridLayout->addLayout(hboxLayout, 1, 0, 1, 1);


        retranslateUi(WaypointChooseDlg);
        QObject::connect(okButton, SIGNAL(clicked()), WaypointChooseDlg, SLOT(accept()));
        QObject::connect(cancelButton, SIGNAL(clicked()), WaypointChooseDlg, SLOT(reject()));
        QObject::connect(fixlist, SIGNAL(cellDoubleClicked(int,int)), WaypointChooseDlg, SLOT(accept()));

        QMetaObject::connectSlotsByName(WaypointChooseDlg);
    } // setupUi

    void retranslateUi(QDialog *WaypointChooseDlg)
    {
        WaypointChooseDlg->setWindowTitle(QApplication::translate("WaypointChooseDlg", "Waypoint Choose", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("WaypointChooseDlg", "&Choose", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("WaypointChooseDlg", "&Abort", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class WaypointChooseDlg: public Ui_WaypointChooseDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WAYPOINTCHOOSEDLG_H
