/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "generatedview.h"

#include <QApplication>
#include <QClipboard>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>
#include <QFont>
#include <QPixmap>
#include <QBitmap>

#include "widgets/simplebutton.h"
#include "constants.h"
#include "../helper.h"


GeneratedView::GeneratedView(const QString& token, QWidget* p)
    : AbstractView(p),
      m_copyTipVisableTimer(new QTimer(this))
{
    setObjectName("GeneratedView");
    m_copyTipVisableTimer->setInterval(3000);
    QObject::connect(m_copyTipVisableTimer, &QTimer::timeout, [this] {
        m_tokenLabel->setText(m_token);
    });

    initialize();


    m_tokenLabel->setText(token);
    m_token = token;

}

QWidget* GeneratedView::createMainWidget()
{
    auto mainWidget = new QWidget;

    auto layout = new QVBoxLayout(mainWidget);
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addSpacing(70);

    m_tokenLabel = new NotifyLabel(this);
    m_tokenLabel->setObjectName("token");
    m_tokenLabel->setFixedSize(300, 36);
    QFont font = m_tokenLabel->font();
    font.setPixelSize(30);
    m_tokenLabel->setFont(font);

    qDebug() << font.pixelSize() << "------------------------";


    layout->addWidget(m_tokenLabel, 0, Qt::AlignHCenter);


    SimpleButton *button = new SimpleButton(tr("Copy"),this);

    connect(button, &SimpleButton::clicked, [this] {
        m_tokenLabel->setText(tr("Copied to clipboard successfully"));
        m_tipLabel->setText(tr("Connecting, please wait...interface will close after successfully connected"));
        QString token = m_tokenLabel->text();
        QApplication::clipboard()->setText(m_token);
        qDebug() << "Copy Code button on GeneratedView is clicked.";
        m_copyTipVisableTimer->stop();
        m_copyTipVisableTimer->start();
    });

    SimpleButton *buttonn = new SimpleButton(tr("Cancel"));

    connect(buttonn, &SimpleButton::clicked, [this] {
        qDebug() << "cancel button on GeneratedView is clicked";
        emit cancel();
    });


    addButton(button);
    addButton(buttonn);




    m_tipLabel = new TipLabel(this);
    m_tipLabel->setText(tr("To share your desktop, please provide the above verification code to your help provider"));
    m_tipLabel->setFixedSize(DRA::TipLabelMaxWidth, DRA::TipLabelMaxHeight);

    layout->addSpacing(35.6);
    layout->addWidget(m_tipLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(20);
    layout->addWidget(m_copyTip);
    layout->addSpacing(18);
    layout->addStretch();

    return mainWidget;
}