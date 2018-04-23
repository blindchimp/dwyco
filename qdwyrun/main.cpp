#include "qdwyrun.h"
#include <QApplication>
#include "vc.h"

QString app_to_run;
QString update_app;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if(argc < 3)
    {
        exit(1);
    }
    app_to_run = a.arguments().at(2);
    update_app = a.arguments().at(1);

    // figure out what to do on the mac, since the launcher may be in
    // another place and we might need to change to the lib folder before doing
    // the update.

    vc::non_lh_init();


    qdwyrun w;
    w.show();
    w.centerWidget(&w, 1);
    
    return a.exec();
}

void
oopanic(const char *)
{
    exit(0);
}

