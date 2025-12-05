#ifndef DYNQTEXTEDIT_H
#define DYNQTEXTEDIT_H

#include <QTextEdit>
#include <QKeyEvent>

class DynQTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit DynQTextEdit(QWidget* p = nullptr) : QTextEdit(p)
    {
        setAcceptRichText(false);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setTabChangesFocus(true);

        // THIS LINE IS THE HOLY GRAIL
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        // Force layout on every change
        connect(this, &QTextEdit::textChanged, this, [=]{
            updateGeometry();
            emit geometryAdjusted();
        });
    }

    QSize sizeHint() const override
    {
        int h = document()->size().height() + 8;          // padding
        return QSize(width(), qBound(32, h, 200));         // 1–6 lines
    }

signals:
    void enterPressed();
    void geometryAdjusted();

protected:
    void keyPressEvent(QKeyEvent* e) override
    {
        if (e->key() == Qt::Key_Return && !(e->modifiers() & Qt::ShiftModifier)) {
            emit enterPressed();
            e->accept();
            return;
        }
        QTextEdit::keyPressEvent(e);
    }
};

#endif
