
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "trayicon.h"

#if (defined(Q_OS_WIN) || defined(Q_OS_LINUX) || defined(Q_OS_MACOS)) && !defined(ANDROID)

#include "dwyco_top.h"
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QWindow>

extern DwycoCore *TheDwycoCore;

TrayIcon::TrayIcon(QObject *parent)
    : QObject(parent)
    , m_trayIcon(new QSystemTrayIcon(this))
    , m_menu(new QMenu())
    , m_timer(new QTimer(this))
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        // platform does not support system tray, silently skip
        return;
    }

    QAction *showAction = m_menu->addAction(QObject::tr("Show Dwyco"));
    QObject::connect(showAction, &QAction::triggered, this, [this]() {
        for (QWindow *w : qGuiApp->topLevelWindows())
        {
            if (w->isTopLevel() && w->objectName() != QLatin1String("QQuickPopupOverlay"))
            {
                w->show();
                w->raise();
                w->requestActivate();
                return;
            }
        }
    });

    m_menu->addSeparator();

    QAction *quitAction = m_menu->addAction(QObject::tr("Quit"));
    QObject::connect(quitAction, &QAction::triggered, this, []() {
        // trigger the engine quit which is connected to QGuiApplication::quit
        // in main.cpp
        if (TheDwycoCore)
            TheDwycoCore->exit();
        qGuiApp->quit();
    });

    m_trayIcon->setContextMenu(m_menu);

    m_timer->setInterval(3000);
    QObject::connect(m_timer, &QTimer::timeout, this, &TrayIcon::updateIcon);
    m_timer->start();

    if (TheDwycoCore)
    {
        QObject::connect(TheDwycoCore, &DwycoCore::tox_connection_status_changed,
                         this, &TrayIcon::updateIcon);
        QObject::connect(TheDwycoCore, &DwycoCore::tox_enabledChanged,
                         this, &TrayIcon::updateIcon);
        QObject::connect(TheDwycoCore, &DwycoCore::invisibleChanged,
                         this, &TrayIcon::updateIcon);
    }

    updateIcon();
    m_trayIcon->show();
}

TrayIcon::~TrayIcon()
{
    delete m_menu;
}

void TrayIcon::updateIcon()
{
    if (!m_trayIcon)
        return;
    m_trayIcon->setIcon(makeIcon());
    m_trayIcon->setToolTip(makeTooltip());
}

QIcon TrayIcon::makeIcon() const
{
    QIcon icon;
    icon.addPixmap(makePixmap(22));
    icon.addPixmap(makePixmap(44));
    return icon;
}

QPixmap TrayIcon::makePixmap(int size) const
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    qreal r = size * 0.22;
    qreal half = size / 2.0;

    QPainterPath path;
    path.addRoundedRect(0, 0, size, size, r, r);

    // Left half — DWYCO
    p.save();
    p.setClipRect(QRectF(0, 0, half + 0.5, size));
    p.fillPath(path, dwycoColor());
    p.restore();

    // Right half — Tox
    p.save();
    p.setClipRect(QRectF(half - 0.5, 0, half + 0.5, size));
    p.fillPath(path, toxColor());
    p.restore();

    // Subtle outline
    p.setPen(QPen(QColor(60, 60, 60), 1));
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);

    // Divider
    p.setPen(QPen(QColor(60, 60, 60), 1));
    p.drawLine(QPointF(half, 2), QPointF(half, size - 2));

    p.end();
    return pm;
}

QColor TrayIcon::dwycoColor() const
{
    if (!TheDwycoCore)
        return QColor(160, 160, 160); // gray

    // Invisible mode overrides everything
    if (TheDwycoCore->get_invisible())
        return QColor(180, 80, 180); // purple

    int db = TheDwycoCore->database_online();
    int chat = TheDwycoCore->chat_online();

    if (db > 0 && chat > 0)
        return QColor(50, 200, 50);   // green — fully connected
    if (db > 0)
        return QColor(240, 160, 30);  // orange — db up, chat not yet
    return QColor(220, 50, 50);       // red — offline
}

QColor TrayIcon::toxColor() const
{
    if (!TheDwycoCore)
        return QColor(160, 160, 160); // gray

    if (!TheDwycoCore->get_tox_enabled())
        return QColor(160, 160, 160); // gray — disabled

    if (TheDwycoCore->get_tox_connected())
        return QColor(50, 200, 50);   // green — connected
    return QColor(220, 50, 50);       // red — enabled but disconnected
}

QString TrayIcon::makeTooltip() const
{
    if (!TheDwycoCore)
        return QStringLiteral("Dwyco Pho");

    QString dwycoStr;
    QString toxStr;

    int db = TheDwycoCore->database_online();
    int chat = TheDwycoCore->chat_online();

    if (TheDwycoCore->get_invisible())
        dwycoStr = QStringLiteral("Invisible");
    else if (db > 0 && chat > 0)
        dwycoStr = QStringLiteral("Online");
    else if (db > 0)
        dwycoStr = QStringLiteral("Connecting");
    else
        dwycoStr = QStringLiteral("Offline");

    if (!TheDwycoCore->get_tox_enabled())
        toxStr = QStringLiteral("Disabled");
    else if (TheDwycoCore->get_tox_connected())
        toxStr = QStringLiteral("Connected");
    else
        toxStr = QStringLiteral("Disconnected");

    return QStringLiteral("Dwyco: %1 | Tox: %2").arg(dwycoStr, toxStr);
}

#endif
