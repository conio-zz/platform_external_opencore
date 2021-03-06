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
#ifndef OSCL_SHARED_LIB_INTERFACE_H_INCLUDED
#define OSCL_SHARED_LIB_INTERFACE_H_INCLUDED

#ifndef OSCL_UUID_H_INCLUDED
#include "oscl_uuid.h"
#endif

class OsclSharedLibraryInterface
{
    public:
        virtual OsclAny* SharedLibraryLookup(const OsclUuid& aInterfaceId) = 0;
};
#endif // OSCL_SHARED_LIB_INTERFACE_H_INCLUDED

