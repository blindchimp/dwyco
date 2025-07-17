#ifndef CDCSYNC_H
#define CDCSYNC_H

#include <QStandardItemModel>

/**
 * @brief Initializes a QStandardItemModel for synchronization data.
 *
 * Creates a model with 0 rows and 3 columns, setting the column headers
 * to "UID", "IP", and "PERCENT".
 *
 * @return A pointer to the initialized QStandardItemModel.
 */
QStandardItemModel *init_syncmodel();

/**
 * @brief Refreshes the model with the latest data from the dwyco_ API.
 *
 * This function clears existing data rows while preserving headers and
then
 * repopulates the model. This is the correct way to update a model that
is
 * already set on a view.
 *
 * @param model A pointer to the QStandardItemModel to be refreshed.
 */
void refreshModel(QStandardItemModel *model);

#endif // CDCSYNC_H
