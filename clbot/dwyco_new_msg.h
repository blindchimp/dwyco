
// ===
// The contents of this file are Copyright (C) 1995-2010 Dwyco, Inc.
// This file contains proprietary and confidential
// information that is the property of Dwyco, Inc.
// Use or disclosure of this file is 
// strictly prohibited without express written permission
// from Dwyco, Inc.
//
#ifndef dwyco_new_msg_h
#define dwyco_new_msg_h
#include <QByteArray>

int dwyco_new_msg(QByteArray& uid_out, QByteArray& txt, int& zap_viewer, QByteArray& mid_out, int &has_att);
#endif
