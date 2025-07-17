#include <QStandardItemModel>
// scaffold provided by gemini pro 2.5 ca 7/2025 from prompt:
/*
assume you have access to an api with the following c++ function calls:

int
dwyco_get_num_item_rows()

QString
dwyco_get_item(int row, const char *name)
name can be one of "UID", "IP", "PERCENT".

using qt6 widgets, create a QStandardItemModel object that uses the given api to expose each row to a view. the names mentioned above should be columns in the QStandardItem objects that are created in the model. also, create a function that refreshes the model without deleting the model, since the model will be set into a QListView.

*/
int dwyco_get_num_item_rows();
QString dwyco_get_item(int row, const char* name);

QStandardItemModel *
init_syncmodel()
{
    // Create the model with 0 rows and 3 columns
    auto *apiModel = new QStandardItemModel(0, 3, 0);

    // Set the column headers
    apiModel->setHorizontalHeaderLabels({"UID", "IP", "PERCENT"});
    return apiModel;
}

/**
 * @brief Refreshes the model with the latest data from the dwyco_ API.
 *
 * This function clears existing data rows while preserving headers and then
 * repopulates the model. This is the correct way to update a model that is
 * already set on a view.
 *
 * @param model A pointer to the QStandardItemModel to be refreshed.
 */
void refreshModel(QStandardItemModel *model) {
    if (!model) {
        return;
    }

    // Notify views that the model is about to undergo a major change.
    //model->beginResetModel();

    // Clear only the data rows, leaving the header configuration intact.
    model->removeRows(0, model->rowCount());

    // Get the total number of items from the API.
    const int numRows = dwyco_get_num_item_rows();

    // Create a QStandardItem for each column in each row.
    for (int i = 0; i < numRows; ++i) {
        // Fetch data for each column from the API
        QStandardItem *uidItem = new QStandardItem(dwyco_get_item(i, "UID"));
        QStandardItem *ipItem = new QStandardItem(dwyco_get_item(i, "IP"));
        QStandardItem *percentItem = new QStandardItem(dwyco_get_item(i, "PERCENT"));

        // Items in a row should not be editable by default.
        uidItem->setEditable(false);
        ipItem->setEditable(false);
        percentItem->setEditable(false);

        // Add the items as a new row in the model.
        model->appendRow({uidItem, ipItem, percentItem});
    }

    // Notify views that the model has been updated.
    //model->endResetModel();
}

#undef TEST
#ifdef TEST
#include <QApplication>
#include <QStandardItemModel>
#include <QTableView>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QHeaderView>

/*
 * ===================================================================================
 * Dummy API Implementation (for demonstration purposes)
 * ===================================================================================
 */
#include <QVector>

namespace {
// Use a simple counter to generate different data on each refresh
int refresh_counter = 0;
} // namespace

// Returns the number of items available.
int dwyco_get_num_item_rows() {
    return 5 + (refresh_counter % 4); // Return a varying number of rows
}

// Fetches a specific data field for a given row.
QString dwyco_get_item(int row, const char* name) {
    QString field(name);
    if (field == "UID") {
        return QString("UID-%1").arg(1000 + row);
    }
    if (field == "IP") {
        return QString("192.168.1.%1").arg(10 + row + refresh_counter);
    }
    if (field == "PERCENT") {
        return QString::number(25 + (row * 10 + refresh_counter * 5) % 75) + "%";
    }
    return QString(); // Return empty string for unknown names
}
/* =================================================================================== */




int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Main window and layout
    QWidget window;
    QVBoxLayout *layout = new QVBoxLayout(&window);

    // 1. Create the model
    //auto *apiModel = new QStandardItemModel(0, 3, &window);
    //apiModel->setHorizontalHeaderLabels({"UID", "IP", "PERCENT"});
    auto apiModel = init_syncmodel();

    // 2. Create a view to display the model data
    auto *view = new QTableView();
    view->setModel(apiModel);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 3. Create a button to trigger the refresh
    auto *refreshButton = new QPushButton("🔄 Refresh Data");

    // 4. Connect the button's clicked signal to our refresh function
    QObject::connect(refreshButton, &QPushButton::clicked, [apiModel]() {
        refreshModel(apiModel);
    });

    // Add widgets to the layout
    layout->addWidget(view);
    layout->addWidget(refreshButton);

    // Initial data population
    refreshModel(apiModel);

    window.setWindowTitle("API Data Viewer");
    window.resize(500, 350);
    window.show();

    return app.exec();
}
#endif
