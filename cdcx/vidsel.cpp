
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QtGui>
#include <QMessageBox>
#include "vidsel.h"
#include "ui_vidsel.h"
#include "dlli.h"
#include "ssmap.h"
#include "mainwin.h"
#ifdef USE_VFW
#include <windows.h>
#endif
#ifdef CDCX_RELEASE
#undef FILES
#else
#define FILES
#endif
//#define FILES

#ifdef FILES
#define FILES_OFFSET 2
#else
#define FILES_OFFSET 1
#endif

int HasCamera;
int HasCamHardware;
extern int FirstRun;
int AvoidCamera;
// note: on windows, apparently some drivers will fire up
// threads to perform initialization asynchronously.
// this causes us a problem, because we are assuming when
// a dwyco* call is in progress, we aren't calling any other
// dwyco* calls (ie, we aren't reentrant or thread-safe.
// so, we provide "blocking" versions of some of the video
// calls to workaround this windows video driver stuff.
extern int Block_DLL;

int
block_enable_video_capture_preview(int on, int *ui_id)
{
    Block_DLL = 1;
    int ret = dwyco_enable_video_capture_preview(on);
    Block_DLL = 0;
    // this is super-dumb hack-around. when we turn off
    // preview, we sometimes need to assume the device will have
    // been shutdown before we start tweaking with other video
    // capture related stuff. unfortunately, the dll doesn't provide
    // a clean interface to "shut the device up NOW". so we'll
    // just spin a little bit and hope for the best.
    int dum;
    for(int i = 0; i < 100; ++i)
        dwyco_service_channels(&dum);
    return ret;
}

VidSel::VidSel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VidSel)
{
    ui->setupUi(this);
    ui_id = -1;
    save_ui_id = -1;
    last_index = -1;
    // note: the rest of init is done below, since
    // emitting signals in a constructor is a bit of
    // a dicey situation.
    connect(this, SIGNAL(camera_change(int)), Mainwinform, SIGNAL(camera_change(int)));
    connect(Mainwinform, SIGNAL(vid_control(int)), this, SLOT(toggle_device(int)));
    //connect(Mainwinform, SIGNAL(video_display(int,QImage)), this, SLOT(display_preview(int,QImage)));
    setAttribute(Qt::WA_QuitOnClose, 0);
}

VidSel::~VidSel()
{
    delete ui;
}

void
VidSel::display_preview(int ui_id, QImage img)
{
    if(ui_id == this->ui_id)
        ui->video_label->setPixmap(QPixmap::fromImage(img));
}

void
VidSel::init()
{
    int use_vfw = 0;
    setting_get("use vfw", use_vfw);
#ifdef USE_VFW
    ui->use_vfw->setChecked(use_vfw);
    ui->format_button->setVisible(use_vfw);
    ui->face_is_blue->setVisible(!use_vfw);
    //ui->use_vfw->setEnabled(0);
#else
    ui->use_vfw->setVisible(0);
    ui->format_button->setVisible(0);
#endif
    const char *val;
    int len;
    int type;
    if(dwyco_get_setting("video_format/swap_rb", &val, &len, &type))
    {
        if(type == DWYCO_TYPE_INT)
        {
            int srb = atol(val);
            ui->face_is_blue->setChecked(srb);
        }
    }

    //note: setchecked can cause the signal to be emitted above,
    // and the list to be loaded.
    if(ui->devlist->count() == 0)
        load_devlist();

}


void
VidSel::on_ok_button_clicked()
{
    QListWidgetItem *ci = ui->devlist->currentItem();
    if(!ci)
        return;
    QVariant v = ci->data(Qt::DisplayRole);
    QByteArray ba = v.toByteArray();

    setting_put("video device", DwOString(ba.constData(), 0, ba.length()));
    settings_save();
    //block_enable_video_capture_preview(0, &ui_id);
    save_ui_id = ui_id;
    ui_id = -1; // don't blit here anymore
    ui->video_label->ui_id = -1;
    hide();
}

void
VidSel::showEvent(QShowEvent *se)
{
    if(ui->devlist->currentRow() == 0)
        return;
    //block_enable_video_capture_preview(1, &ui_id);
    ui->video_label->ui_id = DWYCO_VIDEO_PREVIEW_CHAN;
    save_ui_id = -1;
    QWidget::showEvent(se);
}

void
VidSel::on_cancel_button_clicked()
{
    //block_enable_video_capture_preview(0, &ui_id);
    ui->video_label->ui_id = -1;
}

void
VidSel::on_devlist_itemClicked(QListWidgetItem *item)
{

}

void
VidSel::toggle_device(int on)
{
    if(!HasCamHardware)
        return;
    if(on == -1)
        return; // pause is handled above this level
    if(HasCamera && on)
        return;
    if(!HasCamera && !on)
        return;
    if(on)
    {
        // if nothing selected before, just select the first
        // index (we know there is one, because HasCamHardware is set.
        if(last_index == -1 || last_index == 0)
            last_index = 1;
        ui->devlist->setCurrentRow(last_index);

    }
    else
    {
        int save_last = last_index;
        ui->devlist->setCurrentRow(0);
        last_index = save_last;
    }

}

void
VidSel::on_devlist_currentRowChanged(int r)
{
    //ui_id = -1;
    block_enable_video_capture_preview(0, &ui_id);
    ui_id = -1;
    ui->video_label->ui_id = -1;
    HasCamera = 0;
    //emit camera_change(0);
    dwyco_shutdown_vfw();
    dwyco_set_setting("video_input/no_video", "1");
    //dwyco_set_setting("video_input/vfw", "0");
    //dwyco_set_setting("video_input/raw", "0");
    ui->video_label->clear();
    if(r == -1)
    {
        emit camera_change(0);
        return;
    }
    if(r > 0)
    {
        Block_DLL = 1;
        if(r > FILES_OFFSET - 1) // make this "1" if you are going to have the (files) option
        {
#ifdef USE_VFW
            dwyco_start_vfw(r - FILES_OFFSET, ::GetDesktopWindow(), ::GetDesktopWindow()); // -2 for (files)
#else
            dwyco_start_vfw(r - FILES_OFFSET, 0, 0); // -2 for (files)
#endif
            dwyco_set_setting("video_input/no_video", "0");
            dwyco_set_setting("video_input/source", "camera");
            //dwyco_set_setting("video_input/vfw", "1");
            //dwyco_set_setting("video_input/raw", "0");
        }
        else
        {
            dwyco_set_setting("video_input/no_video", "0");
            dwyco_set_setting("video_input/source", "raw");
            //dwyco_set_setting("video_input/vfw", "0");
            //dwyco_set_setting("video_input/raw", "1");
        }
        Block_DLL = 0;
        block_enable_video_capture_preview(1, &ui_id);
        ui->video_label->ui_id = DWYCO_VIDEO_PREVIEW_CHAN;

        HasCamera = 1;

        emit camera_change(HasCamera);
    }
    else
        emit camera_change(0);
    last_index = r;
}

void
VidSel::load_devlist()
{
    DwOString vid_dev("(No Video)");
    ui->devlist->addItem(vid_dev.c_str());
#ifdef FILES
    DwOString file_dev("(Files)");
    ui->devlist->addItem(file_dev.c_str());
    HasCamHardware = 1;
#endif

    setting_get("video device", vid_dev);

    DWYCO_LIST l;
    Block_DLL = 1;
    l = dwyco_get_vfw_drivers();
    Block_DLL = 0;
    if(l)
    {
        int n;
        dwyco_list_numelems(l, &n, 0);
        int already_selected = 0;
        for(int i = 0; i < n; ++i)
        {
            const char *out;
            int len;
            int type;
            if(!dwyco_list_get(l, i, DWYCO_VFW_DRIVER_NAME,
                               &out, &len, &type))
                continue;
            ui->devlist->addItem(out);
            HasCamHardware = 1;
            if(!AvoidCamera)
            {
                if(vid_dev == DwOString(out, 0, len))
                {
                    ui->devlist->item(ui->devlist->count() - 1)->setSelected(1);
                    ui->devlist->setCurrentRow(i + FILES_OFFSET); // +2 if (files)
                }
                else if(FirstRun && !already_selected)
                {
                    // just select the first camera we come across
                    already_selected = 1;
                    ui->devlist->item(ui->devlist->count() - 1)->setSelected(1);
                    ui->devlist->setCurrentRow(i + FILES_OFFSET); // +2 if (files)
                    QListWidgetItem *ci = ui->devlist->currentItem();
                    if(ci)
                    {
                        QVariant v = ci->data(Qt::DisplayRole);
                        QByteArray ba = v.toByteArray();

                        setting_put("video device", DwOString(ba.constData(), 0, ba.length()));
                        settings_save();
                    }
                }
            }
        }
    }
    if(AvoidCamera || ui->devlist->currentRow() == -1)
        ui->devlist->setCurrentRow(0);
}

#ifdef USE_VFW
void
VidSel::on_use_vfw_stateChanged(int state)
{
    on_devlist_currentRowChanged(0);
    Block_DLL = 1;
    dwyco_set_external_video(!state);
    Block_DLL = 0;
    setting_put("use vfw", state);
    settings_save();
    ui->format_button->setVisible(state);
    ui->face_is_blue->setVisible(!state);
    ui->devlist->clear();

    load_devlist();
    //ui_id = -1;
    //save_ui_id = -1;


}

#endif

void
VidSel::on_format_button_clicked()
{
    Block_DLL = 1;
    dwyco_vfw_format();
    Block_DLL = 0;
}

void
VidSel::on_source_button_clicked()
{
    Block_DLL = 1;
#ifdef MAC_CLIENT
    dwyco_vfw_source();
#else
#ifdef USE_VFW
    if(!dwyco_vfw_source())
        QMessageBox::information(this,
                                 "Error", " Can't show camera source adjustments dialog");
#else
    QMessageBox::information(this,
                             "Sorry", "No camera adjustment available at the moment.");
#endif
#endif
    Block_DLL = 0;
}



void VidSel::on_face_is_blue_clicked(bool checked)
{
    dwyco_set_setting("video_format/swap_rb", checked ? "1" : "0");
}
