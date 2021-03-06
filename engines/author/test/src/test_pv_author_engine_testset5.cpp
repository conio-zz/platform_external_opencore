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
#ifndef TEST_PV_AUTHOR_ENGINE_TESTSET5_H_INCLUDED
#include "test_pv_author_engine_testset5.h"
#endif

#ifndef PVMF_COMPOSER_SIZE_AND_DURATION_H_INCLUDED
#include "pvmf_composer_size_and_duration.h"
#endif

#ifndef PVMF_FILEOUTPUT_CONFIG_H_INCLUDED
#include "pvmf_fileoutput_config.h"
#endif

#ifndef PVMP4FFCN_CLIPCONFIG_H_INCLUDED
#include "pvmp4ffcn_clipconfig.h"
#endif

#ifndef PV_MP4_H263_ENC_EXTENSION_H_INCLUDED
#include "pvmp4h263encextension.h"
#endif

#ifndef PVAETEST_NODE_CONFIG_H_INCLUDED
#include "pvaetest_node_config.h"
#endif

void pv_mediainput_async_test_opencomposestop::StartTest()
{
    AddToScheduler();
    iState = PVAE_CMD_CREATE;
    RunIfNotReady();
}


/*@todo add error handling here */
////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_opencomposestop::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "pv_mediainput_async_test_opencomposestop::HandleErrorEvent"));
    iState = PVAE_CMD_RESET;
    RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_opencomposestop::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pv_mediainput_async_test_opencomposestop::HandleInformationalEvent"));

    OsclAny* eventData = NULL;
    switch (aEvent.GetEventType())
    {
        case PVMF_COMPOSER_MAXFILESIZE_REACHED:
        case PVMF_COMPOSER_MAXDURATION_REACHED:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "pv_mediainput_async_test_opencomposestop::HandleNodeInformationalEvent: Max file size reached"));
            iState = PVAE_CMD_RESET;

            Cancel();
            PVPATB_TEST_IS_TRUE(true);
            RunIfNotReady();

            fprintf(iFile, "Recording finished, Closing file please wait *******\n");
            break;

        case PVMF_COMPOSER_DURATION_PROGRESS:
            aEvent.GetEventData(eventData);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "pv_mediainput_async_test_opencomposestop::HandleNodeInformationalEvent: Duration progress: %d ms",
                             (int32)eventData));
            break;

        case PVMF_COMPOSER_FILESIZE_PROGRESS:
            aEvent.GetEventData(eventData);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "pv_mediainput_async_test_opencomposestop::HandleNodeInformationalEvent: File size progress: %d bytes",
                             (int32)eventData));
            break;

        case PVMF_COMPOSER_EOS_REACHED:
            //Engine already stopped at EOS so send reset command.
            iState = PVAE_CMD_RESET;
            //cancel recording timeout scheduled for timer object.
            Cancel();
            RunIfNotReady();

            fprintf(iFile, "Recording finished, Closing file please wait *******\n");

            break;

        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_opencomposestop::CreateTestInputs()
{
    int32 status = 0;
    int32 error = 0;
    iFileParser = NULL;

    iFileServer.Connect();
    uint32 numloops = 0;

    if (PVMF_AVIFF == iMediaInputType)
    {

        OSCL_TRY(error, iFileParser = PVAviFile::CreateAviFileParser(iInputFileName, error, &iFileServer););

        if (error || (NULL == iFileParser))
        {
            if (iFileParser)
            {
                goto ERROR_CODE;
            }
            else
            {
                return false;
            }
        }

        //set recording duration to TEST_TIMEOUT_FACTOR times the file duration in order to time out
        //if EOS is not received by this time.
        iTestDuration = (TEST_TIMEOUT_FACTOR * ((PVAviFile*)iFileParser)->GetFileDuration());

        if (iTestDuration > KAuthoringSessionUnit)
        {
            //if test duration, in microsec, is too large, the uint32
            //variable will rollover. Hence the test run is divided
            //in small sessions of KAuthoringSessionUnit duration
            iAuthoringCount = iTestDuration / KAuthoringSessionUnit;
            iTestDuration = KAuthoringSessionUnit * 1000 * 1000; //in microsec
        }
        else
        {
            iTestDuration = iTestDuration * 1000 * 1000;
        }
        uint32 numStreams = ((PVAviFile*)iFileParser)->GetNumStreams();

        iAddAudioMediaTrack = false;
        iAddVideoMediaTrack = false;

        for (uint32 ii = 0; ii < numStreams; ii++)
        {
            if (oscl_strstr(((PVAviFile*)iFileParser)->GetStreamMimeType(ii).get_cstr(), "audio"))
            {
                iAddAudioMediaTrack = true;
            }

            if (oscl_strstr(((PVAviFile*)iFileParser)->GetStreamMimeType(ii).get_cstr(), "video"))
            {
                iAddVideoMediaTrack = true;
            }

        }

    }
    else if (PVMF_WAVFF == iMediaInputType)
    {
        OSCL_TRY(error, iFileParser = OSCL_NEW(PV_Wav_Parser, ()););
        if (error || (NULL == iFileParser))
        {
            return false;
        }
        if (((PV_Wav_Parser*)iFileParser)->InitWavParser(iInputFileName, NULL) != PVWAVPARSER_OK)
        {

            goto ERROR_CODE;
        }

        iAddAudioMediaTrack = true;
        iAddVideoMediaTrack = false;

        PVWAVFileInfo fileInfo;

        ((PV_Wav_Parser*)iFileParser)->RetrieveFileInfo(fileInfo);

        //set recording duration to TEST_TIMEOUT_FACTOR times the file duration in order to time out
        //if EOS is not received by this time.
        iTestDuration = (TEST_TIMEOUT_FACTOR * ((fileInfo.NumSamples * fileInfo.BytesPerSample) / fileInfo.SampleRate)) ; // in sec

        if (iTestDuration > KAuthoringSessionUnit)
        {
            //if test duration, in microsec, is too large, the uint32
            //variable will rollover. Hence the test run is divided
            //in small sessions of KAuthoringSessionUnit duration
            iAuthoringCount = iTestDuration / KAuthoringSessionUnit;
            iTestDuration = KAuthoringSessionUnit * 1000 * 1000; //in microsec
        }
        else
        {
            iTestDuration = iTestDuration * 1000 * 1000;
        }

    }

    {

        PVMIOControlComp MIOComp(iMediaInputType, (OsclAny*)iFileParser, iLoopTime);

        if (iLoopTime)
        {

            iTestDuration = TEST_TIMEOUT_FACTOR * iLoopTime ; //timeout duration in microsec
            if (iTestDuration > KAuthoringSessionUnit)
            {
                //if test duration, in microsec, is too large, the uint32
                //variable will rollover. Hence the test run is divided
                //in small sessions of KAuthoringSessionUnit duration
                iAuthoringCount = iTestDuration / KAuthoringSessionUnit;
                iTestDuration = KAuthoringSessionUnit * 1000 * 1000; //in microsec
            }
            else
            {
                iTestDuration = iTestDuration * 1000 * 1000;
            }

        }

        status = MIOComp.CreateMIOInputNode(iRealTimeAuthoring, iMediaInputType, iInputFileName);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "pv_mediainput_async_test_opencomposestop::CreateTestInputs: Error - CreateMIOInputNode failed"));

            //delete any MIO Comp created.
            MIOComp.DeleteInputNode();
            goto ERROR_CODE;

        }

        iMIOComponent = MIOComp;
        if (!AddDataSource())
        {

            //delete any MIO Comp created.
            MIOComp.DeleteInputNode();
            goto ERROR_CODE;

        }

        return true;

    }


ERROR_CODE:
    {
        //remove file parser
        if (PVMF_AVIFF == iMediaInputType)
        {
            PVAviFile* fileparser = OSCL_STATIC_CAST(PVAviFile*, iFileParser);
            PVAviFile::DeleteAviFileParser(fileparser);
            fileparser = NULL;
            iFileParser = NULL;
        }
        else if (PVMF_WAVFF == iMediaInputType)
        {
            PV_Wav_Parser* fileparser = OSCL_STATIC_CAST(PV_Wav_Parser*, iFileParser);
            delete(fileparser);
            fileparser = NULL;
            iFileParser = NULL;
        }

        return false;
    }


}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_opencomposestop::AddDataSource()
{
    int32 err = 0;
    uint32 ii = 0;
    uint32 noOfNodes = iMIOComponent.iMIONode.size();

    for (ii = 0; ii < noOfNodes; ii++)
    {
        AddEngineCommand();

        OSCL_TRY(err, iAuthor->AddDataSource(*(iMIOComponent.iMIONode[ii]), (OsclAny*)iAuthor););

        if (err != OSCL_ERR_NONE)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "pvauthor_async_test_miscellaneous::AddDataSource: Error - iAuthor->AddDataSource failed. err=0x%x", err));

            iMIOComponent.DeleteInputNode();
            return false;
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_opencomposestop::ConfigComposer()
{
    if (!ConfigOutputFile())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_opencomposestop::ConfigComposer: Error - ConfigOutputFile failed"));

        return false;
    }

    if ((oscl_strstr(iComposerMimeType.get_str(), "mp4")) || (oscl_strstr(iComposerMimeType.get_str(), "3gp")))
    {
        if (!ConfigMp43gpComposer())
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "pv_mediainput_async_test_opencomposestop::ConfigComposer: Error - ConfigMp43gpComposer failed"));

            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_opencomposestop::ConfigOutputFile()
{

    PvmfFileOutputNodeConfigInterface* clipConfig = OSCL_STATIC_CAST(PvmfFileOutputNodeConfigInterface*, iComposerConfig);
    if (!clipConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_opencomposestop::ConfigAmrComposer: Error - Invalid iComposerConfig"));

        return false;
    }

    if (clipConfig->SetOutputFileName(iOutputFileName) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_opencomposestop::ConfigAmrComposer: Error - SetOutputFileName failed"));

        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_opencomposestop::ConfigMp43gpComposer()
{

    PVMp4FFCNClipConfigInterface* clipConfig;
    clipConfig = OSCL_STATIC_CAST(PVMp4FFCNClipConfigInterface*, iComposerConfig);
    if (!clipConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pv_mediainput_async_test_opencomposestop::ConfigMp43gpComposer: Error - iComposerConfig==NULL"));

        return false;
    }

    OSCL_wHeapString<OsclMemAllocator> versionString = _STRLIT("version");
    OSCL_wHeapString<OsclMemAllocator> titleString = _STRLIT("title");
    OSCL_wHeapString<OsclMemAllocator> authorString = _STRLIT("author");
    OSCL_wHeapString<OsclMemAllocator> copyrightString = _STRLIT("copyright");
    OSCL_wHeapString<OsclMemAllocator> descriptionString = _STRLIT("description");
    OSCL_wHeapString<OsclMemAllocator> ratingString = _STRLIT("rating");

    clipConfig->SetOutputFileName(iOutputFileName);
    clipConfig->SetPresentationTimescale(1000);
    clipConfig->SetVersion(versionString);
    clipConfig->SetTitle(titleString);
    clipConfig->SetAuthor(authorString);
    clipConfig->SetCopyright(copyrightString);
    clipConfig->SetDescription(descriptionString);
    clipConfig->SetRating(ratingString);


    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_opencomposestop::AddMediaTrack()
{
    PVMIOControlComp MIOComp;

    if (iAddAudioMediaTrack)
    {
        if (PVMF_AVIFF == iMediaInputType)
        {
            Oscl_Vector<uint32, OsclMemAllocator> audioStrNum;

            audioStrNum = (iMIOComponent.iPVAviFile)->GetAudioStreamCountList();

            if (audioStrNum.size() == 0)
            {
                return false;
            }

            iAuthor->AddMediaTrack(*(iMIOComponent.iMIONode[audioStrNum[0]]), iAudioEncoderMimeType,
                                   iComposer, iAudioEncoderConfig, (OsclAny*)iAuthor);

            AddEngineCommand();

        }
        else if (PVMF_WAVFF == iMediaInputType)
        {
            PVWAVFileInfo wavFileInfo;
            (iMIOComponent.iPVWavFile)->RetrieveFileInfo(wavFileInfo);


            iAuthor->AddMediaTrack(*(iMIOComponent.iMIONode[0]), iAudioEncoderMimeType,
                                   iComposer, iAudioEncoderConfig, (OsclAny*)iAuthor);

            AddEngineCommand();


        }

        if (!PVAETestNodeConfig::ConfigureAudioEncoder(iAudioEncoderConfig))
        {
            return false;
        }
    }

    if (iAddVideoMediaTrack)
    {
        if (PVMF_AVIFF == iMediaInputType)
        {
            Oscl_Vector<uint32, OsclMemAllocator> vidStrNum;
            vidStrNum = (iMIOComponent.iPVAviFile)->GetVideoStreamCountList();

            if (vidStrNum.size() == 0)
            {
                return false;
            }

            iAuthor->AddMediaTrack(*(iMIOComponent.iMIONode[vidStrNum[0]]), iVideoEncoderMimeType,
                                   iComposer, iVideoEncoderConfig, (OsclAny*)iAuthor);

            AddEngineCommand();

        }
        else if (PVMF_WAVFF == iMediaInputType)
        {
            return false;
        }

    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pv_mediainput_async_test_opencomposestop::ConfigureVideoEncoder()
{

    PVMp4H263EncExtensionInterface* config;
    config = OSCL_STATIC_CAST(PVMp4H263EncExtensionInterface*, iVideoEncoderConfig);
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "pv_mediainput_async_test_opencomposestop::ConfigureVideoEncoder: No configuration needed"));

        return true;
    }

    uint32 width = 0;
    uint32 height = 0;
    uint32 frameRate = 0;
    uint32 frameInterval = 0;

    if (PVMF_AVIFF == iMediaInputType)
    {
        Oscl_Vector<uint32, OsclMemAllocator> vidStrNum =
            (iMIOComponent.iPVAviFile)->GetVideoStreamCountList();

        width = (iMIOComponent.iPVAviFile)->GetWidth(vidStrNum[0]);
        bool orient = false;
        height = (iMIOComponent.iPVAviFile)->GetHeight(orient, vidStrNum[0]);
        frameRate = (uint32)(iMIOComponent.iPVAviFile)->GetFrameRate(vidStrNum[0]);
        frameInterval = (iMIOComponent.iPVAviFile)->GetFrameDuration();

    }

    config->SetNumLayers(KNumLayers);
    config->SetOutputBitRate(0, KVideoBitrate);
    config->SetOutputFrameSize(0, width , height);
    config->SetOutputFrameRate(0, frameRate);
    config->SetIFrameInterval(frameInterval);

    return true;
}

////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_opencomposestop::ResetAuthorConfig()
{
    if (iComposerConfig)
    {
        iComposerConfig->removeRef();
        iComposerConfig = NULL;
    }
    if (iAudioEncoderConfig)
    {
        iAudioEncoderConfig->removeRef();
        iAudioEncoderConfig = NULL;
    }
    if (iVideoEncoderConfig)
    {
        iVideoEncoderConfig->removeRef();
        iVideoEncoderConfig = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_opencomposestop::Cleanup()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "pv_mediainput_async_test_opencomposestop::Cleanup"));

    iComposer = NULL;

    ResetAuthorConfig();

    if (iAuthor)
    {
        PVAuthorEngineFactory::DeleteAuthor(iAuthor);
        iAuthor = NULL;
    }

//	iMIOComponent.DeleteInputNode();
    iOutputFileName = NULL;
    iFileServer.Close();
}


////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_opencomposestop::Run()
{
    if (IsEngineCmdPending())
    {
        return;
    }

    switch (iState)
    {
        case PVAE_CMD_CREATE:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "******pv_mediainput_async_test_opencomposestop::iTestCaseNum:%d******", iTestCaseNum));
            fprintf(iFile, "Creating Author Engine \n");

            iAuthor = PVAuthorEngineFactory::CreateAuthor(this, this, this);
            if (!iAuthor)
            {
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                RunIfNotReady();

            }
            else
            {
                iState = PVAE_CMD_OPEN;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_OPEN:
        {
            fprintf(iFile, "Opening Author Engine\n");
            iAuthor->Open((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_ADD_DATA_SOURCE:
        {
            fprintf(iFile, "Add Data Source\n");
            bool aStatus = CreateTestInputs();
            if (aStatus == 0) //Failed while creating test input
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pv_mediainput_async_test_opencomposestop::CreateTestInputs: Error - failed"));

                PVPATB_TEST_IS_TRUE(false);

                iState = PVAE_CMD_CLEANUPANDCOMPLETE;

                RunIfNotReady();
            }

        }
        break;

        case PVAE_CMD_SELECT_COMPOSER:
        {
            fprintf(iFile, "Select Composer\n");
            iAuthor->SelectComposer(iComposerMimeType, iComposerConfig,
                                    (OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_ADD_MEDIA_TRACK:
        {
            fprintf(iFile, "Add Media Track\n");
            if (!AddMediaTrack())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pv_mediainput_async_test_opencomposestop::AddMediaTrack Error - No track added"));
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_REMOVE_DATA_SOURCE;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_INIT:
        {
            fprintf(iFile, "Initializing  Author Engine\n");
            iAuthor->Init((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_START:
        {
            fprintf(iFile, "Starting Author Engine\n");
            iAuthor->Start();
        }
        break;

        case PVAE_CMD_STOP:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "pv_mediainput_async_test_opencomposestop::Run: Command Stop"));

            fprintf(iFile, "Error: Recording timeout, stop engine please wait *******\n");
            iAuthor->Stop((OsclAny*)iAuthor);

            //flag error as no EOS was found and recording timedout
            PVPATB_TEST_IS_TRUE(false);

        }
        break;

        case PVAE_CMD_RESET:
        {
            ResetAuthorConfig();
            iAuthor->Reset((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_REMOVE_DATA_SOURCE:
        {
            for (uint32 ii = 0; ii < iMIOComponent.iMIONode.size(); ii++)
            {
                iAuthor->RemoveDataSource(*(iMIOComponent.iMIONode[ii]), (OsclAny*)iAuthor);
                AddEngineCommand();
            }
        }
        break;

        case PVAE_CMD_CLOSE:
        {
            iAuthor->Close((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_PAUSE:
        {
            fprintf(iFile, "Pause Initiated ********\n");
            iAuthor->Pause((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_RESUME:
        {
            fprintf(iFile, "Resume Athoring *******\n");
            iAuthor->Resume((OsclAny*)iAuthor);
        }
        break;

        case PVAE_CMD_CLEANUPANDCOMPLETE:
        {
            Cleanup();
            iObserver->CompleteTest(*iTestCase);
        }
        break;

        case PVAE_CMD_QUERY_INTERFACE2:
        case PVAE_CMD_ADD_DATA_SINK:
        case PVAE_CMD_REMOVE_DATA_SINK:
            break;

        case PVAE_CMD_RECORDING:
        {
            fprintf(iFile, "Recording Please Wait*******\n");
            if (!iAuthoringCount)
            {
                iState = PVAE_CMD_STOP;
                RunIfNotReady(iTestDuration);
            }
            else
            {
                iState = PVAE_CMD_RECORDING;
                RunIfNotReady(iTestDuration);
                iAuthoringCount--;
            }
        }
        break;

        default:
            break;
    } //end switch
}

////////////////////////////////////////////////////////////////////////////
void pv_mediainput_async_test_opencomposestop::CommandCompleted(const PVCmdResponse& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pv_mediainput_async_test_opencomposestop::CommandCompleted iState:%d", iState));

    if (aResponse.GetCmdStatus() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "pvauthor_async_test_generic_reset::CommandCompleted iState:%d FAILED", iState));
    }

    switch (iState)
    {
        case PVAE_CMD_OPEN:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_ADD_DATA_SOURCE;
                RunIfNotReady();
            }
            else
            {
                // Open failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_ADD_DATA_SOURCE:
        {
            if (EngineCmdComplete())
            {
                iState = PVAE_CMD_SELECT_COMPOSER;
            }

            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                RunIfNotReady();
            }
            else
            {
                // AddDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_SELECT_COMPOSER:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iComposer = aResponse.GetResponseData();
                if (!ConfigComposer())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "pv_mediainput_async_test_opencomposestop::CommandCompleted: Error - ConfigComposer failed"));

                    PVPATB_TEST_IS_TRUE(false);
                    iState = PVAE_CMD_RESET;
                    RunIfNotReady();
                    return;
                }
                else
                {
                    iState = PVAE_CMD_ADD_MEDIA_TRACK;
                    RunIfNotReady();
                }
            }
            else
            {
                // SelectComposer failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_ADD_MEDIA_TRACK:
        {
            if (EngineCmdComplete())
            {
                if (iAddVideoMediaTrack)
                {
                    ConfigureVideoEncoder();
                }

                iState = PVAE_CMD_INIT;
            }

            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                RunIfNotReady();
            }
            else
            {
                // AddMediaTrack failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_INIT:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_START;
                RunIfNotReady();
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_START:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iPauseResumeEnable)
                {
                    iState = PVAE_CMD_PAUSE;
                    RunIfNotReady(KPauseDuration); //Pause after 5 sec
                }
                else
                {
                    iState = PVAE_CMD_RECORDING;
                    RunIfNotReady();
                }
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_PAUSE:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                fprintf(iFile, "PAUSED: Resume after 10 seconds*******\n");
                iState = PVAE_CMD_RESUME;
                /* Stay paused for 10 seconds */
                RunIfNotReady(10*1000*1000);
            }
            else
            {
                //Pause failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_RESUME:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iState = PVAE_CMD_RECORDING;

                //Start Authoring again
                RunIfNotReady();
            }
            else
            {
                //Resume failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_STOP:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iOutputFileName = NULL;
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            else
            {
                // Stop failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_RESET:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if ((iMIOComponent.iMediaInput.size() == 0) || (iMIOComponent.iMIONode.size() == 0))
                {
                    if (aResponse.GetCmdStatus() == PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    //Since there are no MIO Components/Nodes, we end here
                    //No need to call RemoveDataSource
                    iObserver->CompleteTest(*iTestCase);
                    break;
                }

                iState = PVAE_CMD_REMOVE_DATA_SOURCE;
                RunIfNotReady();
            }
            else
            {
                // Reset failed
                PVPATB_TEST_IS_TRUE(false);
                iObserver->CompleteTest(*iTestCase);
            }
        }
        break;

        case PVAE_CMD_REMOVE_DATA_SOURCE:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (EngineCmdComplete())
                {
                    iOutputFileName = NULL;
                    iMIOComponent.DeleteInputNode();
                    if (PVMF_AVIFF == iMediaInputType)
                    {
                        PVAviFile* fileparser = OSCL_STATIC_CAST(PVAviFile*, iFileParser);
                        PVAviFile::DeleteAviFileParser(fileparser);
                        fileparser = NULL;
                        iFileParser = NULL;
                    }
                    else if (PVMF_WAVFF == iMediaInputType)
                    {
                        PV_Wav_Parser* fileparser = OSCL_STATIC_CAST(PV_Wav_Parser*, iFileParser);
                        delete(fileparser);
                        fileparser = NULL;
                        iFileParser = NULL;
                    }

                    iFileParser = NULL;
                    iState = PVAE_CMD_CLOSE;
                    RunIfNotReady();

                }
                else
                {
                    return; //wait for completion of all RemoveDataSource calls.
                }
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iObserver->CompleteTest(*iTestCase);
            }
        }
        break;

        case PVAE_CMD_CLOSE:
        {
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            iObserver->CompleteTest(*iTestCase);
        }
        break;

        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iObserver->CompleteTest(*iTestCase);
        }
        break;
    }  //end switch
}

