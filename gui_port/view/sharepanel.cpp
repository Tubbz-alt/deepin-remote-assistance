/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QLabel>
#include <dseparatorhorizontal.h>
#include <dtextbutton.h>

#include "constants.h"

#include "sharepanel.h"
#include "interface.h"
#include "generatingview.h"
#include "generatedview.h"
#include "connectedview.h"
#include "errorview.h"

DWIDGET_USE_NAMESPACE

SharePanel::SharePanel(IShareController* controller, QWidget* p)
    : AbstractPanel(tr(" "), p),
      m_controller(controller)
{
    setObjectName("SharePanel");
    connect(controller, SIGNAL(noNetwork()), this, SLOT(onNoNetwork()));
    connect(controller, SIGNAL(stopped()), this, SLOT(onStopped()));

    if (controller->isSharing()) {
        onSharing();
        return;
    }

    connect(controller, SIGNAL(sharing()), this, SLOT(onSharing()));
    connect(controller, SIGNAL(generatingAccessToken()), this, SLOT(onGeneratingAccessToken()));
//    connect(controller, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
//    connect(controller, SIGNAL(genAccessTokenFailed()), this, SLOT(onGenAccessTokenFailed()));
    connect(controller, SIGNAL(genAccessTokenSuccessed(QString)), this, SLOT(onGenAccessTokenSuccessed(QString)));
    controller->startGenAccessToken();
}
SharePanel::~SharePanel()
{
    dtor();
}


void SharePanel::dtor()
{
    if (m_controller != nullptr) {
        m_controller->deleteLater();
        m_controller = nullptr;
    }
}

void SharePanel::emitChangePanel()
{
    dtor();
    qDebug() <<"emit changePanel";
    emit changePanel(ViewPanel::Main);
}

void SharePanel::abort()
{
    onDisconnected();
    emitChangePanel();
}

void SharePanel::onStopped()
{
    qDebug() << "onStopped";
    emitChangePanel();
}

void SharePanel::onSharing()
{
    qDebug() << "sharing";
    auto view = new ConnectedView;
    view->setText(tr("正在进行远程协助"));
    connect(view, SIGNAL(disconnect()), this, SLOT(onDisconnectedWithAsk()));
    setWidget(view);
}

void SharePanel::onGeneratingAccessToken()
{
    qDebug() << "generating access token";
    auto view = new GeneratingView;
    connect(view, SIGNAL(cancel()), this, SLOT(onDisconnected()));
    setWidget(view);
}

void SharePanel::onDisconnectedWithAsk()
{
    qDebug() << "disconnect";
    m_controller->disconnect();



}

void SharePanel::onDisconnected()
{
    qDebug() << "disconnect immedately"<< m_controller->isSharing();
    m_controller->cancel();
    emitChangePanel();
}

void SharePanel::onGenAccessTokenFailed()
{
    setTitle(tr("Remote Assistance"));
    qDebug() << "gen access token failed";
    auto view = new ErrorView;
    auto button = new DTextButton(tr("Cancel"));
    QObject::connect(button, &DTextButton::clicked, [this]{
        onDisconnectedWithAsk();
    });
    view->addButton(button);
    button = new DTextButton(tr("Retry"));
    QObject::connect(button, &DTextButton::clicked, [this]{
        setTitle(tr("Sharing"));
        m_controller->retry();
    });
    view->addButton(button);
    setWidget(view->setText(tr("Failed to establish the connection, you can retry to connect")));
}

void SharePanel::onGenAccessTokenSuccessed(QString token)
{
    qDebug() << "gen access token done";
    auto view = new GeneratedView(token);
    //connect(
    connect(view, SIGNAL(cancel()), this, SLOT(onDisconnected()));
    view->show();
    setWidget(view);
}
