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

#ifndef PV_OMXDEFS_H_INCLUDED
#include "pv_omxdefs.h"
#endif

#ifndef OMX_PROXY_INTERFACE_H_INCLUDED
#include "omx_proxy_interface.h"
#endif

#ifndef PV_OMXCORE_H_INCLUDED
#include "pv_omxcore.h"
#endif

#ifndef THREADSAFE_MEMPOOL_H_INCLUDED
#include "threadsafe_mempool.h"
#endif

#if PROXY_INTERFACE

extern ProxyApplication_OMX*	pProxyTerm[];
extern OMX_HANDLETYPE		ComponentHandle[];


// the messages include input and output buffer circulation and cmd msgs into omx
// MAX_NUMBER_OF_OMX_PROXY_MSGS must be larger than the sum of:
// a) total number of omx output buffers
// b) total number of omx input buffers
// c) total number of cmds that can be sent at a time to an omx component (typically 1 or 2)
// (50 seems enough - this has to be setup up-front before anything is negotiated)
#define MAX_NUMBER_OF_OMX_PROXY_MSGS 50
#define MAX_SIZE_OF_OMX_PROXY_MSG 256


/**********************
PROXY APP CLASS FUNCTIONS
***********************/


//Constructor function for class ProxyApplication
ProxyApplication_OMX::ProxyApplication_OMX()
{
    iNumMessage = iNumNotice = 0;
    iMemCmd = 0;
    iNumClientMsg = 0;
    ipProxy = NULL;
    iProxyId = TERM_PROXY_ID;
    iNumCreate = iNumCreateAppenders = iNumDelete = 0;
    ReturnValueOmxApi = OMX_ErrorNone;

    iInitSemOmx.Create();
    iMemoryPool = NULL;
    iMemoryPool = ThreadSafeMemPoolFixedChunkAllocator::Create(MAX_NUMBER_OF_OMX_PROXY_MSGS);

    if (iMemoryPool != NULL)
    {
        // do a dummy ALLOC HERE TO Create mempool. Otherwise the mempool may be
        // created in the 2nd thread and will fail to deallocate properly.
        OsclAny *dummy = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
        iMemoryPool->deallocate(dummy);
    }

}

//Destructor function for class ProxyApplication
ProxyApplication_OMX::~ProxyApplication_OMX()
{
    iInitSemOmx.Close();
    if (iMemoryPool)
    {
        iMemoryPool->removeRef();
        iMemoryPool = NULL;
    }

}

/* Initialize the proxy objects & create a new thread */
void ProxyApplication_OMX::Start()
{
    // obtain the global memory lock (if one exists, it will be non-null)

    OsclLockBase *thread_mem_lock = OsclMem::GetLock();

    // if this lock exists, i.e. if this (master) thread already did  OsclMem::Init(lock)
    //	we'll use the same lock in the new thread to do OsclMem::Init(lock). This will provide
    // mem lock

    // if the global lock is NULL, there will be no mem lock control
    ipProxy = CPVInterfaceProxy_OMX::NewL(*this, thread_mem_lock);


    ipProxy->StartPVThread();
}

/* De-Initialize the proxy object & destroy the new thread */
void ProxyApplication_OMX::Exit()
{
    //this will stop proxy scheduler and thread.
    ipProxy->Delete();
    ipProxy = NULL;
}

void ProxyApplication_OMX::PVThreadLogon(PVMainProxy_OMX& proxy)
{
    iNumCreate++;
    iProxyId = proxy.RegisterProxiedInterface(*this, *this);
    //ProxyIdVar = iProxyId;
}

void ProxyApplication_OMX::PVThreadLogoff(PVMainProxy_OMX& proxy)
{
    iNumDelete++;
    proxy.UnregisterProxiedInterface(iProxyId);
}

/* Unpack the message and call the appropriate OpenMAX API*/
void ProxyApplication_OMX::ProcessMessage(TPVCommandId cmdid, OsclAny* cmd)
{
    iNumMessage++;

    switch (cmdid)
    {
        case GET_PARAMS:
        {
            GetParameterMsg* Command = (GetParameterMsg*) cmd;

            ReturnValueOmxApi = ComponentGetParameter(Command->hComponent, Command->nParamIndex, Command->ComponentParameterStructure);

            iMemoryPool->deallocate((OsclAny*)Command);

            //delete Command;
            iMemCmd--;
            iInitSemOmx.Signal();
        }
        break;

        case SET_PARAMS:
        {
            SetParameterMsg* Command = (SetParameterMsg*) cmd;
            ReturnValueOmxApi = ComponentSetParameter(Command->hComponent, Command->nParamIndex, Command->ComponentParameterStructure);
            iMemoryPool->deallocate((OsclAny*)Command);

            //delete Command;
            iMemCmd--;
            iInitSemOmx.Signal();
        }
        break;

        case GET_CONFIG:
        {
            GetConfigMsg* Command = (GetConfigMsg*) cmd;
            ReturnValueOmxApi = ComponentGetConfig(Command->hComponent, Command->nIndex, Command->pComponentConfigStructure);
            //delete Command;
            iMemoryPool->deallocate((OsclAny*)Command);

            iMemCmd--;
            iInitSemOmx.Signal();
        }
        break;

        case SET_CONFIG:
        {
            SetConfigMsg* Command = (SetConfigMsg*) cmd;
            ReturnValueOmxApi = ComponentSetConfig(Command->hComponent, Command->nIndex, Command->pComponentConfigStructure);
            //delete Command;
            iMemoryPool->deallocate((OsclAny*)Command);

            iMemCmd--;
            iInitSemOmx.Signal();
        }
        break;

        case GET_EXT:
        {
            GetExtMsg* Command = (GetExtMsg*) cmd;
            ReturnValueOmxApi = ComponentGetExtensionIndex(Command->hComponent, Command->cParameterName, Command->pIndexType);
            //delete Command;
            iMemoryPool->deallocate((OsclAny*)Command);

            iMemCmd--;
            iInitSemOmx.Signal();
        }
        break;

        case GET_ST:
        {
            GetStateMsg* Command = (GetStateMsg*) cmd;
            ReturnValueOmxApi = ComponentGetState(Command->hComponent, Command->pState);
            //delete Command;
            iMemoryPool->deallocate((OsclAny*)Command);

            iMemCmd--;
            iInitSemOmx.Signal();
        }
        break;

        case USE_BUF:
        {
            UseBufMsg* Command = (UseBufMsg*) cmd;
            ReturnValueOmxApi = ComponentUseBuffer(Command->hComponent, Command->ppBufferHdr, Command->nPortIndex, Command->pAppPrivate, Command->nSizeBytes, Command->pBuffer);
            //delete Command;
            iMemoryPool->deallocate((OsclAny*)Command);

            iMemCmd--;
            iInitSemOmx.Signal();
        }
        break;

        case ALLOC_BUF:
        {
            AllocBufMsg* Command = (AllocBufMsg*) cmd;
            ReturnValueOmxApi = ComponentAllocateBuffer(Command->hComponent, Command->pBuffer, Command->nPortIndex, Command->pAppPrivate, Command->nSizeBytes);
            //delete Command;
            iMemoryPool->deallocate((OsclAny*)Command);

            iMemCmd--;
            iInitSemOmx.Signal();
        }
        break;

        case FREE_BUF:
        {
            FreeBufMsg* Command = (FreeBufMsg*) cmd;
            ReturnValueOmxApi = ComponentFreeBuffer(Command->hComponent, Command->nPortIndex, Command->pBuffer);
            //delete Command;
            iMemoryPool->deallocate((OsclAny*)Command);

            iMemCmd--;
            iInitSemOmx.Signal();
        }
        break;

        case SET_CALL:
        {
            SetCallMsg* Command = (SetCallMsg*) cmd;
            ReturnValueOmxApi = ComponentSetCallbacks(Command->hComponent, Command->pCallbacks, Command->pAppData);
            //delete Command;
            iMemoryPool->deallocate((OsclAny*)Command);

            iMemCmd--;
            iInitSemOmx.Signal();
        }
        break;

        case SEND_COMM:
        {
            SetCommMsg* Command = (SetCommMsg*) cmd;
            ReturnValueOmxApi = ComponentSendCommand(Command->hComponent, Command->Cmd, Command->nParam, Command->pCmdData);
            //delete Command;
            iMemoryPool->deallocate((OsclAny*)Command);

            iMemCmd--;
            iInitSemOmx.Signal();
        }
        break;

        case EMPTY_BUF:
        {
            EmptyBufMsg* Command = (EmptyBufMsg*) cmd;
            // Do not use wait/signal semaphore for queuing input/output buffers
            // allow the client thread to keep going without having to check the status (assume its OK)
            ComponentEmptyThisBuffer(Command->hComponent, Command->pBuffer);
            //delete Command;
            iMemoryPool->deallocate((OsclAny*)Command);

            iMemCmd--;

        }
        break;

        case FILL_BUF:
        {
            FillBufMsg* Command = (FillBufMsg*) cmd;
            // Do not use wait/signal semaphore for queuing input/output buffers
            // allow the client thread to keep going without having to check the status (assume its OK)
            ComponentFillThisBuffer(Command->hComponent, Command->pBuffer);
            //delete Command;
            iMemoryPool->deallocate((OsclAny*)Command);

            iMemCmd--;
        }
        break;

        case GET_HANDLE:
        {
            GetHandleMsg* Command = (GetHandleMsg*) cmd;

            ReturnValueOmxApi = GlobalProxyComponentGetHandle(Command->pHandle, Command->cComponentName,
                                Command->pAppData, Command->pCallBacks);
            //delete Command;
            iMemoryPool->deallocate((OsclAny*)Command);

            iMemCmd--;
            iInitSemOmx.Signal();
        }
        break;

        case FREE_HANDLE:
        {
            FreeHandleMsg* Command = (FreeHandleMsg*) cmd;


            ReturnValueOmxApi = GlobalProxyComponentFreeHandle(Command->hComponent);
            //delete Command;
            iMemoryPool->deallocate((OsclAny*)Command);

            iMemCmd--;
            iInitSemOmx.Signal();
        }
        break;

        default:
        {
            //printf("\n Invalid API called");
        }
        break;
    }
}


void ProxyApplication_OMX::CleanupMessage(OsclAny* Msg)
{
    proxyApplicationCommand* Command = (proxyApplicationCommand*) Msg;
    //delete Command;
    iMemoryPool->deallocate((OsclAny*)Command);

    iMemCmd--;
}

void ProxyApplication_OMX::HandleCommand(TPVProxyMsgId msgid , TPVCommandId cmdid, OsclAny* aData)
{
    OSCL_UNUSED_ARG(msgid);
    ProcessMessage(cmdid, (OsclAny*) aData);
}

void ProxyApplication_OMX::CleanupNotification(TPVProxyMsgId , OsclAny* aData)
{
    OSCL_UNUSED_ARG(aData);
    //not required in our case
}

void ProxyApplication_OMX::HandleNotification(TPVProxyMsgId , OsclAny* aData)
{
    OSCL_UNUSED_ARG(aData);
    //not required in our case
}

void ProxyApplication_OMX::CleanupCommand(TPVProxyMsgId , OsclAny* aData)
{
    CleanupMessage((OsclAny*) aData);
}

void CreateAppenders()
{
    // to output messages we must create an appender.  here we will
    // use the stderr appender.

    // to prepend the time and message id to all logged messages
    // we must add a layout object to the appender

    PVLoggerAppender* appender = new StdErrAppender<TimeAndIdLayout, 1024>();

    OsclRefCounterSA<BasicDestructDealloc> *appenderRefCounter =
        new OsclRefCounterSA<BasicDestructDealloc>(appender);

    OsclSharedPtr<PVLoggerAppender> appenderPtr(appender, appenderRefCounter);

    // add the appender to the root node of the tree.  this
    // will log all messages from all nodes which enable appender inheritance (default)

    PVLogger* rootnode = PVLogger::GetLoggerObject("");
    rootnode->AddAppender(appenderPtr);

    // set the log level for the root node

    rootnode->SetLogLevel(PVLOGMSG_DEBUG);


    //force the scheduler logger to be created now, so
    //it won't mess up the heap checks by getting
    //created later
    PVLogger::GetLoggerObject("pvscheduler");
    PVLogger::GetLoggerObject("pvproxy");
}


/**************************
PROXY API'S OF CLASS PROXYAPPLICATION START FROM HERE
THESE API'S ARE BEING CALLED FROM THE WRAPPER FUNCTIONS
****************************/

OMX_ERRORTYPE ProxyApplication_OMX::ProxyGetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));

    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }

    GetConfigMsg* Msg = new(ptr) GetConfigMsg(hComponent, nIndex, pComponentConfigStructure);


    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, GET_CONFIG, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }

    iInitSemOmx.Wait();

    return ReturnValueOmxApi;

}

OMX_ERRORTYPE ProxyApplication_OMX::ProxySetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_IN  OMX_PTR pComponentConfigStructure)
{
    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }

    SetConfigMsg* Msg = new(ptr) SetConfigMsg(hComponent, nIndex, pComponentConfigStructure);



    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, SET_CONFIG, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }

    iInitSemOmx.Wait();
    return ReturnValueOmxApi;
}

OMX_ERRORTYPE ProxyApplication_OMX::ProxyGetExtensionIndex(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_STRING cParameterName,
    OMX_OUT OMX_INDEXTYPE* pIndexType)
{

    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }

    GetExtMsg* Msg = new(ptr) GetExtMsg(hComponent, cParameterName, pIndexType);

    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, GET_EXT, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }

    iInitSemOmx.Wait();
    return ReturnValueOmxApi;
}

OMX_ERRORTYPE ProxyApplication_OMX::ProxyGetState(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_STATETYPE* pState)
{

    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }

    GetStateMsg* Msg = new(ptr) GetStateMsg(hComponent, pState);


    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, GET_ST, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }

    iInitSemOmx.Wait();
    return ReturnValueOmxApi;
}


OMX_ERRORTYPE ProxyApplication_OMX::ProxyGetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }

    GetParameterMsg* Msg = new(ptr) GetParameterMsg(hComponent, nParamIndex, ComponentParameterStructure);


    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, GET_PARAMS, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }

    iInitSemOmx.Wait();

    return ReturnValueOmxApi;
}


OMX_ERRORTYPE ProxyApplication_OMX::ProxySetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_IN  OMX_PTR ComponentParameterStructure)
{
    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }

    SetParameterMsg* Msg = new(ptr) SetParameterMsg(hComponent, nParamIndex, ComponentParameterStructure);



    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, SET_PARAMS, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }

    iInitSemOmx.Wait();

    return ReturnValueOmxApi;
}


OMX_ERRORTYPE ProxyApplication_OMX::ProxyUseBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes,
    OMX_IN OMX_U8* pBuffer)
{
    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }

    UseBufMsg* Msg = new(ptr) UseBufMsg(hComponent, ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);


    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, USE_BUF, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }

    iInitSemOmx.Wait();

    return ReturnValueOmxApi;
}

OMX_ERRORTYPE ProxyApplication_OMX::ProxyAllocateBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes)
{
    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }

    AllocBufMsg* Msg = new(ptr) AllocBufMsg(hComponent, pBuffer, nPortIndex, pAppPrivate, nSizeBytes);


    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, ALLOC_BUF, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }

    iInitSemOmx.Wait();
    return ReturnValueOmxApi;
}

OMX_ERRORTYPE ProxyApplication_OMX::ProxyFreeBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_U32 nPortIndex,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }

    FreeBufMsg* Msg = new(ptr) FreeBufMsg(hComponent, nPortIndex, pBuffer);


    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, FREE_BUF, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }

    iInitSemOmx.Wait();
    return ReturnValueOmxApi;
}

OMX_ERRORTYPE ProxyApplication_OMX::ProxySetCallbacks(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
    OMX_IN  OMX_PTR pAppData)
{
    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }

    SetCallMsg* Msg = new(ptr) SetCallMsg(hComponent, pCallbacks, pAppData);

    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, SET_CALL, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }

    iInitSemOmx.Wait();
    return ReturnValueOmxApi;
}

OMX_ERRORTYPE ProxyApplication_OMX::ProxySendCommand(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_COMMANDTYPE Cmd,
    OMX_IN  OMX_U32 nParam,
    OMX_IN  OMX_PTR pCmdData)
{
    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }

    SetCommMsg* Msg = new(ptr) SetCommMsg(hComponent, Cmd, nParam, pCmdData);

    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, SEND_COMM, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }
    iInitSemOmx.Wait();

    return ReturnValueOmxApi;
}

OMX_ERRORTYPE ProxyApplication_OMX::ProxyEmptyThisBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }

    EmptyBufMsg* Msg = new(ptr) EmptyBufMsg(hComponent, pBuffer);


    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, EMPTY_BUF, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }

    // Do not use wait/signal semaphore for queuing input/output buffers
    // allow the client thread to keep going without having to wait for the status (assume its OK)

    return OMX_ErrorNone;
}


OMX_ERRORTYPE ProxyApplication_OMX::ProxyFillThisBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }


    FillBufMsg* Msg = new(ptr) FillBufMsg(hComponent, pBuffer);

    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, FILL_BUF, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }

    // Do not use wait/signal semaphore for queuing input/output buffers
    // allow the client thread to keep going without having to wait for the status (assume its OK)

    return OMX_ErrorNone;
}


OMX_API OMX_ERRORTYPE OMX_APIENTRY ProxyApplication_OMX::ProxyGetHandle(
    OMX_OUT OMX_HANDLETYPE* pHandle,
    OMX_IN  OMX_STRING cComponentName, OMX_IN  OMX_PTR pAppData,
    OMX_IN  OMX_CALLBACKTYPE* pCallBacks)
{
    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }

    GetHandleMsg* Msg = new(ptr) GetHandleMsg(pHandle, cComponentName, pAppData, pCallBacks);


    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, GET_HANDLE, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }

    iInitSemOmx.Wait();

    return ReturnValueOmxApi;
}


OMX_API OMX_ERRORTYPE OMX_APIENTRY ProxyApplication_OMX::ProxyFreeHandle(
    OMX_IN OMX_HANDLETYPE hComponent)
{
    OsclAny *ptr = iMemoryPool->allocate(MAX_SIZE_OF_OMX_PROXY_MSG * sizeof(uint8));
    if (NULL == ptr)
    {
        return OMX_ErrorInsufficientResources;
    }

    FreeHandleMsg* Msg = new(ptr) FreeHandleMsg(hComponent);


    iMemCmd++;
    iNumClientMsg++;

    if ((ipProxy->SendCommand(iProxyId, FREE_HANDLE, (OsclAny*) Msg)) == false)
    {
        return OMX_ErrorUndefined;
    }

    iInitSemOmx.Wait();

    return ReturnValueOmxApi;
}


/**********************************
WRAPPER FUNCTIONS START FROM HERE
THESE FUNCTIONS ARE MAPPED TO THE OPENMAX MACROS AND METHODS
***********************************/

OMX_ERRORTYPE WrapperGetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    OMX_ERRORTYPE ReturnValue;
    OMX_S32 ii, ComponentNumber = 0;

    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {
        if (hComponent == ComponentHandle[ii])
        {
            ComponentNumber = ii;
            break;
        }
    }

    ReturnValue = pProxyTerm[ComponentNumber]->ProxyGetConfig(hComponent, nIndex, pComponentConfigStructure);
    return ReturnValue;
}

OMX_ERRORTYPE WrapperSetConfig(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nIndex,
    OMX_IN  OMX_PTR pComponentConfigStructure)
{
    OMX_ERRORTYPE ReturnValue;
    OMX_S32 ii, ComponentNumber = 0;

    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {
        if (hComponent == ComponentHandle[ii])
        {
            ComponentNumber = ii;
            break;
        }
    }

    ReturnValue = pProxyTerm[ComponentNumber]->ProxySetConfig(hComponent, nIndex, pComponentConfigStructure);
    return ReturnValue;
}

OMX_ERRORTYPE WrapperGetExtensionIndex(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_STRING cParameterName,
    OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    OMX_ERRORTYPE ReturnValue;
    OMX_S32 ii, ComponentNumber = 0;

    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {
        if (hComponent == ComponentHandle[ii])
        {
            ComponentNumber = ii;
            break;
        }
    }

    ReturnValue = pProxyTerm[ComponentNumber]->ProxyGetExtensionIndex(hComponent, cParameterName, pIndexType);
    return ReturnValue;
}

OMX_ERRORTYPE WrapperGetState(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_STATETYPE* pState)
{
    OMX_ERRORTYPE ReturnValue;
    OMX_S32 ii, ComponentNumber = 0;

    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {
        if (hComponent == ComponentHandle[ii])
        {
            ComponentNumber = ii;
            break;
        }
    }

    ReturnValue = pProxyTerm[ComponentNumber]->ProxyGetState(hComponent, pState);
    return ReturnValue;
}


OMX_ERRORTYPE WrapperGetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE ReturnValue = OMX_ErrorNone;
    OMX_S32 ii, ComponentNumber = 0;

    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {
        if (hComponent == ComponentHandle[ii])
        {
            ComponentNumber = ii;
            break;
        }
    }

    ReturnValue = pProxyTerm[ComponentNumber]->ProxyGetParameter(hComponent, nParamIndex, ComponentParameterStructure);
    return ReturnValue;
}


OMX_ERRORTYPE WrapperSetParameter(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_INDEXTYPE nParamIndex,
    OMX_IN  OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE ReturnValue;
    OMX_S32 ii, ComponentNumber = 0;

    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {
        if (hComponent == ComponentHandle[ii])
        {
            ComponentNumber = ii;
            break;
        }
    }

    ReturnValue = pProxyTerm[ComponentNumber]->ProxySetParameter(hComponent, nParamIndex, ComponentParameterStructure);
    return ReturnValue;
}


OMX_ERRORTYPE WrapperUseBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes,
    OMX_IN OMX_U8* pBuffer)
{
    OMX_ERRORTYPE ReturnValue;
    OMX_S32 ii, ComponentNumber = 0;

    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {
        if (hComponent == ComponentHandle[ii])
        {
            ComponentNumber = ii;
            break;
        }
    }

    ReturnValue = pProxyTerm[ComponentNumber]->ProxyUseBuffer(hComponent, ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);
    return ReturnValue;
}

OMX_ERRORTYPE WrapperAllocateBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE ReturnValue;
    OMX_S32 ii, ComponentNumber = 0;

    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {
        if (hComponent == ComponentHandle[ii])
        {
            ComponentNumber = ii;
            break;
        }
    }

    ReturnValue = pProxyTerm[ComponentNumber]->ProxyAllocateBuffer(hComponent, pBuffer, nPortIndex, pAppPrivate, nSizeBytes);
    return ReturnValue;
}

OMX_ERRORTYPE WrapperFreeBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_U32 nPortIndex,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE ReturnValue;
    OMX_S32 ii, ComponentNumber = 0;

    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {
        if (hComponent == ComponentHandle[ii])
        {
            ComponentNumber = ii;
            break;
        }
    }

    ReturnValue = pProxyTerm[ComponentNumber]->ProxyFreeBuffer(hComponent, nPortIndex, pBuffer);
    return ReturnValue;
}

OMX_ERRORTYPE WrapperSetCallbacks(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
    OMX_IN  OMX_PTR pAppData)
{
    OMX_ERRORTYPE ReturnValue;
    OMX_S32 ii, ComponentNumber = 0;

    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {
        if (hComponent == ComponentHandle[ii])
        {
            ComponentNumber = ii;
            break;
        }
    }

    ReturnValue = pProxyTerm[ComponentNumber]->ProxySetCallbacks(hComponent, pCallbacks, pAppData);
    return ReturnValue;
}

OMX_ERRORTYPE WrapperSendCommand(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_COMMANDTYPE Cmd,
    OMX_IN  OMX_U32 nParam,
    OMX_IN  OMX_PTR pCmdData)
{
    OMX_ERRORTYPE ReturnValue;
    OMX_S32 ii, ComponentNumber = 0;

    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {
        if (hComponent == ComponentHandle[ii])
        {
            ComponentNumber = ii;
            break;
        }
    }

    ReturnValue = pProxyTerm[ComponentNumber]->ProxySendCommand(hComponent, Cmd, nParam, pCmdData);
    return ReturnValue;
}

OMX_ERRORTYPE WrapperEmptyThisBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE ReturnValue;
    OMX_S32 ii;
    OMX_S32 ComponentNumber = 0;

    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {
        if (hComponent == ComponentHandle[ii])
        {
            ComponentNumber = ii;
            break;
        }
    }

    ReturnValue = pProxyTerm[ComponentNumber]->ProxyEmptyThisBuffer(hComponent, pBuffer);
    return ReturnValue;
}


OMX_ERRORTYPE WrapperFillThisBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE ReturnValue;
    OMX_S32 ii, ComponentNumber = 0;

    for (ii = 0; ii < MAX_INSTANTIATED_COMPONENTS; ii++)
    {
        if (hComponent == ComponentHandle[ii])
        {
            ComponentNumber = ii;
            break;
        }
    }

    ReturnValue = pProxyTerm[ComponentNumber]->ProxyFillThisBuffer(hComponent, pBuffer);
    return ReturnValue;
}

#endif // PROXY_INTERFACE
