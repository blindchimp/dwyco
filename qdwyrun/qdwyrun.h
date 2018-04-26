#ifndef QDWYRUN_H
#define QDWYRUN_H

#include <QWidget>
#include <QTimer>
#include <QProcess>

namespace Ui {
class qdwyrun;
}

class qdwyrun : public QWidget
{
    Q_OBJECT

public:
    explicit qdwyrun(QWidget *parent = 0);
    ~qdwyrun();
    QTimer idle_timer;
    QProcess *proc;

    void run_proc(QString cmd, QStringList args);
    void run_app();
    void run_update(QString fn);
    void centerWidget(QWidget *w, bool useSizeHint);


public slots:
    void idle();
    void update_finished(int, QProcess::ExitStatus);
    void proc_error(QProcess::ProcessError);
    void update_error(QProcess::ProcessError);
    void done();
    void too_quick(int code, QProcess::ExitStatus e);

    void app_started();

private slots:
    void on_done_button_clicked();

private:
    Ui::qdwyrun *ui;
};

#endif // QDWYRUN_H
