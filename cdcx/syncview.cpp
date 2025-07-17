#include "syncview.h"
#include "ui_syncview.h"

#include "cdcsync.h"
static QStandardItemModel *model;

syncview::syncview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::syncview)
{
    ui->setupUi(this);
    if(!model)
        model = init_syncmodel();
    ui->listView->setModel(model);
}

syncview::~syncview()
{
    delete ui;
}
