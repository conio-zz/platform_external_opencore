/* ------------------------------------------------------------------
 * Copyright (C) 2008 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
/*																			*/
/*	=====================================================================	*/
/*	File: evrcMediaInfoParser.h												*/
/*	Description:															*/
/*																			*/
/*																			*/
/*	Rev:																	*/
/*	Created: 05/24/01														*/
/*	=====================================================================	*/
/*																			*/
/*	Revision History:														*/
/*																			*/
/*	Rev:																	*/
/*	Date:																	*/
/*	Description:															*/
/*																			*/
/* //////////////////////////////////////////////////////////////////////// */

#ifndef EVRC_MEDIAINFO_PARSER_H
#define EVRC_MEDIAINFO_PARSER_H
#include "base_media_info_parser.h"
#include "evrc_media_info.h"

SDP_ERROR_CODE evrcMediaInfoParser(const char *, const int, SDPInfo *);

class SDPEVRCMediaInfoParser : public SDPBaseMediaInfoParser
{
    public:
        SDPEVRCMediaInfoParser() {};
        virtual ~SDPEVRCMediaInfoParser() {};
        SDP_ERROR_CODE parseMediaInfo(const char *buff, const int index, SDPInfo *sdp,
                                      payloadVector payload_vec, bool isSipSdp, int alt_id = 0, bool alt_def_id = false);
};

#endif //EVRC_MEDIAINFO_PARSER_H
