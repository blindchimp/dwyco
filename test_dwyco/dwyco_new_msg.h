#ifndef dwyco_new_msg_h
#define dwyco_new_msg_h
#include "dwstr.h"
int process_remote_msgs();
void processed_msg(DwString& mid);
int dwyco_new_msg(DwString& uid_out, DwString& txt, int& zap_viewer, DwString& mid_out, int &has_att);
int dwyco_new_msg2(DwString& uid_out, DwString& txt, int& zap_viewer, DwString& mid_out, int &has_att, int& is_file, DwString& creator_uid);
#endif