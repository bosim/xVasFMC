/********************************************************************************
** Form generated from reading UI file 'textelemdlg.ui'
**
** Created: Fri 15. Jun 19:19:19 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEXTELEMDLG_H
#define UI_TEXTELEMDLG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_TextElemDlg
{
public:
    QGridLayout *gridLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout1;
    QRadioButton *rightbottom_radiobtn;
    QRadioButton *leftbottom_radiobtn;
    QRadioButton *righttop_radiobtn;
    QRadioButton *lefttop_radiobtn;
    QCheckBox *absolute_placing_checkbox;
    QCheckBox *draw_border_checkbox;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QVBoxLayout *vboxLayout;
    QLabel *label_2;
    QTextEdit *text_edit;

    void setupUi(QDialog *TextElemDlg)
    {
        if (TextElemDlg->objectName().isEmpty())
            TextElemDlg->setObjectName(QString::fromUtf8("TextElemDlg"));
        TextElemDlg->resize(452, 455);
        gridLayout = new QGridLayout(TextElemDlg);
#ifndef Q_OS_MAC
        gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        groupBox = new QGroupBox(TextElemDlg);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setEnabled(false);
        gridLayout1 = new QGridLayout(groupBox);
#ifndef Q_OS_MAC
        gridLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        gridLayout1->setContentsMargins(9, 9, 9, 9);
#endif
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        rightbottom_radiobtn = new QRadioButton(groupBox);
        rightbottom_radiobtn->setObjectName(QString::fromUtf8("rightbottom_radiobtn"));

        gridLayout1->addWidget(rightbottom_radiobtn, 1, 1, 1, 1);

        leftbottom_radiobtn = new QRadioButton(groupBox);
        leftbottom_radiobtn->setObjectName(QString::fromUtf8("leftbottom_radiobtn"));

        gridLayout1->addWidget(leftbottom_radiobtn, 1, 0, 1, 1);

        righttop_radiobtn = new QRadioButton(groupBox);
        righttop_radiobtn->setObjectName(QString::fromUtf8("righttop_radiobtn"));

        gridLayout1->addWidget(righttop_radiobtn, 0, 1, 1, 1);

        lefttop_radiobtn = new QRadioButton(groupBox);
        lefttop_radiobtn->setObjectName(QString::fromUtf8("lefttop_radiobtn"));
        lefttop_radiobtn->setChecked(true);

        gridLayout1->addWidget(lefttop_radiobtn, 0, 0, 1, 1);


        gridLayout->addWidget(groupBox, 2, 0, 1, 2);

        absolute_placing_checkbox = new QCheckBox(TextElemDlg);
        absolute_placing_checkbox->setObjectName(QString::fromUtf8("absolute_placing_checkbox"));

        gridLayout->addWidget(absolute_placing_checkbox, 1, 1, 1, 1);

        draw_border_checkbox = new QCheckBox(TextElemDlg);
        draw_border_checkbox->setObjectName(QString::fromUtf8("draw_border_checkbox"));

        gridLayout->addWidget(draw_border_checkbox, 1, 0, 1, 1);

        hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        okButton = new QPushButton(TextElemDlg);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        hboxLayout->addWidget(okButton);

        cancelButton = new QPushButton(TextElemDlg);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        hboxLayout->addWidget(cancelButton);


        gridLayout->addLayout(hboxLayout, 3, 0, 1, 2);

        vboxLayout = new QVBoxLayout();
#ifndef Q_OS_MAC
        vboxLayout->setSpacing(6);
#endif
        vboxLayout->setContentsMargins(0, 0, 0, 0);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        label_2 = new QLabel(TextElemDlg);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        vboxLayout->addWidget(label_2);

        text_edit = new QTextEdit(TextElemDlg);
        text_edit->setObjectName(QString::fromUtf8("text_edit"));
        text_edit->setTabChangesFocus(true);

        vboxLayout->addWidget(text_edit);


        gridLayout->addLayout(vboxLayout, 0, 0, 1, 2);

        QWidget::setTabOrder(text_edit, draw_border_checkbox);
        QWidget::setTabOrder(draw_border_checkbox, absolute_placing_checkbox);
        QWidget::setTabOrder(absolute_placing_checkbox, lefttop_radiobtn);
        QWidget::setTabOrder(lefttop_radiobtn, righttop_radiobtn);
        QWidget::setTabOrder(righttop_radiobtn, leftbottom_radiobtn);
        QWidget::setTabOrder(leftbottom_radiobtn, rightbottom_radiobtn);
        QWidget::setTabOrder(rightbottom_radiobtn, okButton);
        QWidget::setTabOrder(okButton, cancelButton);

        retranslateUi(TextElemDlg);
        QObject::connect(okButton, SIGNAL(clicked()), TextElemDlg, SLOT(accept()));
        QObject::connect(cancelButton, SIGNAL(clicked()), TextElemDlg, SLOT(reject()));
        QObject::connect(absolute_placing_checkbox, SIGNAL(toggled(bool)), groupBox, SLOT(setEnabled(bool)));

        QMetaObject::connectSlotsByName(TextElemDlg);
    } // setupUi

    void retranslateUi(QDialog *TextElemDlg)
    {
        TextElemDlg->setWindowTitle(QApplication::translate("TextElemDlg", "Text Element", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("TextElemDlg", "Reference Corner:", 0, QApplication::UnicodeUTF8));
        rightbottom_radiobtn->setText(QApplication::translate("TextElemDlg", "R&ight Bottom", 0, QApplication::UnicodeUTF8));
        leftbottom_radiobtn->setText(QApplication::translate("TextElemDlg", "L&eft Bottom", 0, QApplication::UnicodeUTF8));
        righttop_radiobtn->setText(QApplication::translate("TextElemDlg", "&Right Top", 0, QApplication::UnicodeUTF8));
        lefttop_radiobtn->setText(QApplication::translate("TextElemDlg", "&Left Top", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        absolute_placing_checkbox->setToolTip(QApplication::translate("TextElemDlg", "Absolute placed items are fixed to a screen position, relative placed items are fixed to a LAT/LON point.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        absolute_placing_checkbox->setText(QApplication::translate("TextElemDlg", "&Place absolute (see tooltip)", 0, QApplication::UnicodeUTF8));
        draw_border_checkbox->setText(QApplication::translate("TextElemDlg", "&Draw borders around the text", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("TextElemDlg", "OK", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("TextElemDlg", "Cancel", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("TextElemDlg", "Text:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TextElemDlg: public Ui_TextElemDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEXTELEMDLG_H
