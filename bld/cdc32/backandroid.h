#ifndef BACKANDROID_H
#define BACKANDROID_H

namespace dwyco {
void android_backup();
int android_restore_msgs();
int android_days_since_last_backup();
int android_get_backup_state();
int android_set_backup_state(int i);
}

#endif // BACKANDROID_H