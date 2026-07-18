
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
    , m_appVisible(true)
    , m_backgroundAlert(false)
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
        // new_msg fires before auto-fetch clears unviewed tags, so this
        // catches messages that would never reach any_unviewedChanged(true)
        QObject::connect(TheDwycoCore, &DwycoCore::new_msg,
                         this, [this](const QString&, const QString&, const QString&) {
            if (!m_appVisible)
                m_backgroundAlert = true;
            evaluateBlink();
        });
        QObject::connect(TheDwycoCore, &DwycoCore::any_unviewedChanged,
                         this, [this](bool unviewed) {
            if (unviewed && !m_appVisible)
                m_backgroundAlert = true;
            evaluateBlink();
        });
    }

    QObject::connect(qGuiApp, &QGuiApplication::applicationStateChanged,
                     this, [this](Qt::ApplicationState state) {
        bool wasVisible = m_appVisible;
        m_appVisible = (state == Qt::ApplicationActive);
        if (m_appVisible && !wasVisible) {
            m_backgroundAlert = false;
            evaluateBlink();
        } else if (!m_appVisible && wasVisible) {
            if (TheDwycoCore && TheDwycoCore->get_any_unviewed())
                m_backgroundAlert = true;
            evaluateBlink();
        }
    });
    // initial visibility state
    m_appVisible = (qGuiApp->applicationState() == Qt::ApplicationActive);
    if (!m_appVisible && TheDwycoCore && TheDwycoCore->get_any_unviewed())
        m_backgroundAlert = true;

    m_blinkTimer->setInterval(1000);
    QObject::connect(m_blinkTimer, &QTimer::timeout, this, &TrayIcon::blink);

    evaluateBlink();
    m_trayIcon->show();
}

TrayIcon::~TrayIcon()
{
    delete m_menu;
}

void TrayIcon::evaluateBlink()
{
    bool shouldBlink = !m_appVisible && m_backgroundAlert;
    if (shouldBlink) {
        m_blinkOn = true;
        m_blinkTimer->start();
    } else {
        m_blinkTimer->stop();
    }
    updateIcon();
}

void TrayIcon::updateIcon()
{
    if (!m_trayIcon)
        return;
    m_normalIcon = makeIcon();
    m_dimmedIcon = makeDimmedIcon(m_normalIcon);
    m_trayIcon->setToolTip(makeTooltip());

    if (m_blinkTimer->isActive() && !m_blinkOn)
        m_trayIcon->setIcon(m_dimmedIcon);
    else
        m_trayIcon->setIcon(m_normalIcon);
}

void TrayIcon::blink()
{
    if (!m_trayIcon)
        return;
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

static bool isDwycoGreen(int r, int g, int b)
{
    return g > 200 && r < 50 && b < 50;
}

static bool isDwycoBlack(int r, int g, int b)
{
    return r < 30 && g < 30 && b < 30;
}

QPixmap TrayIcon::makePixmap(int size) const
{
    // Start from greenguy source
    QPixmap base(":/new/prefix1/icons/greenguy.png");
    if (base.isNull()) {
        // fallback: solid green square
        base = QPixmap(32, 32);
        base.fill(Qt::green);
    }

    QImage img = base.toImage().convertToFormat(QImage::Format_ARGB32);

    bool offline = !TheDwycoCore ||
        (TheDwycoCore->database_online() <= 0 && TheDwycoCore->chat_online() <= 0);
    bool invisible = TheDwycoCore && TheDwycoCore->get_invisible();
    bool toxOn = TheDwycoCore && TheDwycoCore->get_tox_connected();

    // Apply color transformations to the 32x32 source
    if (offline) {
        // Flip upside down
        img = img.flipped(Qt::Vertical);
        // Replace green pixels with red
        for (int y = 0; y < img.height(); ++y) {
            for (int x = 0; x < img.width(); ++x) {
                QRgb px = img.pixel(x, y);
                if (isDwycoGreen(qRed(px), qGreen(px), qBlue(px)))
                    img.setPixel(x, y, qRgba(220, 50, 50, qAlpha(px)));
            }
        }
    } else if (invisible) {
        // Tint green -> purple, black -> dark purple
        for (int y = 0; y < img.height(); ++y) {
            for (int x = 0; x < img.width(); ++x) {
                QRgb px = img.pixel(x, y);
                if (isDwycoGreen(qRed(px), qGreen(px), qBlue(px)))
                    img.setPixel(x, y, qRgba(180, 80, 180, qAlpha(px)));
                else if (isDwycoBlack(qRed(px), qGreen(px), qBlue(px)))
                    img.setPixel(x, y, qRgba(100, 30, 100, qAlpha(px)));
            }
        }
    }

    // If tox is connected, tint green -> hot pink
    if (toxOn) {
        for (int y = 0; y < img.height(); ++y) {
            for (int x = 0; x < img.width(); ++x) {
                QRgb px = img.pixel(x, y);
                if (isDwycoGreen(qRed(px), qGreen(px), qBlue(px)))
                    img.setPixel(x, y, qRgba(100, 180, 255, qAlpha(px)));
            }
        }
    }

    // Scale to target size
    QPixmap pm = QPixmap::fromImage(img).scaled(size, size,
        Qt::KeepAspectRatio, Qt::FastTransformation);

    return pm;
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
        int total = TheSyncDescModel->count();
        int connected = TheSyncDescModel->get_connection_count();
        if (total > 0)
            syncStr = QStringLiteral("Sync: %1/%2").arg(connected).arg(total);
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

#endif
