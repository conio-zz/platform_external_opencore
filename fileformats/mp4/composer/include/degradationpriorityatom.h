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
/*********************************************************************************/
/*
    This PVA_FF_DegradationPriorityAtom Class provides a compact marking of the random access
    points within the stream.
*/


#ifndef __DegradationPriorityAtom_H__
#define __DegradationPriorityAtom_H__

#include "fullatom.h"


class PVA_FF_DegradationPriorityAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_DegradationPriorityAtom(uint8 version, uint32 flags); // Constructor

        virtual ~PVA_FF_DegradationPriorityAtom();

        // Adding to and getting the sample priority values
        void addPriority(uint16 priority);
        Oscl_Vector<uint16, OsclMemAllocator>* getPriorities()
        {
            return _ppriorities;
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:

        Oscl_Vector<uint16, OsclMemAllocator>* _ppriorities;
};


#endif

