
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QScreen>
#include <QSettings>
#include <QHostInfo>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QDebug>
#include <QQmlFileSelector>
#include "testagent.h"
#ifdef ANDROID
#include "notificationclient.h"
#include <QJniObject>
typedef QJniObject QAndroidJniObject;
#endif

QQmlApplicationEngine *TheEngine;

void dwyco_register_qml(QQmlContext *root);
void start_desktop_background();

#ifdef ANDROID
NotificationClient *notificationClient;
#endif
static
void
myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
#if 1
    if(msg.contains("Timers cannot be stopped from another thread"))
        ::abort();
    if(msg.contains("Timers can only be used with threads started with QThread"))
        ::abort();
#endif

}

// Pulls the GUI test agent's own flags out of argv before the
// application object is built. This has to happen up front: Qt copies
// argc/argv into QGuiApplication::arguments(), and setup_locations()
// (dwyco_top.cpp:1366) treats argv[1] as the profile directory, so any
// extra flag we leave in there would be mistaken for the profile path.
// Rewriting argv in place means `phoo /tmp/prof --test-agent /tmp/sock`
// and `phoo --test-agent /tmp/sock /tmp/prof` both work.
static
void
phoo_extract_test_args(int &argc, char **argv, QString &sock, bool &seed, bool &enabled)
{
    int out = 1;
    for(int i = 1; i < argc; ++i) {
        const QByteArray a(argv[i]);
        if(a == "--test-agent") {
            enabled = true;
            if(i + 1 < argc)
                sock = QString::fromLocal8Bit(argv[++i]);
            continue;
        }
        if(a == "--test-no-seed") {
            seed = false;
            continue;
        }
        argv[out++] = argv[i];
    }
    argv[out] = nullptr;
    argc = out;
}

#if 0
#include <mutex>
int Go;
int Go_f;
std::mutex Go_mutex;
#endif

int main(int argc, char *argv[])
{
    //Go = 1;
#if 0 && defined(DWYCO_RELEASE)
    qInstallMessageHandler(myMessageOutput);
#endif

    QString test_socket;
    bool test_seed = true;
    bool test_agent = false;
    phoo_extract_test_args(argc, argv, test_socket, test_seed, test_agent);

#ifdef ANDROID
//    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif


    QGuiApplication app(argc, argv);

#if 0 && defined(_WIN32)
    QQuickStyle::setStyle("Fusion");
    QQuickStyle::setFallbackStyle("Fusion");
#else
    QQuickStyle::setStyle("Material");
    QQuickStyle::setFallbackStyle("Material");
#endif

    //qDebug() << QQuickStyle::availableStyles();

    // note: qt seems to use some of these names in constructing
    // file names. this can be a problem if different FS's with different
    // naming conventions are being used. this manifests itself with
    // file names conflicting, and problems copying existing files from
    // different machines. it gets worse if you are using file syncing
    // things like dropbox and btsync.
    QCoreApplication::setOrganizationName("dwyco");
    QCoreApplication::setOrganizationDomain("dwyco.com");
    QCoreApplication::setApplicationName(QString("phoo"));
    QSettings::setDefaultFormat(QSettings::IniFormat);
    // note: need to set the path to the right place, same as fn_pfx for dll
    //QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, FPATH);

    QQmlApplicationEngine engine;
    TheEngine = &engine;
    QQmlFileSelector *sel = new QQmlFileSelector(TheEngine);
    QStringList sels;
#if defined(DWYCO_FORCE_DESKTOP_VGQT) || defined(ANDROID) || defined(DWYCO_IOS)
    //sels.append("vgqt");
#endif

#if (defined(Q_OS_WIN) || defined(Q_OS_LINUX) || defined(Q_OS_MACOS)) && !defined(ANDROID)
    sels.append("desktop");
#endif
    if(sels.count() > 0)
        sel->setExtraSelectors(sels);


#ifdef ANDROID
    notificationClient = new NotificationClient(&engine);
    engine.rootContext()->setContextProperty(QLatin1String("notificationClient"),
            notificationClient);
    notificationClient->set_allow_notification(0);
#endif

    dwyco_register_qml(engine.rootContext());
    // this screen resolution stuff was stolen from
    // qt world summit talk 2015 by vplay guy
    QScreen *screen = app.primaryScreen();

    qreal dpi;
#if defined(Q_OS_WIN)
    dpi = screen->logicalDotsPerInch() * app.devicePixelRatio();
#elif defined(Q_OS_ANDROID)
    QAndroidJniObject qtActivity =
        QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative",
                "activity", "()Landroid/app/Activity;");
    QAndroidJniObject resources = qtActivity.callObjectMethod("getResources",
                                  "()Landroid/content/res/Resources;");
    QAndroidJniObject displayMetrics = resources.callObjectMethod("getDisplayMetrics",
                                       "()Landroid/util/DisplayMetrics;");
    int density = displayMetrics.getField<int>("densityDpi");
    dpi = density;
#else
    dpi = screen->physicalDotsPerInch() * app.devicePixelRatio();
#endif


    engine.rootContext()->setContextProperty("screenDpi", dpi);
#ifdef DWYCO_DEBUG
    engine.rootContext()->setContextProperty("dwyco_debug", true);
#else
    engine.rootContext()->setContextProperty("dwyco_debug", false);
#endif

    // GUI test agent. It has to be built after dwyco_register_qml() and
    // before engine.load(): setup_locations() -> settings_load() runs
    // inside dwyco_register_qml(), so by this point the settings map that
    // the startup gates read from is already loaded and can be seeded.
    // Without --test-agent none of this exists and test_mode is false.
    PhooTestAgent *agent = nullptr;
    if(test_agent && !test_socket.isEmpty()) {
        agent = new PhooTestAgent(test_socket, &engine);
        if(test_seed)
            agent->seedTestProfile();
        engine.rootContext()->setContextProperty("testTelemetry", agent->telemetry());
    }
    engine.rootContext()->setContextProperty("test_mode", agent != nullptr);

    QObject::connect(TheEngine, &QQmlEngine::quit, &app, &QGuiApplication::quit);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    int ret;
    ret = app.exec();
    start_desktop_background();

    return ret;
}
