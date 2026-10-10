/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "testagent.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMetaMethod>
#include <QPainter>
#include <QPen>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSaveFile>
#include <QThread>
#include <QTimer>
#include <QVariant>

#include <QtGui/qpa/qwindowsysteminterface.h>

#include <algorithm>
#include <QDebug>

#include "ssmap.h"

static bool g_test_mode = false;

bool
PhooTestAgent::test_mode_enabled()
{
    return g_test_mode;
}

// ---------------------------------------------------------------------------
// frame writer (worker thread)
// ---------------------------------------------------------------------------

PhooFrameWriter::PhooFrameWriter(QString dir, QObject *parent)
    : QObject(parent), m_dir(std::move(dir))
{}

void
PhooFrameWriter::writeFrame(int seq, const QImage &img)
{
    if(img.isNull())
        return;
    // QSaveFile so a killed process can't leave a half written frame that
    // ffmpeg then chokes on.
    const QString path =
        QStringLiteral("%1/frames/%2.png").arg(m_dir).arg(seq, 6, 10, QLatin1Char('0'));
    QSaveFile f(path);
    if(!f.open(QIODevice::WriteOnly))
        return;
    if(img.save(&f, "PNG"))
        f.commit();
}

void
PhooFrameWriter::flush()
{
    emit done();
}

// Debug aid: logs every mouse event Qt actually delivers to the window,
// so "the injector sent it" can be distinguished from "qt dropped it".
class PhooInputTracer : public QObject
{
public:
    PhooInputTracer(PhooTestAgent *a, QObject *parent = nullptr)
        : QObject(parent), m_agent(a) {}

protected:
    bool eventFilter(QObject *o, QEvent *ev) override
    {
        switch(ev->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove:
        case QEvent::Enter:
        case QEvent::Leave:
        case QEvent::MouseButtonDblClick: {
            auto *me = static_cast<QMouseEvent *>(ev);
            qInfo().noquote()
                << QStringLiteral("[trace] %1 pos=(%2,%3) buttons=%4 grabber=%5")
                       .arg(QString::fromLatin1(ev->type() == QEvent::MouseMove
                                                    ? "move"
                                                    : ev->type() == QEvent::Enter
                                                          ? "enter"
                                                          : ev->type() == QEvent::Leave
                                                                ? "leave"
                                                                : ev->type() == QEvent::MouseButtonPress
                                                                      ? "press"
                                                                      : ev->type() == QEvent::MouseButtonRelease
                                                                            ? "release"
                                                                            : "dblclick"),
                       QString::number(qRound(me->position().x())),
                       QString::number(qRound(me->position().y())),
                       QString::number(int(me->buttons())),
                       m_agent && m_agent->window()
                           ? (m_agent->window()->mouseGrabberItem()
                                  ? QStringLiteral("yes")
                                  : QStringLiteral("no"))
                           : QStringLiteral("?"));
            break;
        }
        default:
            break;
        }
        return QObject::eventFilter(o, ev);
    }

private:
    PhooTestAgent *m_agent;
};

// ---------------------------------------------------------------------------
// input tracing
// ---------------------------------------------------------------------------

void
PhooTestAgent::setInputTrace(bool on)
{
    if(!m_window) {
        qWarning() << "[testagent] trace: no window bound yet";
        return;
    }
    if(on) {
        if(!m_inputTracer) {
            m_inputTracer = new PhooInputTracer(this);
            m_inputTracer->setParent(this);
            m_window->installEventFilter(m_inputTracer);
        }
        qInfo() << "[testagent] input trace ON";
    } else {
        if(m_inputTracer)
            m_window->removeEventFilter(m_inputTracer);
        qInfo() << "[testagent] input trace OFF";
    }
}

// ---------------------------------------------------------------------------

PhooTestAgent::PhooTestAgent(const QString &socket_path, QObject *parent)
    : QObject(parent), m_socketPath(socket_path)
{
    g_test_mode = true;
    m_clock.start();

    m_telemetry = new PhooTestTelemetry(this);

    QLocalServer::removeServer(m_socketPath);
    m_server = new QLocalServer(this);
    m_server->setSocketOptions(QLocalServer::UserAccessOption);
    if(!m_server->listen(m_socketPath)) {
        qWarning() << "[testagent] FAILED to listen on" << m_socketPath
                   << ":" << m_server->errorString();
        return;
    }
    connect(m_server, &QLocalServer::newConnection, this, &PhooTestAgent::onNewConnection);
    qInfo() << "[testagent] listening on" << m_socketPath;

    // 1ms heartbeat. the delta between ticks is our event loop lag: if the
    // gui thread wedges on something (a blocking core.init(), a deadlock,
    // a synchronous network call) this stops advancing, which is the
    // "is the UI still responding" signal.
    m_heartbeat = new QTimer(this);
    m_heartbeat->setTimerType(Qt::PreciseTimer);
    m_heartbeat->setInterval(1);
    connect(m_heartbeat, &QTimer::timeout, this, &PhooTestAgent::onHeartbeat);
    m_heartbeat->start();
    m_lastTickMs = m_clock.elapsed();

    // the window doesn't exist yet when this object is made (it gets
    // constructed before engine.load()), so poll for it briefly.
    m_windowPoll = new QTimer(this);
    m_windowPoll->setInterval(50);
    connect(m_windowPoll, &QTimer::timeout, this, &PhooTestAgent::onWindowPoll);
    m_windowPoll->start();
}

PhooTestAgent::~PhooTestAgent()
{
    if(m_recording)
        recordStop();
    else
        cleanupRecorder();
    if(m_server) {
        m_server->close();
        QLocalServer::removeServer(m_socketPath);
    }
}

void
PhooTestAgent::onWindowPoll()
{
    if(m_window)
        return;
    bindWindow();
}

void
PhooTestAgent::bindWindow()
{
    // idempotent: dispatch() calls this on every request, and a second
    // connect() to frameSwapped would fire onFrameSwapped N times per
    // frame and leak connections.
    if(m_window)
        return;
    for(QWindow *w : QGuiApplication::topLevelWindows()) {
        if(auto *qw = qobject_cast<QQuickWindow *>(w)) {
            m_window = qw;
            connect(m_window, &QQuickWindow::frameSwapped,
                    this, &PhooTestAgent::onFrameSwapped);
            qInfo() << "[testagent] bound window" << m_window->title()
                    << m_window->width() << "x" << m_window->height();
            m_windowPoll->stop();
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// seeding
// ---------------------------------------------------------------------------

// writes the startup gates so the app boots straight to a usable convlist
// without the first run profile dialog, the reindex page, the blank_page
// spinner or the pin dialog.
//
// this is only safe because settings_load() has already run by the time
// dwyco_register_qml() returns (dwyco_top.cpp:4660 -> setup_locations ->
// dwyco_top.cpp:1484), and main.cpp calls dwyco_register_qml() at line 121,
// before engine.load() at line 151. so the in-memory Settings map that the
// qml gates read from is already populated when we get here.
void
PhooTestAgent::seedTestProfile()
{
    struct { const char *k; const char *v; } seeds[] = {
        {"first-run", "done"},    // skips ProfileDialog (main.qml:1371-1378)
        {"reindex1", "1"},        // skips Reindex (main.qml:1380-1385)
        {"acct-created", "true"}, // skips blank_page, enables toolbars
        {"pw", ""},               // empty pin => password_ok == 1
        {"salt", ""},             // calc_pw("") == "" (PINDialog.qml:46-52)
        {"first-pin", "1"},
        {"show_unreviewed", "0"},
        {"pin_duration", "0"},    // don't re-lock in the middle of a test
    };
    for(auto &s : seeds)
        setting_put(s.k, s.v);
    qInfo() << "[testagent] seeded test profile gates";
}

// ---------------------------------------------------------------------------
// socket plumbing
// ---------------------------------------------------------------------------

void
PhooTestAgent::onNewConnection()
{
    while(QLocalSocket *s = m_server->nextPendingConnection()) {
        m_socks.append(s);
        connect(s, &QLocalSocket::readyRead, this, &PhooTestAgent::onReadyRead);
        connect(s, &QLocalSocket::disconnected, this, [this, s]() {
            m_socks.removeAll(s);
            s->deleteLater();
        });
    }
}

void
PhooTestAgent::onReadyRead()
{
    QByteArray buf;
    for(auto *s : m_socks)
        buf += s->readAll();

    while(true) {
        int nl = buf.indexOf('\n');
        if(nl < 0)
            break;
        QByteArray line = buf.left(nl);
        buf.remove(0, nl + 1);
        line = line.trimmed();
        if(line.isEmpty())
            continue;

        QLocalSocket *sock = m_socks.isEmpty() ? nullptr : m_socks.first();

        QJsonParseError err{};
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        QJsonObject req;
        if(err.error == QJsonParseError::NoError && doc.isObject()) {
            req = doc.object();
        }
        else {
            // bare line: "probe phoo.convlist.trivia" or just "ping".
            // first token is the command, the rest are positional args.
            const QStringList parts =
                QString::fromUtf8(line).split(' ', Qt::SkipEmptyParts);
            if(parts.isEmpty())
                continue;
            req["cmd"] = parts[0];
            if(parts.size() > 1) req["name"] = parts[1];
            if(parts.size() > 2) req["prop"] = parts[2];
        }

        QJsonObject resp = dispatch(req, sock);
        resp["id"] = req.value("id");
        write(resp, sock);
    }
}

void
PhooTestAgent::write(const QJsonObject &obj, QLocalSocket *sock)
{
    QByteArray b = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    b.append('\n');
    if(sock && sock->state() == QLocalSocket::ConnectedState)
        sock->write(b);
}

void
PhooTestAgent::onFrameSwapped()
{
    m_telemetry->m_frameCount++;
    m_telemetry->m_lastFrameAgeMs = m_clock.elapsed();

    // if input was injected since the last swap, this frame is the paint
    // that reflects it. that gap is the input->paint latency.
    if(m_inputStampMs >= 0) {
        m_lastInputLatencyMs = m_telemetry->m_lastFrameAgeMs - m_inputStampMs;
        m_inputStampMs = -1;

        // annotate the most recent input event with the frame that
        // actually showed the result, so a recording can mark both ends
        auto annotate = [this](QJsonArray &arr) {
            for(int i = arr.size() - 1; i >= 0; --i) {
                QJsonObject o = arr.at(i).toObject();
                if(o["event"].toString() == "input" &&
                   !o.contains("paint_frame")) {
                    o["paint_frame"] = m_recording ? m_recFrameIndex : -1;
                    o["latency_ms"] = m_lastInputLatencyMs;
                    arr.replace(i, o);
                    return true;
                }
            }
            return false;
        };
        annotate(m_events);
        if(m_recording)
            annotate(m_recEvents);
    }
}

void
PhooTestAgent::onHeartbeat()
{
    qint64 now = m_clock.elapsed();
    qint64 delta = now - m_lastTickMs;
    m_lastTickMs = now;
    m_heartbeatCount++;
    if(delta > m_telemetry->m_maxLagMs)
        m_telemetry->m_maxLagMs = delta;
    m_lagSamples.append(delta);
    if(m_lagSamples.size() > 4096)
        m_lagSamples.removeFirst();
}

// ---------------------------------------------------------------------------
// object lookup
// ---------------------------------------------------------------------------

QObject *
PhooTestAgent::findObject(const QString &name) const
{
    if(name.isEmpty() || !m_window)
        return nullptr;

    // Pass 1: the live visual tree, depth first. This is the one that
    // matters -- it is what the user can actually see and click, and it
    // gives the object that is really in the scene at its real position.
    //
    // The QObject-children fallback below is deliberately second: walking
    // it can turn up an object that shares an objectName but is not the
    // one being painted (a twin left over in the component/qml object
    // tree), which then reports a bogus position and swallows clicks.
    QList<QQuickItem *> stack;
    if(QQuickItem *root = m_window->contentItem())
        stack.append(root);
    QSet<QQuickItem *> seenItems;
    while(!stack.isEmpty()) {
        QQuickItem *it = stack.takeLast();
        if(!it || seenItems.contains(it))
            continue;
        seenItems.insert(it);
        if(it->objectName() == name)
            return it;
        const QList<QQuickItem *> kids = it->childItems();
        for(int i = kids.size() - 1; i >= 0; --i)
            stack.append(kids.at(i));
    }

    // Pass 2: popups, dialogs, menus and other plain QObjects that never
    // appear in childItems(). Breadth first, and only now.
    QList<QObject *> queue;
    queue.append(const_cast<QQuickWindow *>(m_window));
    QSet<QObject *> seen;
    while(!queue.isEmpty()) {
        QObject *o = queue.takeFirst();
        if(!o || seen.contains(o))
            continue;
        seen.insert(o);
        if(o->objectName() == name)
            return o;
        if(auto *it = qobject_cast<QQuickItem *>(o)) {
            for(QQuickItem *c : it->childItems())
                queue.append(c);
        }
        for(QObject *c : o->children())
            queue.append(c);
    }
    return nullptr;
}

QQuickItem *
PhooTestAgent::findItem(const QString &name) const
{
    return qobject_cast<QQuickItem *>(findObject(name));
}

bool
PhooTestAgent::isDescendantOrSelf(QQuickItem *maybe_child, QQuickItem *ancestor) const
{
    for(QQuickItem *i = maybe_child; i; i = i->parentItem()) {
        if(i == ancestor)
            return true;
    }
    return false;
}

// A QQuickOverlay is the container Qt parks popups in. It sits above the
// whole window at z=1000001 and reports acceptedMouseButtons == all, so a
// naive geometric walk always stops there. But Qt's own delivery ignores
// the overlay unless a popup inside it is actually showing, and matching
// that is the difference between reporting every control as occluded and
// reporting the truth. An overlay with nothing visible inside it cannot
// block anything.
static bool
overlayBlocks(const QQuickItem *overlay)
{
    const auto kids = overlay->childItems();
    for(QQuickItem *k : kids) {
        if(k->isVisible() && k->opacity() > 0.0)
            return true;
        for(QQuickItem *g : k->childItems()) {
            if(g->isVisible() && g->opacity() > 0.0)
                return true;
        }
    }
    return false;
}

// frontmost item at a scene point that would actually receive the click.
// last child == topmost in qt quick, so walk children in reverse.
QQuickItem *
PhooTestAgent::hitTest(const QPointF &scene_pos) const
{
    if(!m_window)
        return nullptr;
    QQuickItem *root = m_window->contentItem();
    if(!root)
        return nullptr;

    QList<QQuickItem *> stack;
    stack.append(root);
    QQuickItem *best = nullptr;
    while(!stack.isEmpty()) {
        QQuickItem *it = stack.takeLast();
        if(!it)
            continue;
        if(!it->isVisible() || it->opacity() <= 0.0)
            continue;
        if(!it->contains(it->mapFromScene(scene_pos)))
            continue;
        const bool is_overlay =
            qstrcmp(it->metaObject()->className(), "QQuickOverlay") == 0;
        if(is_overlay) {
            if(!overlayBlocks(it))
                continue;   // no popup showing: not a blocker
            best = it;
        } else if(it->acceptedMouseButtons() != Qt::NoButton) {
            best = it;
        }
        const QList<QQuickItem *> kids = it->childItems();
        for(int i = kids.size() - 1; i >= 0; --i)
            stack.append(kids.at(i));
    }
    return best;
}

// ---------------------------------------------------------------------------
// reporting
// ---------------------------------------------------------------------------

QJsonObject
PhooTestAgent::itemReport(QQuickItem *it) const
{
    QJsonObject o;
    o["name"] = it->objectName();
    o["type"] = QString::fromLatin1(it->metaObject()->className());
    o["x"] = it->x();
    o["y"] = it->y();
    o["width"] = it->width();
    o["height"] = it->height();
    o["visible"] = it->isVisible();
    o["opacity"] = it->opacity();
    o["enabled"] = it->isEnabled();
    o["z"] = it->z();
    o["focused"] = it->hasActiveFocus();
    o["acceptedMouseButtons"] = int(it->acceptedMouseButtons());
    const QRectF r = it->mapRectToScene(QRectF(0, 0, it->width(), it->height()));
    o["scene_x"] = r.x();
    o["scene_y"] = r.y();
    o["scene_w"] = r.width();
    o["scene_h"] = r.height();
    return o;
}

QJsonObject
PhooTestAgent::objectReport(QObject *ob) const
{
    if(auto *it = qobject_cast<QQuickItem *>(ob))
        return itemReport(it);
    QJsonObject o;
    o["name"] = ob->objectName();
    o["type"] = QString::fromLatin1(ob->metaObject()->className());
    o["visible"] = ob->property("visible").toBool();
    o["enabled"] = ob->property("enabled").toBool();
    o["opacity"] = ob->property("opacity").toDouble();
    return o;
}

QJsonObject
PhooTestAgent::treeReport(QQuickItem *it, int depth, int max_depth) const
{
    QJsonObject o = itemReport(it);
    if(depth >= max_depth)
        return o;
    QJsonArray kids;
    for(QQuickItem *c : it->childItems())
        kids.append(treeReport(c, depth + 1, max_depth));
    o["children"] = kids;
    return o;
}

// ---------------------------------------------------------------------------
// region of interest
// ---------------------------------------------------------------------------

QRect
PhooTestAgent::resolveRoi(const QJsonObject &req, bool *ok) const
{
    if(ok) *ok = true;
    if(!m_window) {
        if(ok) *ok = false;
        return QRect();
    }

    // explicit rect wins: {"rect":[x,y,w,h]} in scene coords
    if(req.contains("rect")) {
        const QJsonArray r = req["rect"].toArray();
        if(r.size() == 4)
            return QRect(r[0].toInt(), r[1].toInt(), r[2].toInt(), r[3].toInt());
    }
    if(req.contains("x") && req.contains("y") &&
       req.contains("w") && req.contains("h")) {
        return QRect(req["x"].toInt(), req["y"].toInt(),
                     req["w"].toInt(), req["h"].toInt());
    }

    const QString name = req["name"].toString();
    if(!name.isEmpty()) {
        if(QQuickItem *it = findItem(name)) {
            // round outward so we never clip a pixel
            return it->mapRectToScene(QRectF(0, 0, it->width(), it->height()))
                      .toAlignedRect();
        }
        if(ok) *ok = false;
        return QRect();
    }

    return QRect(0, 0, m_window->width(), m_window->height());
}

QImage
PhooTestAgent::grabRoi(const QRect &roi) const
{
    if(!m_window)
        return QImage();
    QImage full = m_window->grabWindow();
    if(full.isNull())
        return full;
    // normalise so imageDiff can walk the buffers directly
    if(full.format() != QImage::Format_ARGB32)
        full = full.convertToFormat(QImage::Format_ARGB32);
    if(roi.isNull() || roi == full.rect())
        return full;
    return full.copy(roi.intersected(full.rect()));
}

qint64
PhooTestAgent::imageDiff(const QImage &a, const QImage &b, int tol)
{
    if(a.isNull() || b.isNull() || a.size() != b.size())
        return -1;

    const int w = a.width(), h = a.height();
    qint64 diff = 0;
    for(int y = 0; y < h; ++y) {
        const QRgb *pa = reinterpret_cast<const QRgb *>(a.constScanLine(y));
        const QRgb *pb = reinterpret_cast<const QRgb *>(b.constScanLine(y));
        for(int x = 0; x < w; ++x) {
            if(qAbs(int(pa[x] & 0xff) - int(pb[x] & 0xff)) > tol ||
               qAbs(int((pa[x] >> 8) & 0xff) - int((pb[x] >> 8) & 0xff)) > tol ||
               qAbs(int((pa[x] >> 16) & 0xff) - int((pb[x] >> 16) & 0xff)) > tol) {
                ++diff;
            }
        }
    }
    return diff;
}

// ---------------------------------------------------------------------------
// input injection
// ---------------------------------------------------------------------------

bool
PhooTestAgent::resolvePoint(const QJsonObject &req, QPointF *out) const
{
    if(req.contains("x") && req.contains("y")) {
        *out = QPointF(req["x"].toDouble(), req["y"].toDouble());
        return true;
    }
    const QString name = req["name"].toString();
    if(name.isEmpty())
        return false;
    if(QQuickItem *it = findItem(name)) {
        *out = it->mapToScene(QPointF(it->width() / 2.0, it->height() / 2.0));
        return true;
    }
    return false;
}

bool
PhooTestAgent::injectMouse(const QPointF &p, Qt::MouseButton button,
                           Qt::KeyboardModifiers mods, QEvent::Type type)
{
    if(!m_window)
        return false;
    const QPointF global = m_window->mapToGlobal(p.toPoint());

    // the event's button state has to be the set of buttons still held
    // *after* this event, which is what the platform plugin would report.
    // handing back the released button on a release event makes Qt drop
    // the delivery: the press lands on the button but the release never
    // does, so onReleased and onClicked never fire.
    Qt::MouseButtons state;
    switch(type) {
    case QEvent::MouseButtonPress:
        m_buttons |= button;
        state = m_buttons;
        break;
    case QEvent::MouseButtonRelease:
        m_buttons &= ~button;
        state = m_buttons;
        break;
    default:
        state = m_buttons;
        break;
    }

    QWindowSystemInterface::handleMouseEvent<QWindowSystemInterface::SynchronousDelivery>(
        m_window, p, global, state, button, type, mods);
    return true;
}

void
PhooTestAgent::injectMove(const QPointF &p)
{
    injectMouse(p, Qt::NoButton, Qt::NoModifier, QEvent::MouseMove);
}

void
PhooTestAgent::injectClick(const QPointF &p, Qt::MouseButton b, Qt::KeyboardModifiers m)
{
    // Qt only delivers a press to an item that is already in the window's
    // hover chain, so a move has to land at the point *before* the press.
    // Without this the press goes nowhere on windows that have never seen
    // a real pointer (i.e. always, for in-process injection).
    //
    // Deliberately NO move between the press and the release. A button's
    // press handling goes through a mouse grabber, and nudging the point
    // by a pixel mid-click is enough to make Qt Quick drop the release --
    // the button sees onPressed and then never sees onReleased, so
    // onClicked never fires. A real mouse doesn't jitter mid-click either.
    injectMove(p);
    injectMouse(p, b, m, QEvent::MouseButtonPress);
    injectMouse(p, b, m, QEvent::MouseButtonRelease);
}

void
PhooTestAgent::injectKey(int key, Qt::KeyboardModifiers mods, const QString &text)
{
    if(!m_window)
        return;
    QWindowSystemInterface::handleKeyEvent<QWindowSystemInterface::SynchronousDelivery>(
        m_window, QEvent::KeyPress, key, mods, text);
    QWindowSystemInterface::handleKeyEvent<QWindowSystemInterface::SynchronousDelivery>(
        m_window, QEvent::KeyRelease, key, mods, text);
}

// ---------------------------------------------------------------------------
// ping
// ---------------------------------------------------------------------------

QJsonObject
PhooTestAgent::ping()
{
    bindWindow();
    QJsonObject o;
    o["ok"] = true;
    o["uptime_ms"] = m_clock.elapsed();
    o["frame_count"] = m_telemetry->m_frameCount;
    // frame_count only moves when the scene actually repaints, so on a
    // static screen it stays at 0 even though the app is perfectly
    // healthy. heartbeat_count is the rendering-independent liveness
    // signal: it advances on every event loop turn unless the gui thread
    // is blocked.
    o["heartbeat_count"] = m_heartbeatCount;
    o["last_frame_age_ms"] = m_telemetry->m_lastFrameAgeMs;
    o["eventloop_max_lag_ms"] = m_telemetry->m_maxLagMs;

    if(!m_lagSamples.isEmpty()) {
        QList<qint64> s = m_lagSamples;
        std::sort(s.begin(), s.end());
        o["eventloop_p99_lag_ms"] = s[s.size() * 99 / 100];
        o["eventloop_median_lag_ms"] = s[s.size() / 2];
    }
    o["window_bound"] = m_window != nullptr;
    if(m_window) {
        o["window_width"] = m_window->width();
        o["window_height"] = m_window->height();
        o["window_exposed"] = m_window->isExposed();
        o["window_title"] = m_window->title();
    }
    o["last_input_latency_ms"] = m_lastInputLatencyMs;
    o["client_count"] = m_socks.size();
    return o;
}

// ---------------------------------------------------------------------------
// recording
// ---------------------------------------------------------------------------

namespace {
// how many frames may be waiting on the writer before we start dropping.
// grabWindow() stalls the gui thread, so if encoding can't keep up the
// right answer is to lose frames and say so, not to slow the app down
// until the test times out.
constexpr int kMaxPendingFrames = 8;
// the click ring fades out over this long, so the moment of the click is
// visible when scrubbing the video
constexpr qint64 kCursorFlashMs = 400;
}

QJsonObject
PhooTestAgent::recordCmd(const QJsonObject &req)
{
    const QString sub = req["sub"].toString();
    if(sub == "start")
        return recordStart(req);
    if(sub == "stop")
        return recordStop();
    if(sub == "status") {
        QJsonObject o;
        o["ok"] = true;
        o["recording"] = m_recording || m_stopping;
        o["stopping"] = m_stopping;
        o["frames"] = m_recFrameIndex;
        o["dropped"] = qint64(m_recDropped);
        o["dir"] = m_recDir;
        o["elapsed_ms"] = m_recording ? (m_clock.elapsed() - m_recStartMs) : 0;
        o["pending"] = int(m_recPending.load());
        return o;
    }
    QJsonObject o;
    o["ok"] = false;
    o["error"] = "record needs sub = start|stop|status";
    return o;
}

QJsonObject
PhooTestAgent::recordStart(const QJsonObject &req)
{
    QJsonObject o;
    if(m_recording || m_stopping) {
        o["ok"] = false;
        o["error"] = m_stopping ? "a stop is still in progress"
                                : "already recording";
        return o;
    }
    if(!m_window) {
        o["ok"] = false;
        o["error"] = "no window bound yet";
        return o;
    }

    const QString dir = req["path"].toString();
    if(dir.isEmpty()) {
        o["ok"] = false;
        o["error"] = "record start needs a path";
        return o;
    }

    const int fps = qBound(1, req["fps"].toInt(15), 120);
    m_recMaxMs = req["max_ms"].toInt(15000);
    m_recCursor = req.value("cursor").toBool(true);

    if(!QDir().mkpath(dir + "/frames")) {
        o["ok"] = false;
        o["error"] = "cannot create " + dir + "/frames";
        return o;
    }

    m_recDir = dir;
    m_recFrameIndex = 0;
    m_recDropped = 0;
    m_recFrameLog = QJsonArray();
    m_recEvents = QJsonArray();
    m_recStartMs = m_clock.elapsed();
    m_recPending.store(0);

    m_recWriter = new PhooFrameWriter(m_recDir);
    m_recThread = new QThread(this);
    m_recWriter->moveToThread(m_recThread);
    connect(m_recThread, &QThread::finished, m_recWriter, &QObject::deleteLater);
    m_recThread->start();

    m_recTimer = new QTimer(this);
    m_recTimer->setTimerType(Qt::PreciseTimer);
    m_recTimer->setInterval(1000 / fps);
    connect(m_recTimer, &QTimer::timeout, this, &PhooTestAgent::onRecordTick);
    m_recTimer->start();
    m_recording = true;

    qInfo() << "[testagent] recording to" << m_recDir
            << "at" << fps << "fps for" << m_recMaxMs << "ms";

    o["ok"] = true;
    o["path"] = m_recDir;
    o["fps"] = fps;
    o["max_ms"] = m_recMaxMs;
    o["width"] = m_window->width();
    o["height"] = m_window->height();
    return o;
}

bool
PhooTestAgent::captureFrame()
{
    if(!m_window)
        return false;
    const QRect roi(0, 0, m_window->width(), m_window->height());
    QImage img = grabRoi(roi);
    if(img.isNull())
        return false;

    if(m_recCursor && m_lastInputValid) {
        paintCursor(img, m_lastInputPos, m_clock.elapsed() - m_lastInputPosMs);
    }

    // bound the queue
    if(m_recPending.load() >= kMaxPendingFrames) {
        m_recDropped++;
        return false;
    }

    m_recPending.fetch_add(1);
    // the worker decrements after writing; do it via a lambda on the
    // writer so we don't need a second signal just for the counter
    QImage copy = img;
    QMetaObject::invokeMethod(
        m_recWriter, [this, seq = m_recFrameIndex, copy]() {
            m_recWriter->writeFrame(seq, copy);
            m_recPending.fetch_sub(1);
        }, Qt::QueuedConnection);

    m_recFrameLog.append(QJsonArray{m_recFrameIndex, m_clock.elapsed() - m_recStartMs,
                                    qint64(m_recDropped)});
    m_recFrameIndex++;
    return true;
}

void
PhooTestAgent::onRecordTick()
{
    if(!m_recording || m_stopping)
        return;
    captureFrame();
    if(m_recMaxMs > 0 && m_clock.elapsed() - m_recStartMs >= m_recMaxMs)
        recordStop(); // auto-stop; the harness's "record stop" is then a no-op-ish read
}

void
PhooTestAgent::paintCursor(QImage &img, const QPointF &pos, qint64 ms_since_click)
{
    if(img.format() != QImage::Format_ARGB32)
        img = img.convertToFormat(QImage::Format_ARGB32);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing, true);

    const qreal x = pos.x();
    const qreal y = pos.y();

    // persistent thin crosshair, so you can always see where the last
    // injected input landed
    p.setPen(QPen(QColor(0, 255, 255, 220), 1));
    p.drawLine(QPointF(x - 14, y), QPointF(x + 14, y));
    p.drawLine(QPointF(x, y - 14), QPointF(x, y + 14));
    p.setPen(QPen(QColor(0, 255, 255, 255), 1));
    p.drawEllipse(QPointF(x, y), 7, 7);

    // transient filled ring right after a click
    if(ms_since_click >= 0 && ms_since_click < kCursorFlashMs) {
        const qreal t = qreal(ms_since_click) / qreal(kCursorFlashMs);
        const int alpha = int(230 * (1.0 - t));
        p.setPen(QPen(QColor(255, 80, 80, alpha), 3));
        p.setBrush(QColor(255, 60, 60, alpha / 3));
        p.drawEllipse(QPointF(x, y), 18, 18);
    }
    p.end();
}

void
PhooTestAgent::noteInputEvent(const QString &cmd, const QString &name,
                              const QPointF &p)
{
    m_lastInputPos = p;
    m_lastInputValid = true;
    m_lastInputPosMs = m_clock.elapsed();

    QJsonObject ev;
    ev["seq"] = m_eventSeq++;
    ev["t"] = m_clock.elapsed();
    ev["event"] = "input";
    ev["cmd"] = cmd;
    ev["name"] = name;
    ev["x"] = p.x();
    ev["y"] = p.y();
    ev["frame"] = m_recording ? m_recFrameIndex : -1;
    m_events.append(ev);

    if(m_recording)
        m_recEvents.append(ev);
}

QJsonObject
PhooTestAgent::recordStop()
{
    QJsonObject o;
    if(!m_recording) {
        o["ok"] = true;
        o["frames"] = 0;
        o["note"] = "was not recording";
        return o;
    }

    // A stop is not finished when the flag flips: the writer queue still
    // has to drain and meta.json still has to be written. Keep reporting
    // "recording" until that is all done, or a harness that waits for the
    // stop races ahead of the files it came for.
    m_stopping = true;

    if(m_recTimer) {
        m_recTimer->stop();
        m_recTimer->deleteLater();
        m_recTimer = nullptr;
    }

    // Drain: flush() is queued behind every writeFrame(), so once done()
    // fires the whole queue is on disk.
    //
    // done() MUST be a QueuedConnection here. The writer lives on the
    // writer thread, so a direct connection would run the lambda *there*
    // and mutate `flushed` from another thread while the gui thread spins
    // on it -- a data race that means the loop always runs to its deadline
    // and meta.json never gets written.
    bool flushed = false;
    if(m_recThread && m_recWriter) {
        connect(m_recWriter, &PhooFrameWriter::done, this,
                [&flushed]() { flushed = true; }, Qt::QueuedConnection);
        QMetaObject::invokeMethod(m_recWriter, "flush", Qt::QueuedConnection);
        // bounded so a wedged writer can't hang the harness
        const qint64 deadline = m_clock.elapsed() + 5000;
        while(!flushed && m_clock.elapsed() < deadline) {
            QEventLoop loop;
            QTimer::singleShot(5, &loop, &QEventLoop::quit);
            loop.exec();
        }
        m_recThread->quit();
        m_recThread->wait(2000);
        m_recWriter = nullptr;
        m_recThread = nullptr;
    }

    const qint64 duration = m_clock.elapsed() - m_recStartMs;

    // An input with no paint_frame never caused a repaint: the click
    // landed on something inert. That is a real and useful distinction
    // (the ui did not respond because there was nothing to change), so
    // make it explicit rather than leaving it as a missing key.
    for(int i = 0; i < m_recEvents.size(); ++i) {
        QJsonObject e = m_recEvents.at(i).toObject();
        if(e["event"].toString() != "input")
            continue;
        const bool painted = e.contains("paint_frame") &&
                             e["paint_frame"].toInt(-1) >= 0;
        e["painted"] = painted;
        if(!painted) {
            e["paint_frame"] = -1;
            e["latency_ms"] = -1;
        }
        m_recEvents.replace(i, e);
    }

    QJsonObject meta;
    meta["frames"] = m_recFrameIndex;
    meta["dropped"] = qint64(m_recDropped);
    meta["duration_ms"] = duration;
    meta["width"] = m_window ? m_window->width() : 0;
    meta["height"] = m_window ? m_window->height() : 0;
    meta["frame_log"] = m_recFrameLog;
    meta["events"] = m_recEvents;
    meta["flushed"] = flushed;

    // atomic write so a reader never sees half a meta.json
    QSaveFile f(m_recDir + "/meta.json");
    if(f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(meta).toJson(QJsonDocument::Indented));
        f.commit();
    }

    cleanupRecorder();

    o["ok"] = true;
    o["frames"] = meta["frames"];
    o["dropped"] = meta["dropped"];
    o["duration_ms"] = duration;
    o["dir"] = m_recDir;
    o["meta"] = m_recDir + "/meta.json";
    o["flushed"] = flushed;
    o["events"] = m_recEvents;
    return o;
}

void
PhooTestAgent::cleanupRecorder()
{
    m_recording = false;
    m_stopping = false;
    if(m_recTimer) {
        m_recTimer->stop();
        m_recTimer->deleteLater();
        m_recTimer = nullptr;
    }
    if(m_recThread) {
        m_recThread->quit();
        m_recThread->wait(2000);
        m_recThread->deleteLater();
        m_recThread = nullptr;
    }
    m_recWriter = nullptr;
    m_recFrameLog = QJsonArray();
    m_recEvents = QJsonArray();
    m_recDir.clear();
}

// ---------------------------------------------------------------------------
// dispatch
// ---------------------------------------------------------------------------

QJsonObject
PhooTestAgent::dispatch(const QJsonObject &req, QLocalSocket *sock)
{
    Q_UNUSED(sock);
    const QString cmd = req["cmd"].toString();

    QJsonObject out;
    bindWindow();

    if(cmd == "ping")
        return ping();

    if(cmd == "quit") {
        out["ok"] = true;
        QTimer::singleShot(0, qApp, &QCoreApplication::quit);
        return out;
    }

    if(cmd == "events") {
        out["ok"] = true;
        out["events"] = m_events;
        m_events = QJsonArray();
        m_eventSeq = 0;
        return out;
    }

    if(cmd == "mark") {
        QJsonObject ev;
        ev["seq"] = m_eventSeq++;
        ev["t"] = m_clock.elapsed();
        ev["event"] = "mark";
        ev["label"] = req["label"].toString();
        m_events.append(ev);
        if(m_recording)
            m_recEvents.append(ev);
        out["ok"] = true;
        return out;
    }

    if(cmd == "record")
        return recordCmd(req);

    if(cmd == "trace_input") {
        setInputTrace(req["on"].toBool(true));
        out["ok"] = true;
        return out;
    }

    // Commands that need a window. Anything not in this list is rejected
    // first so a typo reports "unknown cmd" rather than "no window yet".
    static const QStringList need_window = {
        "find", "rect", "probe", "tree", "get", "set", "call", "grab",
        "settle", "click", "press", "release", "hover", "move", "drag",
        "presshold", "type", "key"
    };
    if(!need_window.contains(cmd)) {
        out["ok"] = false;
        out["error"] = "unknown cmd: " + cmd;
        return out;
    }

    if(!m_window) {
        out["ok"] = false;
        out["error"] = "no window bound yet";
        return out;
    }

    // ---- lookup / reporting ----
    if(cmd == "find") {
        QObject *o = findObject(req["name"].toString());
        out["ok"] = o != nullptr;
        if(o)
            out["object"] = objectReport(o);
        return out;
    }

    if(cmd == "rect") {
        QQuickItem *it = findItem(req["name"].toString());
        if(!it) {
            out["ok"] = false;
            out["error"] = "not found: " + req["name"].toString();
            return out;
        }
        out["ok"] = true;
        out["rect"] = itemReport(it);
        return out;
    }

    if(cmd == "probe") {
        const QString name = req["name"].toString();
        QObject *obj = findObject(name);
        if(!obj) {
            out["ok"] = false;
            out["error"] = "not found: " + name;
            return out;
        }
        out["ok"] = true;
        out["exists"] = true;

        // the target may not be a QQuickItem at all: a Dialog, a Menu, or
        // the QQuickWindow itself. Report what we actually know rather
        // than failing, and don't invent a geometry we don't have.
        QQuickItem *it = qobject_cast<QQuickItem *>(obj);
        const bool is_window = (obj == m_window);
        QRectF r;
        if(it)
            r = it->mapRectToScene(QRectF(0, 0, it->width(), it->height()));
        else if(is_window)
            r = QRectF(0, 0, m_window->width(), m_window->height());
        const QPointF center = r.center();

        QQuickItem *hit = r.isNull() ? nullptr : hitTest(center);
        const bool hit_ok = hit && it && isDescendantOrSelf(hit, it);

        bool ancestors_visible = true;
        for(QQuickItem *i = it; i; i = i->parentItem()) {
            if(!i->isVisible()) { ancestors_visible = false; break; }
        }
        const bool in_scene =
            r.intersects(QRectF(0, 0, m_window->width(), m_window->height()));

        const bool visible = obj->property("visible").toBool();
        const bool enabled = obj->property("enabled").toBool();
        const double opacity = obj->property("opacity").toDouble();
        const double w = it ? it->width()
                            : (is_window ? m_window->width() : 0);
        const double h = it ? it->height()
                            : (is_window ? m_window->height() : 0);

        out["is_item"] = it != nullptr;
        out["visible"] = visible;
        out["enabled"] = enabled;
        out["opacity"] = opacity;
        out["width"] = w;
        out["height"] = h;
        out["ancestors_visible"] = ancestors_visible;
        out["in_scene"] = in_scene;
        out["hit_testable"] = hit_ok;
        out["hit_name"] = hit ? hit->objectName() : QString();
        out["hit_type"] = hit
                              ? QString::fromLatin1(hit->metaObject()->className())
                              : QString();
        out["scene_rect"] = QJsonArray{r.x(), r.y(), r.width(), r.height()};
        out["center"] = QJsonArray{center.x(), center.y()};
        out["interactable"] = visible && enabled && opacity > 0.0 &&
                              w > 0 && h > 0 && ancestors_visible &&
                              in_scene && hit_ok;
        return out;
    }

    if(cmd == "tree") {
        QQuickItem *root = m_window->contentItem();
        const QString nm = req["name"].toString();
        if(!nm.isEmpty()) {
            if(QQuickItem *it = findItem(nm))
                root = it;
        }
        out["ok"] = true;
        out["tree"] = root ? treeReport(root, 0, req["depth"].toInt(3)) : QJsonObject();
        return out;
    }

    // ---- properties ----
    if(cmd == "get" || cmd == "set") {
        QObject *o = findObject(req["name"].toString());
        if(!o) {
            out["ok"] = false;
            out["error"] = "not found: " + req["name"].toString();
            return out;
        }
        const QByteArray prop = req["prop"].toString().toUtf8();
        if(cmd == "get") {
            const QVariant v = o->property(prop.constData());
            out["ok"] = true;
            out["value"] = QJsonValue::fromVariant(v);
            return out;
        }
        const bool good = o->setProperty(prop.constData(), req["value"].toVariant());
        out["ok"] = good;
        if(!good)
            out["error"] = "setProperty failed";
        return out;
    }

    if(cmd == "call") {
        QObject *o = findObject(req["name"].toString());
        if(!o) {
            out["ok"] = false;
            out["error"] = "not found: " + req["name"].toString();
            return out;
        }
        const QString method = req["method"].toString();
        const QJsonArray args = req["args"].toArray();

        // Walk the metaobject and invoke with the declared parameter types.
        // Q_ARG(type,...) can't be used here because we don't know the
        // arity or types at compile time; building QGenericArgument by hand
        // off the QVariant's storage is the way to do it generically. The
        // variants have to outlive the invoke, hence the keepalive list.
        bool done = false;
        QVariant keepalive[8];
        for(int i = 0; i < o->metaObject()->methodCount() && !done; ++i) {
            const QMetaMethod mm = o->metaObject()->method(i);
            if(mm.name() != method)
                continue;

            QGenericArgument ga[8];
            const int np = mm.parameterCount();
            if(np > 8 || args.size() < np) {
                out["ok"] = false;
                out["error"] = QStringLiteral("arity mismatch for %1 "
                                              "(method wants %2, got %3)")
                                  .arg(method).arg(np).arg(args.size());
                return out;
            }
            for(int a = 0; a < np; ++a) {
                const int tid = mm.parameterType(a);
                keepalive[a] = args.at(a).toVariant();
                if(keepalive[a].userType() != tid)
                    keepalive[a].convert(QMetaType(tid));
                ga[a] = QGenericArgument(mm.parameterTypeName(a).constData(),
                                         keepalive[a].constData());
            }

            // return value storage, when the method actually returns one
            QVariant retval;
            const QMetaType rt = mm.returnMetaType();
            if(rt.isValid()) {
                retval = QVariant(rt);
                QGenericReturnArgument rarg(rt.name(), retval.data());
                done = mm.invoke(o, Qt::DirectConnection, rarg, ga[0], ga[1],
                                 ga[2], ga[3], ga[4], ga[5], ga[6], ga[7]);
            } else {
                done = mm.invoke(o, Qt::DirectConnection,
                                 QGenericReturnArgument(), ga[0], ga[1],
                                 ga[2], ga[3], ga[4], ga[5], ga[6], ga[7]);
            }
            if(done && rt.isValid())
                out["return"] = QJsonValue::fromVariant(retval);
            break;
        }
        if(!done) {
            // functions declared inside qml aren't in the metaobject, so
            // they can't be reached from here. drive the ui with clicks
            // instead, or set the property that the function would read.
            out["ok"] = false;
            out["error"] = "no invokable named " + method +
                           " (qml-declared functions aren't reachable; "
                           "use get/set or click)";
            return out;
        }
        out["ok"] = true;
        return out;
    }

    // ---- pixels ----
    if(cmd == "grab") {
        bool ok = false;
        const QRect roi = resolveRoi(req, &ok);
        if(!ok) {
            out["ok"] = false;
            out["error"] = "cannot resolve region";
            return out;
        }
        const QImage img = grabRoi(roi);
        if(img.isNull()) {
            out["ok"] = false;
            out["error"] = "grabWindow returned null";
            return out;
        }
        out["ok"] = true;
        out["width"] = img.width();
        out["height"] = img.height();
        QByteArray bytes;
        QBuffer buf(&bytes);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "png");
        out["data_b64"] = QString::fromLatin1(bytes.toBase64());
        return out;
    }

    if(cmd == "settle") {
        // the core "has it stopped moving" primitive. grab the region every
        // interval and look for consec consecutive identical samples,
        // bounded by max_ms.
        bool ok = false;
        const QRect roi = resolveRoi(req, &ok);
        if(!ok) {
            out["ok"] = false;
            out["error"] = "cannot resolve region";
            return out;
        }
        const int interval = req["interval_ms"].toInt(100);
        const int consec_needed = req["consec"].toInt(3);
        const int max_ms = req["max_ms"].toInt(3000);
        const int tol = req["tol"].toInt(0);

        const qint64 start = m_clock.elapsed();
        QImage prev;
        int consec = 0;
        qint64 max_diff = 0;
        int samples = 0;
        bool settled = false;

        // run a nested event loop so the gui thread keeps rendering while we
        // wait. without this we'd deadlock trying to grab a window that
        // isn't being drawn.
        while(m_clock.elapsed() - start < max_ms) {
            QEventLoop loop;
            QTimer::singleShot(interval, &loop, &QEventLoop::quit);
            loop.exec();

            const QImage cur = grabRoi(roi);
            ++samples;
            if(cur.isNull()) {
                consec = 0;
                continue;
            }
            if(!prev.isNull() && prev.size() == cur.size()) {
                const qint64 d = imageDiff(prev, cur, tol);
                if(d == 0) {
                    ++consec;
                } else {
                    consec = 0;
                    if(d > max_diff)
                        max_diff = d;
                }
            }
            prev = cur;
            if(consec >= consec_needed) {
                settled = true;
                break;
            }
        }

        out["ok"] = true;
        out["settled"] = settled;
        out["elapsed_ms"] = m_clock.elapsed() - start;
        out["samples"] = samples;
        out["max_diff_pixels"] = max_diff;
        out["frame_count"] = m_telemetry->m_frameCount;
        out["eventloop_max_lag_ms"] = m_telemetry->m_maxLagMs;
        return out;
    }

    // ---- input ----
    if(cmd == "click" || cmd == "press" || cmd == "release" ||
       cmd == "hover" || cmd == "move" || cmd == "drag") {
        QPointF p;
        if(!resolvePoint(req, &p)) {
            out["ok"] = false;
            out["error"] = "cannot resolve point";
            return out;
        }
        Qt::KeyboardModifiers mods = Qt::NoModifier;
        if(req["shift"].toBool()) mods |= Qt::ShiftModifier;
        if(req["ctrl"].toBool())  mods |= Qt::ControlModifier;
        if(req["alt"].toBool())   mods |= Qt::AltModifier;

        noteInputEvent(cmd, req["name"].toString(), p);
        m_inputStampMs = m_clock.elapsed();
        m_inputLabel = cmd + ":" + req["name"].toString();

        if(cmd == "click") {
            injectClick(p, Qt::LeftButton, mods);
        } else if(cmd == "press") {
            injectMouse(p, Qt::LeftButton, mods, QEvent::MouseButtonPress);
        } else if(cmd == "release") {
            injectMouse(p, Qt::LeftButton, mods, QEvent::MouseButtonRelease);
        } else if(cmd == "hover" || cmd == "move") {
            injectMove(p);
        } else { // drag
            QPointF to;
            QJsonObject r2 = req;
            r2.remove("name");
            r2["x"] = req["tox"];
            r2["y"] = req["toy"];
            resolvePoint(r2, &to);
            injectMouse(p, Qt::LeftButton, mods, QEvent::MouseButtonPress);
            for(int i = 1; i <= 10; ++i) {
                injectMouse(p + (to - p) * (i / 10.0), Qt::LeftButton, mods,
                            QEvent::MouseMove);
            }
            injectMouse(to, Qt::LeftButton, mods, QEvent::MouseButtonRelease);
        }
        out["ok"] = true;
        out["point"] = QJsonArray{p.x(), p.y()};
        out["t"] = m_clock.elapsed();
        return out;
    }

    if(cmd == "presshold") {
        QPointF p;
        if(!resolvePoint(req, &p)) {
            out["ok"] = false;
            out["error"] = "cannot resolve point";
            return out;
        }
        const int ms = req["ms"].toInt(800);
        QPointF p2;
        resolvePoint(req, &p2);
        noteInputEvent(cmd, req["name"].toString(), p2);
        m_inputStampMs = m_clock.elapsed();
        m_inputLabel = "presshold:" + req["name"].toString();
        injectMouse(p, Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonPress);
        QEventLoop loop;
        QTimer::singleShot(ms, &loop, &QEventLoop::quit);
        loop.exec();
        injectMouse(p, Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonRelease);
        out["ok"] = true;
        out["t"] = m_clock.elapsed();
        return out;
    }

    if(cmd == "type") {
        const QString nm = req["name"].toString();
        if(!nm.isEmpty()) {
            if(QQuickItem *it = findItem(nm))
                it->forceActiveFocus();
        }
        const QString text = req["text"].toString();
        m_inputStampMs = m_clock.elapsed();
        m_inputLabel = "type:" + nm;
        for(QChar c : text) {
            if(c == '\n') {
                injectKey(Qt::Key_Return, Qt::NoModifier, QStringLiteral("\r"));
            } else {
                injectKey(c.toUpper().unicode(), Qt::NoModifier, QString(c));
            }
            // let each key event be processed before the next one
            QEventLoop loop;
            QTimer::singleShot(1, &loop, &QEventLoop::quit);
            loop.exec();
        }
        out["ok"] = true;
        out["t"] = m_clock.elapsed();
        return out;
    }

    if(cmd == "key") {
        const int k = req["keycode"].toInt();
        Qt::KeyboardModifiers mods = Qt::NoModifier;
        if(req["shift"].toBool()) mods |= Qt::ShiftModifier;
        if(req["ctrl"].toBool())  mods |= Qt::ControlModifier;
        m_inputStampMs = m_clock.elapsed();
        m_inputLabel = "key:" + QString::number(k);
        injectKey(k, mods, req["text"].toString());
        out["ok"] = true;
        out["t"] = m_clock.elapsed();
        return out;
    }

    out["ok"] = false;
    out["error"] = "unknown cmd: " + cmd;
    return out;
}