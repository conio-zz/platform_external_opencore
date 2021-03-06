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

#ifndef PV_OMX_INTERFACE_PROXY_H_INCLUDED
#include "pv_omx_interface_proxy.h"
#endif

#ifndef PV_OMX_INTERFACE_PROXY_HANDLER_H_INCLUDED
#include "pv_omx_interface_proxy_handler.h"
#endif

void CPVInterfaceProxyHandler_OMX::Run()
{
    if (Status() != OSCL_REQUEST_ERR_NONE)
        return;

    CPVProxyMsg_OMX command(0, 0, 0, NULL);

    //Enter critical section...
    iProxy->iHandlerQueueCrit.Lock();

    //check command queue...
    if (iProxy->iCommandQueue.size() > 0)
    {
        command = iProxy->iCommandQueue[0];
        iProxy->iCommandQueue.erase(&iProxy->iCommandQueue[0]);
    }

    PendForExec();

    //if there's another message queued,
    //go ahead and signal request complete.
    if (iProxy->iCommandQueue.size() > 0)
        PendComplete(OSCL_REQUEST_ERR_NONE);

    //Leave critical section...
    iProxy->iHandlerQueueCrit.Unlock();

    //Process command
    if (command.iMsg)
    {
        //call proxy server message handler
        CPVProxyInterface_OMX *proxy = iProxy->FindInterface(command.iProxyId);
        if (proxy)
            proxy->iServer->HandleCommand(command.iMsgId, command.iCmdId, command.iMsg);
        else
            OsclError::Panic("PVPROXYDEBUG", 0);//debug error.
    }

}

