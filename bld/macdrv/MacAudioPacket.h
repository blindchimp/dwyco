
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
/*
 *  MacAudioPacket.h
 *  DwycoVideoTest
 *
 *  Created by Christopher Corbell on 11/15/09.
 *
 */

#ifndef _MACAUDIOPACKET_H_
#define _MACAUDIOPACKET_H_

#include <string>
#include <sstream>

struct MacAudioPacket {
    char *			data; // allocated with new char[]; dll will delete
    int				dataSize;
    Float64			sampleTime;

    MacAudioPacket() : data(NULL), dataSize(0), sampleTime(0.0) {}

    std::string ToString() {
        std::stringstream ss;
        ss << "<MacAudioPacket dataSize=\"" << dataSize << "\" sampleTime=\"" << sampleTime << "\"/>";
        return ss.str();
    }
};

#endif // _MACAUDIOPACKET_H_
