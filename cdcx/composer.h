
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef composer_h
#define composer_h

#include <QMainWindow>
#include <QCloseEvent>
#include <QTimer>
#include "ui_composer.h"
#include "dwstr.h"
#include "dvp.h"
#include "dlli.h"
// base class for msg composer, subclasses are
// forward msgs
// special msgs (for pal auth, for example)
// profile creation
// viewing individual msgs

#define COMP_STYLE_REGULAR 0
#define COMP_STYLE_FILE 1
#define COMP_STYLE_FORWARD 2
#define COMP_STYLE_SPECIAL 3
#define COMP_STYLE_AUTOREPLY 4
struct sender : public QObject
{
    Q_OBJECT
public:
    sender() : vp(this) {}
    ~sender() {
        vp.invalidate();
    }
    DVP vp;
    DVP cvp; // composer for this send context
    DwOString uid; // uid of recipient
    DwOString pers_id; // id of q-msg
public slots:
    void pers_msg_send_status(int status, DwOString pid, DwOString ruid);
    void msg_progress(DwOString pid, DwOString ruid, DwOString msg, int percent);
    void stop_and_remove();

};

class composer : public QMainWindow
{
    Q_OBJECT

protected:
    static void DWYCOCALLCONV dwyco_record_profile_done(int /*id*/, void *arg);
    static void DWYCOCALLCONV dwyco_record_done(int /*id*/, void *arg);

public:
    composer(int style = 0, int special_type = 0, QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~composer();
    static QList<DVP> Composers;

protected:
    int compid;
    int chan_id; // audio channel id
    DVP vp;
    DwOString uid;
    int special_type;
    int forward;
    DwOString pers_id;
    DwOString image_fn;

public:
    int ui_id;
    int has_attachment;
    void set_uid(const DwOString& uid);
    QList<DwOString> compids;

    int failed; // at least one of a multi-send failed
    int multi_recipient;
    int send_in_progress;
    int modified;
    virtual int get_composition();
    virtual void switch_composition() {
        return ;
    }
    virtual void closeEvent(QCloseEvent *ev);

protected slots:
    virtual void on_actionRecord_triggered(bool);
    virtual void on_actionPlay_triggered(bool);
    virtual void on_actionStop_triggered(bool);
    virtual void on_actionStart_over_triggered(bool);
    virtual void on_actionSend_Message_triggered(bool);
    virtual void on_actionCancel_triggered(bool);
    virtual void on_actionShow_triggered(bool);
    virtual void on_actionSelect_triggered(bool);
    virtual void on_actionDon_t_allow_forwarding_triggered(bool);
    virtual void on_actionHi_Quality_Video_triggered(bool);

    virtual void on_actionRecord_Video_triggered(bool);
    virtual void on_actionRecord_Audio_triggered(bool);
    virtual void on_actionRecord_Picture_triggered(bool);

    virtual void text_changed();
    virtual void line_text_changed(const QString&);

    virtual void camera_event(int);
    virtual void animate();
    virtual void update_recipient_decorations(DwOString);
    virtual void audio_recording_event(int, int);
    virtual void pers_msg_send_status(int status, DwOString pid, DwOString ruid);
    virtual void msg_progress(DwOString pid, DwOString ruid, DwOString msg, int percent);


public:
    Ui::Composer ui;
private:
    QTimer animation_timer;
    QLabel *blinker;
public:
    int blink;
    int recording;
    int recording_audio;

private slots:
    virtual void on_actionIgnore_user_triggered();
    virtual void on_actionCompose_Message_triggered();
    virtual void on_actionShow_Chatbox_triggered();
signals:
    void ignore_event(DwOString);
    void pal_event(DwOString);
    void audio_recording(int, int);
    void send_msg_event(DwOString);
    void user_cancel();
};

class composer_profile : public composer
{
    Q_OBJECT

    static void DWYCOCALLCONV
    dwyco_profile_composer_fetch_done(int succ, const char *reason,
                                      const char *s1, int len_s1,
                                      const char *s2, int len_s2,
                                      const char *s3, int len_s3,
                                      const char *filename,
                                      const char *uid, int len_uid,
                                      int reviewed, int regular,
                                      void *arg);
    static void DWYCOCALLCONV dwyco_record_profile_done(int /*id*/, void *arg);

public:
    composer_profile(QWidget *parent = 0, Qt::WindowFlags f = 0) ;
    static composer_profile *get_composer_profile(const DwOString& uid);

    int viewid;

    void start();
    void closeEvent(QCloseEvent *);

protected slots:
    virtual void on_actionRecord_triggered(bool);
    virtual void on_actionSend_Message_triggered(bool);
    virtual void on_actionStart_over_triggered(bool);
    virtual void camera_event(int);
};

class viewer_profile : public composer
{
    Q_OBJECT

    friend void
    DWYCOCALLCONV
    dwyco_profile_fetch_done(int succ, const char *reason,
                             const char *s1, int len_s1,
                             const char *s2, int len_s2,
                             const char *s3, int len_s3,
                             const char *filename,
                             const char *uid, int len_uid,
                             int reviewed, int regular,
                             void *arg);
public:
    viewer_profile(QWidget *parent = 0, Qt::WindowFlags f = 0) ;
    virtual ~viewer_profile() {}
    int viewid;
    static viewer_profile *get_viewer_profile(const DwOString& uid);
    static void delete_all();

    int start_fetch();
    void closeEvent(QCloseEvent *);

protected slots:
    void update_profile(DwOString);
    virtual void on_actionPlay_triggered(bool);
    virtual void on_actionStop_triggered(bool);
    virtual void camera_event(int);

};

class composer_file : public composer
{
    Q_OBJECT

public:
    composer_file(QString fn, QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~composer_file() {}
    QString filename;
protected:
    virtual int get_composition();
    virtual void switch_composition();


protected slots:
    //virtual void on_actionSend_Message_triggered(bool);
    virtual void camera_event(int);

};

class composer_forward : public composer
{
    Q_OBJECT
public:

    composer_forward(QWidget *par = 0, Qt::WindowFlags f = 0);
    virtual ~composer_forward() {}

    int init(QByteArray uid, DwOString mid);
    QByteArray uid_of_previous_sender;
    DwOString mid_to_forward;
protected:
    virtual int get_composition();
    virtual void switch_composition();

protected slots:
    //virtual void on_actionSend_Message_triggered(bool);
    virtual void camera_event(int);
};

#if 0
class composer_autoreply : public composer
{
    Q_OBJECT
public:

    composer_autoreply(QWidget *par = 0, Qt::WindowFlags f = 0);
    virtual ~composer_autoreply() {}


protected:


protected slots:
    virtual void on_actionSend_Message_triggered(bool);
    //virtual void camera_event(int);
};
#endif

void preview_file_zap(DWYCO_SAVED_MSG_LIST sm, vidlab *lab, DwOString user_fn, QByteArray uid, DwOString mid, int no_resize = 0);
#endif
