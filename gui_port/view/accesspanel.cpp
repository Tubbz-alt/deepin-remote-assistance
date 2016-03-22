/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QLabel>
#include <QVBoxLayout>

#include "accesspanel.h"
#include <dtextbutton.h>

#include "connectingview.h"
#include "connectedview.h"
#include "errorview.h"
#include "inputview.h"
#include "constants.h"

DWIDGET_USE_NAMESPACE

AccessPanel::AccessPanel(IAccessController* controller, QWidget* p)
    : AbstractPanel(tr("我要求助"), p),
      m_controller(controller)
{
    setObjectName("AccessPanel");
    connect(controller, SIGNAL(noNetwork()), this, SLOT(onNoNetwork()));
    connect(controller, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(controller, SIGNAL(stopped()), this, SLOT(onStopped()));

    if (controller->isAlreadyConnected()) {
        onConnected();
        return;
    }

    auto view = new InputView;
    connect(view, SIGNAL(connect(QString)), this, SLOT(onConnect(QString)));
    connect(view, SIGNAL(cancel()), this, SLOT(onDisconnected()));
    setWidget(view);

    connect(controller, SIGNAL(connecting()), this, SLOT(onConnecting()));
    connect(controller, SIGNAL(connectFailed(AccessErrors)), this, SLOT(onConnectFailed(AccessErrors)));
    controller->initStatus();
}

void AccessPanel::dtor()
{
    if (m_controller != nullptr) {
        m_controller->deleteLater();
        m_controller = nullptr;
    }
}

void AccessPanel::emitChangePanel()
{
    dtor();
    qDebug() <<"";
    emit changePanel(ViewPanel::Main);
}

void AccessPanel::onStopped()
{
    qDebug() << "onStopped";
    emitChangePanel();
}

void AccessPanel::abort()
{
    onDisconnected();
}

void AccessPanel::onConnect(QString token)
{
    m_controller->connect(token);
    onConnecting();
}

void AccessPanel::onConnecting()
{
    setTitle(tr("Accessing"));
    qDebug() << "connecting";
    auto view = new ConnectingView;
    connect(view, SIGNAL(cancel()), this, SLOT(onDisconnected()));
    setWidget(view);
}

void AccessPanel::onConnected()
{
    qDebug() << "connected";
    auto view = new ConnectedView;
    view->setText(tr("You are accessing the desktop shared by other users, you can choose to continue or disconnect"));
    connect(view, SIGNAL(disconnect()), this, SLOT(onDisconnected()));
    setWidget(view);
    connect(m_controller, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    emit connected();
}

void AccessPanel::onConnectFailed(AccessErrors e)
{
    qDebug() << "connect failed";
    setTitle(tr("Remote Assistance"));
    auto view = new ErrorView;
    switch (e) {
    case AccessError::ConnectServerFailed:
        qDebug() << "connect server failed";
        break;
    case AccessError::InvalidToken:
        qDebug() << "invalid token";
        break;
    }
    view->setText(tr("Failed to establish the connection, you can retry to connect"));
    auto button = new DTextButton(tr("Cancel"));
    connect(button, &DTextButton::clicked, [this]{
        emitChangePanel();
    });
    view->addButton(button);
    button = new DTextButton(tr("Retry"));
    button->setEnabled(false);
    // waiting the remoting window to be closed.
    // NB: QTimer::singleShot not support lambda in Qt5.3.
    auto timer = new QTimer(this); // FIXME: this timer may not needed now.
    timer->setInterval(200);
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, [=]{
        qDebug() << "enable retry button";
        button->setEnabled(true);
        timer->deleteLater();
    });
    timer->start();
    connect(button, &DTextButton::clicked, [this]{
        m_controller->retry();
    });
    view->addButton(button);
    setWidget(view);
}

void AccessPanel::onDisconnected()
{
    qDebug() << "disconnected accessing";
    m_controller->disconnect();
    emitChangePanel();
}

void AccessPanel::focus()
{
    if (m_view->objectName() == "InputView")
        qobject_cast<InputView*>(m_view)->focus();
}
