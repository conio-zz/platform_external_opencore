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
#ifndef TEST_PV_AUTHOR_ENGINE_TESTSET2_H_INCLUDED
#include "test_pv_author_engine_testset2.h"
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

void pvauthor_async_test_generic_reset::StartTest()
{
    AddToScheduler();
    iState = PVAE_CMD_CREATE;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "******pvauthor_async_test_generic_reset::iTestCaseNum:%d******", iTestCaseNum));
    RunIfNotReady();
}



////////////////////////////////////////////////////////////////////////////
void pvauthor_async_test_generic_reset::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR, (0, "pvauthor_async_test_generic_reset::HandleErrorEvent"));
    PVPATB_TEST_IS_TRUE(false);
    iObserver->CompleteTest(*iTestCase);
}

////////////////////////////////////////////////////////////////////////////
void pvauthor_async_test_generic_reset::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_test_generic_reset::HandleInformationalEvent"));
    OsclAny* eventData = NULL;
    switch (aEvent.GetEventType())
    {
        case PVMF_COMPOSER_MAXFILESIZE_REACHED:
        case PVMF_COMPOSER_MAXDURATION_REACHED:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "pvauthor_async_test_miscellaneous::HandleNodeInformationalEvent: Max file size reached"));
            Cancel();
            PVPATB_TEST_IS_TRUE(true);
            iObserver->CompleteTest(*iTestCase);
            break;

        case PVMF_COMPOSER_DURATION_PROGRESS:
            aEvent.GetEventData(eventData);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "pvauthor_async_test_miscellaneous::HandleNodeInformationalEvent: Duration progress: %d ms",
                             (int32)eventData));
            fprintf(iStdOut, "Duration: %d ms\n", (int32)eventData);
            break;

        case PVMF_COMPOSER_FILESIZE_PROGRESS:
            aEvent.GetEventData(eventData);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "pvauthor_async_test_miscellaneous::HandleNodeInformationalEvent: File size progress: %d bytes",
                             (int32)eventData));
            fprintf(iStdOut, "File size: %d bytes\n", (int32)eventData);
            break;

        case PVMF_COMPOSER_EOS_REACHED:
            //Engine already stopped at EOS so send reset command.
            iState = PVAE_CMD_RESET;
            //cancel recording timeout scheduled for timer object.
            Cancel();
            RunIfNotReady();
            break;
        default:
            break;
    }

}

////////////////////////////////////////////////////////////////////////////
int pvauthor_async_test_generic_reset::CreateAudioInput()
{
    int status = 0;
    PVAETestInput testInput;

    if (iAudioInputType == INVALID_INPUT_TYPE)	// check condition when -audio tag is not specified
    {
        return -1;
    }
    else
    {
        status = testInput.CreateInputNode(iAudioInputType, iInputFileNameAudio, iAVTConfig);
    }

    if (!status)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::CreateAudioTestInput: Error - Create input node failed"));
        return status;
    }

    return AddDataSource(testInput);
}

////////////////////////////////////////////////////////////////////////////
int pvauthor_async_test_generic_reset::CreateVideoInput()
{
    int status = 0;
    PVAETestInput testInput;

    if (iVideoInputType == INVALID_INPUT_TYPE)	// check condition when -videoo tag is not specified
    {
        return -1;
    }
    else
    {
        testInput = PVAETestInput();
        status = testInput.CreateInputNode(iVideoInputType, iInputFileNameVideo, iAVTConfig);
    }

    if (!status)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::CreateTestInputs: Error - CreateInputNode failed"));
        return status;
    }

    return AddDataSource(testInput);
}

////////////////////////////////////////////////////////////////////////////
int pvauthor_async_test_generic_reset::CreateTextInput()
{
    int status = 0;
    PVAETestInput testInput;

    if (iTextInputType == INVALID_INPUT_TYPE)	// check condition when -video tag is not specified
    {
        return -1;
    }
    else
    {
        testInput = PVAETestInput();
        status = testInput.CreateInputNode(iTextInputType, iInputFileNameText, iAVTConfig);
    }

    if (!status)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::CreateTestInputs: Error - CreateInputNode failed"));
        return status;
    }

    return AddDataSource(testInput);
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_test_generic_reset::AddDataSource(PVAETestInput& aInput)
{
    int32 err = 0;

    OSCL_TRY(err, iTestInputs.push_back(aInput););
    if (err != OSCL_ERR_NONE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::AddDataSource: Error - iTestInputs.push_back failed. err=0x%x", err));
        aInput.DeleteInputNode();
        return false;
    }


    OSCL_TRY(err, iAuthor->AddDataSource(*(aInput.iNode), (OsclAny*)iAuthor););
    if (err != OSCL_ERR_NONE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::AddDataSource: Error - iAuthor->AddDataSource failed. err=0x%x", err));
        aInput.DeleteInputNode();
        return false;
    }

    return true;
}

void pvauthor_async_test_generic_reset::SelectComposer()
{
    iAuthor->SelectComposer(iComposerMimeType, iComposerConfig, (OsclAny*)iAuthor);
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_test_generic_reset::ConfigComposer()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_test_generic_reset::ConfigComposer"));
    if ((iTestCaseNum >= GenericTestBegin) && (iTestCaseNum < GenericTestEnd))
    {
        uint32 aLenFileName;
        aLenFileName = oscl_strlen(iTempOutputFileName) - oscl_strlen(oscl_strchr(iTempOutputFileName, '.'));
        oscl_wchar temp[ARRAY_SIZE];
        oscl_memset(temp, 0, ARRAY_SIZE);
        oscl_strncpy(temp, iTempOutputFileName, aLenFileName);
        oscl_snprintf(temp + aLenFileName, ARRAY_SIZE, _STRLIT("_GenericReset_TestNum_%d"), iTestCaseNum);
        oscl_strcat(temp, oscl_strchr(iTempOutputFileName, '.'));
        iOutputFileName = temp;
    }

    if (!ConfigAmrAacComposer())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::ConfigComposer: Error - ConfigAmrAacComposer failed"));
        return false;
    }

    //Not needed we pick up the output filename from cmd line

    if (!ConfigMp43gpComposer())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::ConfigComposer: Error - ConfigMp43gpComposer failed"));
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_test_generic_reset::ConfigAmrAacComposer()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_test_generic_reset::ConfigAmrAacComposer"));


    if (!((iComposerMimeType == KAMRNbComposerMimeType) || (iComposerMimeType == KAACADTSComposerMimeType) || (iComposerMimeType == KAACADIFComposerMimeType)))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "pvauthor_async_test_generic_reset::ConfigAmrAacComposer: AMR-AAC Composer not used in this test case"));
        return true;
    }


    PvmfFileOutputNodeConfigInterface* clipConfig = OSCL_STATIC_CAST(PvmfFileOutputNodeConfigInterface*, iComposerConfig);
    if (!clipConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::ConfigAmrAacComposer: Error - Invalid iComposerConfig"));
        return false;
    }

    if (clipConfig->SetOutputFileName(iOutputFileName) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::ConfigAmrAacComposer: Error - SetOutputFileName failed"));
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_test_generic_reset::ConfigMp43gpComposer()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                    (0, "pvauthor_async_test_generic_reset::ConfigMp43gpComposer"));

    if (!(iComposerMimeType == KAMRNbComposerMimeType) && !(iComposerMimeType == KAACADTSComposerMimeType) && !(iComposerMimeType == KAACADIFComposerMimeType))
    {
        //break;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "pvauthor_async_test_generic_reset::ConfigMp43gpComposer: Mp4-3GPP composer not used in this test case"));
        return true;
    }

    PVMp4FFCNClipConfigInterface* clipConfig;
    clipConfig = OSCL_STATIC_CAST(PVMp4FFCNClipConfigInterface*, iComposerConfig);
    if (!clipConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::ConfigMp43gpComposer: Error - iComposerConfig==NULL"));
        return false;
    }

    iVersionString = _STRLIT("version");
    iTitleString = _STRLIT("title");
    iAuthorString = _STRLIT("author");
    iCopyrightString = _STRLIT("copyright");
    iDescriptionString = _STRLIT("description");
    iRatingString = _STRLIT("rating");

    clipConfig->SetOutputFileName(iOutputFileName);
    clipConfig->SetPresentationTimescale(1000);
    clipConfig->SetVersion(iVersionString);
    clipConfig->SetTitle(iTitleString);
    clipConfig->SetAuthor(iAuthorString);
    clipConfig->SetCopyright(iCopyrightString);
    clipConfig->SetDescription(iDescriptionString);
    clipConfig->SetRating(iRatingString);

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_test_generic_reset::ConfigComposerOutput()
{
    PvmfComposerSizeAndDurationInterface* config =
        OSCL_REINTERPRET_CAST(PvmfComposerSizeAndDurationInterface*, iOutputSizeAndDurationConfig);
    if (!config)
        return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_test_generic_reset::QueryComposerOutputInterface()
{
    switch (iTestCaseNum)
    {
        case Generic_QueryInterface_Reset_Test:
            iAuthor->QueryInterface(PvmfComposerSizeAndDurationUuid,
                                    iOutputSizeAndDurationConfig, (OsclAny*)iAuthor);
            return true;

        default:
            return false;
    }
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_test_generic_reset::AddAudioMediaTrack()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_test_generic_reset::AddAudioMediaTrack"));

    if (iAudioInputType == INVALID_INPUT_TYPE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::AddAudioMediaTrackL: Error - Invalid audio input type"));
        return false;
    }

    bool testInputFound = false;
    PVAETestInput testInput;
    for (uint32 ii = 0; ii < iTestInputs.size(); ii++)
    {
        if (iTestInputs[ii].iType == iAudioInputType)
        {
            testInputFound = true;
            testInput = iTestInputs[ii];
            break;
        }
    }

    if (!testInputFound)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::AddAudioMediaTrack: Error - Test input not found"));
        return false;
    }

    iAuthor->AddMediaTrack(*(testInput.iNode), iAudioEncoderMimeType, iComposer,
                           iAudioEncoderConfig, (OsclAny*)iAuthor);
    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_test_generic_reset::AddVideoMediaTrack()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_test_generic_reset::AddVideoMediaTrack"));

    if (iVideoInputType == INVALID_INPUT_TYPE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::AddVideoMediaTrack: Error - Invalid input type"));
        return false;
    }

    bool testInputFound = false;
    PVAETestInput testInput;

    for (uint32 ii = 0; ii < iTestInputs.size(); ii++)
    {
        if (iTestInputs[ii].iType == iVideoInputType)
        {
            testInputFound = true;
            testInput = iTestInputs[ii];
            break;
        }
    }

    if (!testInputFound)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::AddVideoMediaTrack: Error - Test input not found"));
        return false;
    }

    iAuthor->AddMediaTrack(*(testInput.iNode), iVideoEncoderMimeType, iComposer,
                           iVideoEncoderConfig, (OsclAny*)iAuthor);
    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_test_generic_reset::AddTextMediaTrack()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_test_generic_reset::AddTextMediaTrack"));

    if (iTextInputType == INVALID_INPUT_TYPE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::AddTextMediaTrack: Error - Invalid input type"));
        return false;
    }

    bool testInputFound = false;
    PVAETestInput testInput;

    for (uint32 ii = 0; ii < iTestInputs.size(); ii++)
    {
        if (iTestInputs[ii].iType == iTextInputType)
        {
            testInputFound = true;
            testInput = iTestInputs[ii];
            break;
        }
    }

    if (!testInputFound)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "pvauthor_async_test_generic_reset::AddTextMediaTrack: Error - Test input not found"));
        return false;
    }

    iAuthor->AddMediaTrack(*(testInput.iNode), iTextEncoderMimeType, iComposer,
                           iTextEncoderConfig, (OsclAny*)iAuthor);
    return true;
}
bool pvauthor_async_test_generic_reset::ConfigureVideoEncoder()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_test_generic_reset::ConfigureVideoEncoder"));

    PVMp4H263EncExtensionInterface* config;
    config = OSCL_STATIC_CAST(PVMp4H263EncExtensionInterface*, iVideoEncoderConfig);
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "pvauthor_async_test_generic_reset::ConfigureVideoEncoder: No configuration needed"));
        return true;
    }

    config->SetNumLayers(1);
    config->SetOutputBitRate(0, KVideoBitrate);
    config->SetOutputFrameSize(0, iAVTConfig.iWidth , iAVTConfig.iHeight);
    config->SetOutputFrameRate(0, iAVTConfig.iFps);
    config->SetIFrameInterval(iAVTConfig.iFrameInterval);

    return true;
}

////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_test_generic_reset::ConfigureAudioEncoder()
{
    // Single core AMR encoder node support
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_test_generic_reset::ConfigureAudioEncoder"));

    if (!iAudioEncoderConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "pvauthor_async_test_generic_reset::ConfigureAudioEncoder: No configuration needed"));
        return true;
    }

    return PVAETestNodeConfig::ConfigureAudioEncoder(iAudioEncoderConfig);
}

bool pvauthor_async_test_generic_reset::ConfigureTextEncoder()
{
    // Single core AMR encoder node support
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_test_generic_reset::ConfigureAudioEncoder"));

    if (!iTextEncoderConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "pvauthor_async_test_generic_reset::ConfigureTextEncoder: No configuration needed"));
        return true;
    }

    return true;
}
////////////////////////////////////////////////////////////////////////////
bool pvauthor_async_test_generic_reset::DeleteTestInputs()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_test_generic_reset::DeleteTestInputs"));

    for (uint32 ii = 0; ii < iTestInputs.size(); ii++)
        iTestInputs[ii].DeleteInputNode();

    iTestInputs.clear();
    return true;
}

void pvauthor_async_test_generic_reset::ResetAuthorConfig()
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
    if (iTextEncoderConfig)
    {
        iTextEncoderConfig->removeRef();
        iTextEncoderConfig = NULL;
    }
}

void pvauthor_async_test_generic_reset::Cleanup()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "pvauthor_async_test_generic_reset::Cleanup"));

    iComposer = NULL;

    ResetAuthorConfig();

    if (iAuthor)
    {
        PVAuthorEngineFactory::DeleteAuthor(iAuthor);
        iAuthor = NULL;
    }

    DeleteTestInputs();
    iOutputFileName = NULL;
}


////////////////////////////////////////////////////////////////////////////
void pvauthor_async_test_generic_reset::Run()
{
    switch (iState)
    {
        case PVAE_CMD_CREATE:
        {
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
            iAuthor->Open((OsclAny*)iAuthor);
            break;

        case PVAE_CMD_ADD_DATA_SOURCE_AUDIO:
        {
            int aStatus = -1;
            // Create audio input
            aStatus = CreateAudioInput();
            if (aStatus == 0) //Failed while creating audio input
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pvauthor_async_test_generic_reset::CreateTestInputs: Error - CreateAudioInput() failed"));
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else if (aStatus == -1) //Failed due to test being audio only or video only
            {
                iState = PVAE_CMD_ADD_DATA_SOURCE_VIDEO;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_ADD_DATA_SOURCE_VIDEO:
        {
            // Create video input
            int aStatus = -1;
            aStatus = CreateVideoInput();
            if (aStatus == 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pvauthor_async_test_generic_reset::CreateTestInputs: Error - CreateVideoInput() failed"));
                Cleanup();
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else if (aStatus == -1) //Failed due to test being audio only or video only
            {
                iState = PVAE_CMD_ADD_DATA_SOURCE_TEXT;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_ADD_DATA_SOURCE_TEXT:
        {
            // Create text input
            int aStatus = -1;
            aStatus = CreateTextInput();
            if (aStatus == 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pvauthor_async_test_generic_reset::CreateTestInputs: Error - CreateTextInput() failed"));
                Cleanup();
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else if (aStatus == -1) //Failed due to test being audio only or video only
            {
                iState = PVAE_CMD_SELECT_COMPOSER;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_SELECT_COMPOSER:
            SelectComposer();
            break;

        case PVAE_CMD_QUERY_INTERFACE:
            if (!QueryComposerOutputInterface())
            {
                iState = PVAE_CMD_ADD_AUDIO_MEDIA_TRACK;
                RunIfNotReady();
            }
            break;

        case PVAE_CMD_ADD_AUDIO_MEDIA_TRACK:
            if (!AddAudioMediaTrack())
            {
                bAudioTrack = false;
                iState = PVAE_CMD_ADD_VIDEO_MEDIA_TRACK;
                RunIfNotReady();
            }
            break;

        case PVAE_CMD_ADD_VIDEO_MEDIA_TRACK:
        {
            bool bVideoTrack = AddVideoMediaTrack();
            if (!bVideoTrack && !bAudioTrack) //No tracks have been added
            {
                bVideoTrack = false;
                iState = PVAE_CMD_ADD_TEXT_MEDIA_TRACK;
                RunIfNotReady();
            }
            else if (!bVideoTrack) //Audio track added but no video track
            {
                iState = PVAE_CMD_ADD_TEXT_MEDIA_TRACK;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_ADD_TEXT_MEDIA_TRACK:
        {
            bool bTextTrack = AddTextMediaTrack();
            if (!bTextTrack && !bVideoTrack && !bAudioTrack) //No tracks have been added
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "pvauthor_async_test_generic_reset::CommandCompleted: Error - No track added"));
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_CLEANUPANDCOMPLETE;
                RunIfNotReady();
            }
            else if (!bTextTrack) //Audio and video tracks added but no text track
            {
                iState = PVAE_CMD_INIT;
                RunIfNotReady();
            }
        }
        break;

        case PVAE_CMD_INIT:
            iAuthor->Init((OsclAny*)iAuthor);
            break;

        case PVAE_CMD_START:
            iAuthor->Start();
            break;
        case PVAE_CMD_STOP:
            iAuthor->Stop((OsclAny*)iAuthor);
            break;
        case PVAE_CMD_RESET:
        {
            ResetAuthorConfig();
            iAuthor->Reset((OsclAny*)iAuthor);
        }
        break;
        case PVAE_CMD_REMOVE_DATA_SOURCE:
        {
            for (uint ii = 0; ii < iTestInputs.size(); ii++)
            {
                iAuthor->RemoveDataSource(*(iTestInputs[ii].iNode), (OsclAny*)iAuthor);
            }
        }
        break;
        case PVAE_CMD_CLOSE:
            iAuthor->Close((OsclAny*)iAuthor);
            break;
        case PVAE_CMD_PAUSE:
            iAuthor->Pause((OsclAny*)iAuthor);
            break;
        case PVAE_CMD_RESUME:
            iAuthor->Resume((OsclAny*)iAuthor);
            break;
        case PVAE_CMD_CLEANUPANDCOMPLETE:
        {
            Cleanup();
            iObserver->CompleteTest(*iTestCase);
        }
        break;

        case PVAE_CMD_QUERY_INTERFACE2:
            break;
        case PVAE_CMD_ADD_DATA_SINK:
            break;
        case PVAE_CMD_REMOVE_DATA_SINK:
            break;
        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
void pvauthor_async_test_generic_reset::CommandCompleted(const PVCmdResponse& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "pvauthor_async_test_generic_reset::CommandCompleted iState:%d", iState));

    if (aResponse.GetCmdStatus() != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "pvauthor_async_test_generic_reset::CommandCompleted iState:%d FAILED", iState));
    }
    switch (iState)
    {
        case PVAE_CMD_OPEN:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iState == iStateToReset)
                {
                    iState = PVAE_CMD_RESET;
                    RunIfNotReady();
                }
                else
                {
                    iState = PVAE_CMD_ADD_DATA_SOURCE_AUDIO;
                    RunIfNotReady();
                }
            }
            else
            {
                // Open failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_ADD_DATA_SOURCE_AUDIO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iState == iStateToReset)
                {
                    iState = PVAE_CMD_RESET;
                    RunIfNotReady();
                }
                else
                {
                    iState = PVAE_CMD_ADD_DATA_SOURCE_VIDEO;
                    RunIfNotReady();
                }
            }
            else
            {
                // AddDataSourceAudio failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_ADD_DATA_SOURCE_VIDEO:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iState == iStateToReset)
                {
                    iState = PVAE_CMD_RESET;
                    RunIfNotReady();
                }
                else
                {
                    iState = PVAE_CMD_ADD_DATA_SOURCE_TEXT;
                    RunIfNotReady();
                }
            }
            else
            {
                // AddDataSourceVideo failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_ADD_DATA_SOURCE_TEXT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iState == iStateToReset)
                {
                    iState = PVAE_CMD_RESET;
                    RunIfNotReady();
                }
                else
                {
                    iState = PVAE_CMD_SELECT_COMPOSER;
                    RunIfNotReady();
                }
            }
            else
            {
                // AddDataSourceText failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_SELECT_COMPOSER:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iComposer = aResponse.GetResponseData();
                if (!ConfigComposer())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "pvauthor_async_test_generic_reset::CommandCompleted: Error - ConfigComposer failed"));
                    PVPATB_TEST_IS_TRUE(false);
                    iObserver->CompleteTest(*iTestCase);
                    return;
                }
                else
                {
                    if (iState == iStateToReset)
                    {
                        iState = PVAE_CMD_RESET;
                        RunIfNotReady();
                    }
                    else
                    {
                        iState = PVAE_CMD_QUERY_INTERFACE;
                        RunIfNotReady();
                    }
                }
            }
            else
            {
                // SelectComposer failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_QUERY_INTERFACE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                ConfigComposerOutput();
                if (iState == iStateToReset)
                {
                    iState = PVAE_CMD_RESET;
                    RunIfNotReady();
                }
                else
                {
                    iState = PVAE_CMD_ADD_AUDIO_MEDIA_TRACK;
                    RunIfNotReady();
                }
            }
            else
            {
                // QueryInterface failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_ADD_AUDIO_MEDIA_TRACK:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (!ConfigureAudioEncoder())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "pvauthor_async_test_generic_reset::CommandCompleted: Error - ConfigureAudioEncoder failed"));
                    PVPATB_TEST_IS_TRUE(false);
                    iState = PVAE_CMD_CLOSE;
                    RunIfNotReady();
                }
                else
                {
                    if (iState == iStateToReset)
                    {
                        iState = PVAE_CMD_RESET;
                        RunIfNotReady();
                    }
                    else
                    {
                        iState = PVAE_CMD_ADD_VIDEO_MEDIA_TRACK;
                        RunIfNotReady();
                    }
                }
            }
            else
            {
                // AddAudioMediaTrack failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;

        case PVAE_CMD_ADD_VIDEO_MEDIA_TRACK:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (!ConfigureVideoEncoder())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "pvauthor_async_test_generic_reset::CommandCompleted: Error - ConfigureVideoEncoder failed"));
                    PVPATB_TEST_IS_TRUE(false);
                    iObserver->CompleteTest(*iTestCase);
                }
                else
                {
                    if (iState == iStateToReset)
                    {
                        iState = PVAE_CMD_RESET;
                        RunIfNotReady();
                    }
                    else
                    {
                        iState = PVAE_CMD_ADD_TEXT_MEDIA_TRACK;
                        RunIfNotReady();
                    }
                }
            }
            else
            {
                // AddVideoMediaTrack failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;

        case PVAE_CMD_ADD_TEXT_MEDIA_TRACK:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (!ConfigureTextEncoder())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "pvauthor_async_test_miscellaneous::CommandCompleted: Error - ConfigureTextEncoder failed"));
                    PVPATB_TEST_IS_TRUE(false);
                    iObserver->CompleteTest(*iTestCase);
                }
                else
                {
                    if (iState == iStateToReset)
                    {
                        iState = PVAE_CMD_RESET;
                        RunIfNotReady();
                    }
                    else
                    {
                        iState = PVAE_CMD_INIT;
                        RunIfNotReady();
                    }
                }
            }
            else
            {
                // AddTextMediaTrack failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iState == iStateToReset)
                {
                    iState = PVAE_CMD_RESET;
                    RunIfNotReady();
                }
                else
                {
                    iState = PVAE_CMD_START;
                    RunIfNotReady();
                }
            }
            else
            {
                // Init failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;

        case PVAE_CMD_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iPauseResumeEnable)
                {
                    if (iState == iStateToReset)
                    {
                        iState = PVAE_CMD_RESET;
                        RunIfNotReady();
                    }
                    else
                    {
                        iState = PVAE_CMD_PAUSE;
                        RunIfNotReady(KPauseDuration); //Pause after 5 sec
                    }
                }
                else
                {
                    if (iState == iStateToReset)
                    {
                        iState = PVAE_CMD_RESET;
                        RunIfNotReady();
                    }
                    else
                    {
                        iState = PVAE_CMD_STOP;
                        RunIfNotReady(KTestDuration*1000*1000); // Run for 10 seconds
                    }
                }
            }
            else
            {
                // Start failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_PAUSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iState == iStateToReset)
                {
                    iState = PVAE_CMD_RESET;
                    RunIfNotReady();
                }
                else
                {
                    iState = PVAE_CMD_RESUME;
                    /* Stay paused for 10 seconds */
                    RunIfNotReady(10*1000*1000);
                }
            }
            else
            {
                //Pause failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iState == iStateToReset)
                {
                    iState = PVAE_CMD_RESET;
                    RunIfNotReady();
                }
                else
                {
                    iState = PVAE_CMD_STOP;
                    RunIfNotReady(KPauseDuration); //Run for another 5 sec
                }
            }
            else
            {
                //Resume failed
                PVPATB_TEST_IS_TRUE(false);
                iState = PVAE_CMD_RESET;
                RunIfNotReady();
            }
            break;
        case PVAE_CMD_STOP:
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
            break;
        case PVAE_CMD_RESET:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iTestInputs.size() == 0)
                {
                    if (aResponse.GetCmdStatus() == PVMFSuccess)
                    {
                        PVPATB_TEST_IS_TRUE(true);
                    }
                    else
                    {
                        PVPATB_TEST_IS_TRUE(false);
                    }
                    //Since there are no testInputs, we end here
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
            break;

        case PVAE_CMD_REMOVE_DATA_SOURCE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iRemoveDataSourceDone++;
                if (iRemoveDataSourceDone < iTestInputs.size())
                {
                    return;//We will wait for all RemoveDataSource calls to complete
                }
                iOutputFileName = NULL;
                if (!DeleteTestInputs())
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "pvauthor_async_test_generic_reset::CommandCompleted: Error - DeleteTestInputs failed"));
                    PVPATB_TEST_IS_TRUE(false);
                    iObserver->CompleteTest(*iTestCase);
                    return;
                }
                else
                {
                    iState = PVAE_CMD_CLOSE;
                    RunIfNotReady();
                }
            }
            else
            {
                // RemoveDataSource failed
                PVPATB_TEST_IS_TRUE(false);
                iObserver->CompleteTest(*iTestCase);
            }
            break;
        case PVAE_CMD_CLOSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                PVPATB_TEST_IS_TRUE(true);
            }
            else
            {
                PVPATB_TEST_IS_TRUE(false);
            }
            iObserver->CompleteTest(*iTestCase);
            break;
        default:
        {
            // Testing error if this is reached
            PVPATB_TEST_IS_TRUE(false);
            iObserver->CompleteTest(*iTestCase);
        }
    }
}
