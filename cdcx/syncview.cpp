#include "syncview.h"
#include "ui_syncview.h"

#include "cdcsync.h"
QStandardItemModel *SyncModel;

syncview::syncview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::syncview)
{
    ui->setupUi(this);
    if(!SyncModel)
        SyncModel = init_syncmodel();
    ui->tableView->setModel(SyncModel);
}

syncview::~syncview()
{
    delete ui;
}
