#include "qdwyrun.h"
#include <QApplication>
#include "vc.h"

QString App_to_run;
QString Update_app_name;
QString App_nice_name;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if(argc < 3)
    {
        exit(1);
    }
    App_to_run = a.arguments().at(2);
    Update_app_name = a.arguments().at(1);
    if(argc >= 4)
    {
        App_nice_name = a.arguments().at(3);
    }
    else
    {
        App_nice_name = "App";
    }

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

