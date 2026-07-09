
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
#include "syncmodel.h"
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
    , m_blinkTimer(new QTimer(this))
    , m_blinkOn(true)
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
        QObject::connect(TheDwycoCore, &DwycoCore::active_group_nameChanged,
                         this, &TrayIcon::updateIcon);
        QObject::connect(TheDwycoCore, &DwycoCore::group_statusChanged,
                         this, &TrayIcon::updateIcon);
        QObject::connect(TheDwycoCore, &DwycoCore::group_private_key_validChanged,
                         this, &TrayIcon::updateIcon);
    }
    if (TheSyncDescModel)
    {
        QObject::connect(TheSyncDescModel, &SyncDescModel::connection_countChanged,
                         this, &TrayIcon::updateIcon);
    }

    if (TheDwycoCore)
    {
        QObject::connect(TheDwycoCore, &DwycoCore::any_unviewedChanged,
                         this, [this](bool unviewed) {
            if (unviewed) {
                m_blinkOn = true;
                m_blinkTimer->start();
            } else {
                m_blinkTimer->stop();
            }
            updateIcon();
        });
    }
    m_blinkTimer->setInterval(1000);
    QObject::connect(m_blinkTimer, &QTimer::timeout, this, &TrayIcon::blink);

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
    m_normalIcon = makeIcon();
    m_dimmedIcon = makeDimmedIcon(m_normalIcon);
    m_trayIcon->setToolTip(makeTooltip());

    if (TheDwycoCore && TheDwycoCore->get_any_unviewed() && !m_blinkOn)
        m_trayIcon->setIcon(m_dimmedIcon);
    else
        m_trayIcon->setIcon(m_normalIcon);
}

void TrayIcon::blink()
{
    if (!m_trayIcon)
        return;
    if (!TheDwycoCore || !TheDwycoCore->get_any_unviewed()) {
        m_blinkTimer->stop();
        updateIcon();
        return;
    }
    m_blinkOn = !m_blinkOn;
    if (m_blinkOn)
        m_trayIcon->setIcon(m_normalIcon);
    else
        m_trayIcon->setIcon(m_dimmedIcon);
}

QIcon TrayIcon::makeDimmedIcon(const QIcon &normal) const
{
    QIcon dimmed;
    for (int size : {22, 24, 32, 44, 48, 64}) {
        QPixmap pm = normal.pixmap(size, size);
        QPixmap dim(size, size);
        dim.fill(Qt::transparent);
        QPainter p(&dim);
        p.setOpacity(0.35);
        p.drawPixmap(0, 0, pm);
        p.end();
        dimmed.addPixmap(dim);
    }
    return dimmed;
}

QIcon TrayIcon::makeIcon() const
{
    QIcon icon;
    icon.addPixmap(makePixmap(22));
    icon.addPixmap(makePixmap(24));
    icon.addPixmap(makePixmap(32));
    icon.addPixmap(makePixmap(44));
    icon.addPixmap(makePixmap(48));
    icon.addPixmap(makePixmap(64));
    return icon;
}

QPixmap TrayIcon::makePixmap(int size) const
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    qreal r = size * 0.22;

    QPainterPath path;
    path.addRoundedRect(0, 0, size, size, r, r);

    if (isInGroup())
    {
        qreal dwycoEnd = size * 0.30;
        qreal syncEnd = size * 0.70;

        // DWYCO panel (left)
        p.save();
        p.setClipRect(QRectF(0, 0, dwycoEnd + 0.5, size));
        p.fillPath(path, dwycoColor());
        p.restore();

        // Sync panel (middle)
        p.save();
        p.setClipRect(QRectF(dwycoEnd - 0.5, 0, syncEnd - dwycoEnd + 1, size));
        p.fillPath(path, QColor(60, 60, 60));
        p.restore();

        // Tox panel (right)
        p.save();
        p.setClipRect(QRectF(syncEnd - 0.5, 0, size - syncEnd + 0.5, size));
        p.fillPath(path, toxColor());
        p.restore();

        // Cloud icon centered in sync panel
        QString iconPath = hasSyncConnections()
            ? QStringLiteral(":/material/icons/material/mdpi/ic_cloud_done_white_24dp.png")
            : QStringLiteral(":/material/icons/material/mdpi/ic_cloud_off_white_24dp.png");
        QPixmap cloudIcon(iconPath);
        if (!cloudIcon.isNull()) {
            qreal syncPanelW = syncEnd - dwycoEnd;
            qreal iconSize = syncPanelW * 0.8;
            qreal iconX = dwycoEnd + (syncPanelW - iconSize) / 2;
            qreal iconY = (size - iconSize) / 2;
            QPixmap scaled = cloudIcon.scaled(iconSize, iconSize,
                                              Qt::KeepAspectRatio, Qt::SmoothTransformation);
            p.drawPixmap(QPointF(iconX, iconY), scaled);
        }

        // Dividers
        p.setPen(QPen(QColor(60, 60, 60), 1));
        p.drawLine(QPointF(dwycoEnd, 2), QPointF(dwycoEnd, size - 2));
        p.drawLine(QPointF(syncEnd, 2), QPointF(syncEnd, size - 2));
    }
    else
    {
        qreal half = size / 2.0;

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

        // Divider
        p.setPen(QPen(QColor(60, 60, 60), 1));
        p.drawLine(QPointF(half, 2), QPointF(half, size - 2));
    }

    // Subtle outline
    p.setPen(QPen(QColor(60, 60, 60), 1));
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);

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

    QString syncStr;
    if (isInGroup()) {
        if (hasSyncConnections())
            syncStr = QStringLiteral("Sync: OK (%1)").arg(TheSyncDescModel->get_connection_count());
        else
            syncStr = QStringLiteral("Sync: Off");
    }

    if (!syncStr.isEmpty())
        return QStringLiteral("Dwyco: %1 | Tox: %2 | %3").arg(dwycoStr, toxStr, syncStr);
    return QStringLiteral("Dwyco: %1 | Tox: %2").arg(dwycoStr, toxStr);
}

bool TrayIcon::isInGroup() const
{
    if (!TheDwycoCore)
        return false;
    return TheDwycoCore->get_active_group_name().length() > 0
        && TheDwycoCore->get_group_status() == 0
        && TheDwycoCore->get_group_private_key_valid() == 1;
}

bool TrayIcon::hasSyncConnections() const
{
    if (!TheSyncDescModel)
        return false;
    return TheSyncDescModel->get_connection_count() > 0;
}

#endif
