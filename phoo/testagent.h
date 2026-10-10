/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// testagent: an in-process GUI automation endpoint for phoo.
//
// this is only constructed when --test-agent <socket> is on the command
// line, so a normal build/run has none of this active.
//
// it exists because phoo's UI can't be tested well from the outside:
// phoo never goes idle (main.qml's service_timer polls core.service_channels()
// every 1-100ms forever) and several animations loop infinitely
// (PulseLoader, the call button blinks in SimpleChatBox.qml), so a driver
// that waits for "quiet" would wait forever. the agent answers the three
// questions a UX test actually asks:
//
//   is it responding?    -> ping: frame counter + event loop lag
//   has it settled?      -> settle: region of interest pixel diff
//   can it be clicked?   -> probe: visible/enabled/opacity/size/hit test
//
// input is injected with QWindowSystemInterface, which is the same entry
// point a physical mouse uses, so events flow through the real Qt input
// pipeline -- but without needing the window to hold focus, and without
// needing a real X server.
//
// protocol: newline delimited JSON on a QLocalServer. a request is
//   {"id": 1, "cmd": "click", "name": "phoo.convlist.trivia"}
// and the reply is
//   {"id": 1, "ok": true, ...}
//
// a bare line that isn't JSON is treated as {"cmd": <line>} so you can
// poke at a running app by hand:
//   printf 'probe phoo.convlist.trivia\n' | nc -U /tmp/phoo.sock
#ifndef PHOO_TESTAGENT_H
#define PHOO_TESTAGENT_H

#include <QElapsedTimer>
#include <QEvent>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QSet>
#include <QString>

class QLocalServer;
class QLocalSocket;
class QQuickItem;
class QQuickWindow;
class QTimer;

// the little object QML can see, so a binding or a test can ask "how many
// frames have we drawn and how janky were they" from inside the scene.
class PhooTestTelemetry : public QObject
{
    Q_OBJECT
public:
    explicit PhooTestTelemetry(QObject *parent = nullptr) : QObject(parent) {}
    Q_INVOKABLE qint64 frameCount() const { return m_frameCount; }
    Q_INVOKABLE qint64 lastFrameAgeMs() const { return m_lastFrameAgeMs; }
    Q_INVOKABLE qint64 maxLagMs() const { return m_maxLagMs; }
    // set by the agent on every frame swap
    qint64 m_frameCount = 0;
    qint64 m_lastFrameAgeMs = -1;
    qint64 m_maxLagMs = 0;
};

class PhooTestAgent : public QObject
{
    Q_OBJECT
public:
    PhooTestAgent(const QString &socket_path, QObject *parent = nullptr);
    ~PhooTestAgent() override;

    // true when the app was launched with --test-agent. QML reads this
    // through the "test_mode" context property (main.cpp) to suppress
    // infinite animations and tooltip popups.
    static bool test_mode_enabled();

    // writes the startup gates so the app boots straight to a usable
    // convlist: no first run profile dialog, no reindex page, no
    // blank_page spinner, no pin dialog.
    void seedTestProfile();

    PhooTestTelemetry *telemetry() const { return m_telemetry; }

private slots:
    void onNewConnection();
    void onReadyRead();
    void onFrameSwapped();
    void onHeartbeat();
    void onWindowPoll();

private:
    // ---- request dispatch ----
    QJsonObject dispatch(const QJsonObject &req, QLocalSocket *sock);

    // ---- object lookup ----
    // finds by objectName anywhere under the window. returns null when
    // there is no such named object.
    QObject *findObject(const QString &name) const;
    QQuickItem *findItem(const QString &name) const;
    // the frontmost item at a scene point that accepts mouse buttons,
    // or null. this is what a real click would land on.
    QQuickItem *hitTest(const QPointF &scene_pos) const;
    bool isDescendantOrSelf(QQuickItem *maybe_child, QQuickItem *ancestor) const;

    // ---- reporting ----
    QJsonObject objectReport(QObject *o) const;
    QJsonObject itemReport(QQuickItem *it) const;
    QJsonObject treeReport(QQuickItem *it, int depth, int max_depth) const;

    // ---- region of interest ----
    // resolves the "name"/"rect" fields of a request into scene coords.
    // falls back to the whole window when neither is usable.
    QRect resolveRoi(const QJsonObject &req, bool *ok) const;
    QImage grabRoi(const QRect &roi) const;
    // number of pixels that differ between two same-sized images.
    // cheap, no allocation: walks the 32bpp buffers directly.
    static qint64 imageDiff(const QImage &a, const QImage &b, int tol);

    // ---- input injection ----
    bool injectMouse(const QPointF &scene_pos, Qt::MouseButton button,
                     Qt::KeyboardModifiers mods, QEvent::Type type);
    void injectClick(const QPointF &p, Qt::MouseButton b, Qt::KeyboardModifiers m);
    void injectMove(const QPointF &p);
    void injectKey(int key, Qt::KeyboardModifiers mods, const QString &text);
    // resolves the "name"/"x"+"y" fields of a request to a scene point.
    bool resolvePoint(const QJsonObject &req, QPointF *out) const;

    // ---- lifecycle ----
    void bindWindow();
    void write(const QJsonObject &obj, QLocalSocket *sock);
    QJsonObject ping();

    QString m_socketPath;

    QLocalServer *m_server = nullptr;
    QList<QLocalSocket *> m_socks;

    QQuickWindow *m_window = nullptr;
    QTimer *m_windowPoll = nullptr;

    // telemetry
    QElapsedTimer m_clock;
    PhooTestTelemetry *m_telemetry = nullptr;

    // event loop lag, sampled by a 1ms timer. a delta much bigger than
    // the interval means the gui thread was blocked by something.
    QTimer *m_heartbeat = nullptr;
    qint64 m_lastTickMs = 0;
    QList<qint64> m_lagSamples;

    // input -> paint latency, measured from injection to the frame swap
    // that follows it.
    qint64 m_inputStampMs = -1;
    QString m_inputLabel;
    qint64 m_lastInputLatencyMs = -1;

    // things the harness asked us to remember
    QJsonArray m_events;
    qint64 m_eventSeq = 0;
};

#endif // PHOO_TESTAGENT_H