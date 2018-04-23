
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VIDSEL_H
#define VIDSEL_H

#include <QWidget>
#include <QListWidgetItem>

namespace Ui {
class VidSel;
}

class VidSel : public QWidget
{
    Q_OBJECT

public:
    explicit VidSel(QWidget *parent = 0);
    ~VidSel();

    int ui_id;
    int save_ui_id;
    int last_index;

    void showEvent(QShowEvent *);
    void load_devlist();
    void init();

public slots:
    void toggle_device(int on);
private slots:
    void on_face_is_blue_clicked(bool checked);
    void on_ok_button_clicked();
    void on_cancel_button_clicked();
    void on_devlist_itemClicked(QListWidgetItem *);
    void on_devlist_currentRowChanged(int);
    void on_format_button_clicked();
    void on_source_button_clicked();
#ifdef USE_VFW
    void on_use_vfw_stateChanged(int);
#endif
    void display_preview(int ui_id, QImage img);
signals:
    void camera_change(int);

private:
    Ui::VidSel *ui;
};

#endif // VIDSEL_H
