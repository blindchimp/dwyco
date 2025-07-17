#ifndef SYNCVIEW_H
#define SYNCVIEW_H

#include <QWidget>

namespace Ui {
class syncview;
}

class syncview : public QWidget
{
    Q_OBJECT

public:
    explicit syncview(QWidget *parent = nullptr);
    ~syncview();

private:
    Ui::syncview *ui;
};

#endif // SYNCVIEW_H
