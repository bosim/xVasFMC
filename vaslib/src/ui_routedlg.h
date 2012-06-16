/********************************************************************************
** Form generated from reading UI file 'routedlg.ui'
**
** Created: Fri 15. Jun 19:19:19 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ROUTEDLG_H
#define UI_ROUTEDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RouteDlg
{
public:
    QGridLayout *gridLayout;
    QPushButton *remove_last_item_btn;
    QLabel *label_2;
    QTableWidget *routelist;
    QGroupBox *groupBox;
    QLineEdit *routename;
    QGroupBox *routetype_box;
    QWidget *layoutWidget;
    QVBoxLayout *vboxLayout;
    QRadioButton *routetype_star;
    QRadioButton *routetype_sid;
    QRadioButton *routetype_airway;
    QRadioButton *routetype_unknown;

    void setupUi(QDialog *RouteDlg)
    {
        if (RouteDlg->objectName().isEmpty())
            RouteDlg->setObjectName(QString::fromUtf8("RouteDlg"));
        RouteDlg->resize(382, 446);
        gridLayout = new QGridLayout(RouteDlg);
#ifndef Q_OS_MAC
        gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        remove_last_item_btn = new QPushButton(RouteDlg);
        remove_last_item_btn->setObjectName(QString::fromUtf8("remove_last_item_btn"));
        remove_last_item_btn->setFocusPolicy(Qt::NoFocus);

        gridLayout->addWidget(remove_last_item_btn, 3, 0, 1, 1);

        label_2 = new QLabel(RouteDlg);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(5));
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy);
        label_2->setTextFormat(Qt::AutoText);
        label_2->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout->addWidget(label_2, 2, 0, 1, 1);

        routelist = new QTableWidget(RouteDlg);
        if (routelist->columnCount() < 4)
            routelist->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        routelist->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        routelist->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        routelist->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        routelist->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        routelist->setObjectName(QString::fromUtf8("routelist"));
        routelist->setSelectionMode(QAbstractItemView::NoSelection);
        routelist->setSelectionBehavior(QAbstractItemView::SelectRows);

        gridLayout->addWidget(routelist, 0, 1, 4, 1);

        groupBox = new QGroupBox(RouteDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy1);
        groupBox->setMinimumSize(QSize(110, 60));
        routename = new QLineEdit(groupBox);
        routename->setObjectName(QString::fromUtf8("routename"));
        routename->setGeometry(QRect(10, 30, 91, 22));

        gridLayout->addWidget(groupBox, 0, 0, 1, 1);

        routetype_box = new QGroupBox(RouteDlg);
        routetype_box->setObjectName(QString::fromUtf8("routetype_box"));
        sizePolicy1.setHeightForWidth(routetype_box->sizePolicy().hasHeightForWidth());
        routetype_box->setSizePolicy(sizePolicy1);
        routetype_box->setMinimumSize(QSize(110, 135));
        layoutWidget = new QWidget(routetype_box);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(20, 30, 70, 96));
        vboxLayout = new QVBoxLayout(layoutWidget);
#ifndef Q_OS_MAC
        vboxLayout->setSpacing(6);
#endif
        vboxLayout->setContentsMargins(0, 0, 0, 0);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        vboxLayout->setContentsMargins(0, 0, 0, 0);
        routetype_star = new QRadioButton(layoutWidget);
        routetype_star->setObjectName(QString::fromUtf8("routetype_star"));

        vboxLayout->addWidget(routetype_star);

        routetype_sid = new QRadioButton(layoutWidget);
        routetype_sid->setObjectName(QString::fromUtf8("routetype_sid"));

        vboxLayout->addWidget(routetype_sid);

        routetype_airway = new QRadioButton(layoutWidget);
        routetype_airway->setObjectName(QString::fromUtf8("routetype_airway"));

        vboxLayout->addWidget(routetype_airway);

        routetype_unknown = new QRadioButton(layoutWidget);
        routetype_unknown->setObjectName(QString::fromUtf8("routetype_unknown"));

        vboxLayout->addWidget(routetype_unknown);


        gridLayout->addWidget(routetype_box, 1, 0, 1, 1);

        QWidget::setTabOrder(routename, routetype_star);
        QWidget::setTabOrder(routetype_star, routetype_sid);
        QWidget::setTabOrder(routetype_sid, routetype_airway);
        QWidget::setTabOrder(routetype_airway, routetype_unknown);
        QWidget::setTabOrder(routetype_unknown, routelist);
        QWidget::setTabOrder(routelist, remove_last_item_btn);

        retranslateUi(RouteDlg);

        QMetaObject::connectSlotsByName(RouteDlg);
    } // setupUi

    void retranslateUi(QDialog *RouteDlg)
    {
        RouteDlg->setWindowTitle(QApplication::translate("RouteDlg", "Route", 0, QApplication::UnicodeUTF8));
        remove_last_item_btn->setText(QApplication::translate("RouteDlg", "Remove last item", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("RouteDlg", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'MS Shell Dlg'; font-size:8pt;\">To add items to the</p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'MS Shell Dlg'; font-size:8pt;\">route, simply click</p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'MS Shell Dlg'; font-size:8pt;\">them with  the left </p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'MS Shell Dlg'; font-size:8pt;\">mouse button inside</"
                        "p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'MS Shell Dlg'; font-size:8pt;\">the main chart window.</p></body></html>", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = routelist->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("RouteDlg", "Pos", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = routelist->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("RouteDlg", "ID", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = routelist->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("RouteDlg", "Type", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = routelist->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("RouteDlg", "Labels", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("RouteDlg", "Routename", 0, QApplication::UnicodeUTF8));
        routetype_box->setTitle(QApplication::translate("RouteDlg", "Routetype", 0, QApplication::UnicodeUTF8));
        routetype_star->setText(QApplication::translate("RouteDlg", "STAR", 0, QApplication::UnicodeUTF8));
        routetype_sid->setText(QApplication::translate("RouteDlg", "SID", 0, QApplication::UnicodeUTF8));
        routetype_airway->setText(QApplication::translate("RouteDlg", "Airway", 0, QApplication::UnicodeUTF8));
        routetype_unknown->setText(QApplication::translate("RouteDlg", "Unknown", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class RouteDlg: public Ui_RouteDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ROUTEDLG_H
