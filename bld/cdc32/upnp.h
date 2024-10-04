
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef UPNP_H
#define UPNP_H

namespace dwyco {

int bg_upnp(int natport1, int natport2, int local_port1, int local_port2);

int do_upnp(int natport1, int natport2, int local_port1, int local_port2);

}

#endif // UPNP_H
