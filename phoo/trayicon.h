
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef TRAYICON_H
#define TRAYICON_H

#include <QObject>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>

class TrayIcon : public QObject
{
    Q_OBJECT
public:
    explicit TrayIcon(QObject *parent = nullptr);
    ~TrayIcon() override;

private slots:
    void updateIcon();

private:
    QIcon makeIcon() const;
    QPixmap makePixmap(int size) const;
    QColor dwycoColor() const;
    QColor toxColor() const;
    QString makeTooltip() const;
    bool isInGroup() const;
    bool hasSyncConnections() const;

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_menu;
    QTimer *m_timer;
};

#endif
