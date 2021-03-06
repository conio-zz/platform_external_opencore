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
#ifndef TEST_PV_PLAYER_ENGINE_TESTSET6_H_INCLUDED
#define TEST_PV_PLAYER_ENGINE_TESTSET6_H_INCLUDED

/**
 *  @file test_pv_player_engine_testset6.h
 *  @brief This file contains the class definitions for the sixth set of
 *         test cases for PVPlayerEngine - Streaming tests
 *
 */

#ifndef TEST_PV_PLAYER_ENGINE_H_INCLUDED
#include "test_pv_player_engine.h"
#endif

#ifndef TEST_PV_PLAYER_ENGINE_CONFIG_H_INCLUDED
#include "test_pv_player_engine_config.h"
#endif

#ifndef PVMF_STREAMING_DATA_SOURCE_H_INCLUDED
#include "pvmf_streaming_data_source.h"
#endif

#ifndef PVMF_SOURCE_CONTEXT_DATA_H_INCLUDED
#include "pvmf_source_context_data.h"
#endif


#define AMR_MPEG4_RTSP_URL "rtsp://test.3gp"
#define AMR_MPEG4_RTSP_URL_2 "rtsp://test.mp4"
#define H263_AMR_RTSP_URL "rtsp://test.3gp"
#define MPEG4_RTSP_URL "rtsp://test.3gp"
#define MPEG4_SHRT_HDR_RTSP_URL "rtsp://test.3gp"
#define AAC_RTSP_URL     "rtsp://test.3gp"
#define MPEG4_AAC_RTSP_URL "rtsp://test.3gp"
#define AMR_MPEG4_SDP_FILE "test.sdp"

class PVPlayerDataSourceURL;
class PVPlayerDataSink;
class PVPlayerDataSink;
class PVPlayerDataSinkFilename;
class PvmfFileOutputNodeConfigInterface;
class PvmiCapabilityAndConfig;
class PVPlayerDataSourcePacketSource;

/*!
 *  A test case to test the normal engine sequence of playing an rtsp url
 *  - Data Source: RTSP URL
 *  - Data Sink(s): Video[FileOutputNode-test_player_rtspurl_video.dat]\n
 *                  Audio[FileOutputNode-test_player_rtspurl_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 20 sec.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_streamingopenplaystop : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_streamingopenplaystop(PVPlayerAsyncTestParam aTestParam,
                PVMFFormatType aVideoSinkFormat,
                PVMFFormatType aAudioSinkFormat,
                uint32 aTestID,
                bool aPauseResumeEnable,
                bool aSeekEnable,
                bool aWaitForEOS,
                bool aCloaking,
                bool aCancelDuringPrepare,
                bool aForwardEnable)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iCancelAllCmdId(0)
                , iSeekAfterEOSEnable(false)
                , iSessionDuration(0)
                , bcloaking(aCloaking)
                , oLiveSession(false)
                , iProtocolRollOver(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iStreamDataSource(NULL)
                , iSourceContextData(NULL)
                , iPlayListSwitchMode(0)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oPauseResumeEnable = aPauseResumeEnable;
            oSeekEnable = aSeekEnable;
            oWaitForEOS = aWaitForEOS;
            oCancelDuringPrepare = aCancelDuringPrepare;
            oForwardEnable = aForwardEnable;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
            iNumPause = 0;
            iTargetNumPause = 1;
            iPauseDurationInMS = 10000; //10 seconds by default
        }

        ~pvplayer_async_test_streamingopenplaystop() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_QUERYINTERFACE,
            STATE_ADDDATASOURCE,
            STATE_CONFIGPARAMS,
            STATE_INIT,
            STATE_GETMETADATAKEYLIST,
            STATE_GETMETADATAVALUELIST,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_PREPARE,
            STATE_CANCELALL,
            STATE_WAIT_FOR_CANCELALL,
            STATE_START,
            STATE_SETPLAYBACKRANGE,
            STATE_PAUSE,
            STATE_RESUME,
            STATE_EOSNOTREACHED,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_WAIT_FOR_ERROR_HANDLING,
            STATE_CLEANUPANDCOMPLETE,
            STATE_SETFORWARD
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;
        PVCommandId iCancelAllCmdId;

        void setMultiplePlayMode(uint32 aNum)
        {
            iMultiplePlay = true;
            iTargetNumPlay = aNum;
        }

        void setMultiplePauseMode(uint32 aNum)
        {
            iMultiplePause = true;
            iTargetNumPause = aNum;
        }

        void setProtocolRollOverMode()
        {
            iProtocolRollOver = true;
        }

        void setProtocolRollOverModeWithUnknownURL()
        {
            iProtocolRollOverWithUnknownURLType = true;
        }

        void setPlayListMode()
        {
            iPlayListURL = true;
        }

        void setSeekAfterEOSMode()
        {
            iSeekAfterEOSEnable = true;
        }

        void setPauseDurationInMS(uint32 aDuration)
        {
            iPauseDurationInMS = aDuration;
        }

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr, OsclAny* aEventData);


        void PrintMetadataInfo();

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oPauseResumeEnable;
        bool oSeekEnable;
        bool oCancelDuringPrepare;
        bool oWaitForEOS;
        uint32 iTestID;
        bool iSeekAfterEOSEnable;
        bool oForwardEnable;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;

        bool iMultiplePause;
        int32 iNumPause;
        int32 iTargetNumPause;
        uint32 iPauseDurationInMS;

        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;
        int32 iPlayListSwitchMode;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<128> iKeyStringSetAsync;

        PVMFStreamingDataSource* iStreamDataSource;

        PVMFSourceContextData* iSourceContextData;
};


/*!
 *  A test case to test the normal engine sequence of playing an rtsp url
 *  - Data Source: RTSP URL
 *  - Data Sink(s): Video[FileOutputNode-test_player_rtspurl_video.dat]\n
 *                  Audio[FileOutputNode-test_player_rtspurl_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 20 sec.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_streamingJBadjust: public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_streamingJBadjust(PVPlayerAsyncTestParam aTestParam,
                                              PVMFFormatType aVideoSinkFormat,
                                              PVMFFormatType aAudioSinkFormat,
                                              uint32 aTestID,
                                              bool aPauseResumeEnable,
                                              bool aSeekEnable,
                                              bool aWaitForEOS)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iStreamDataSource(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oPauseResumeEnable = aPauseResumeEnable;
            oSeekEnable = aSeekEnable;
            oWaitForEOS = aWaitForEOS;
            iTestID = aTestID;
        }

        ~pvplayer_async_test_streamingJBadjust() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_QUERYINTERFACE,
            STATE_ADDDATASOURCE,
            STATE_CONFIGPARAMS,
            STATE_INIT,
            STATE_GETMETADATAKEYLIST,
            STATE_GETMETADATAVALUELIST,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_PREPARE,
            STATE_RECONFIGJITTERBUFFER,
            STATE_PREPARING,
            STATE_START,
            STATE_SETPLAYBACKRANGE,
            STATE_PAUSE,
            STATE_RESUME,
            STATE_EOSNOTREACHED,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_WAIT_FOR_ERROR_HANDLING,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oPauseResumeEnable;
        bool oSeekEnable;
        bool oWaitForEOS;
        uint32 iTestID;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;


        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<64> iKeyStringSetAsync;

        PVMFStreamingDataSource* iStreamDataSource;
};

/*!
 *  A test case to test bitstream switching
 *  - Data Source: RTSP URL, SDP File, MS HTTP URL
 *  - Data Sink(s): Video[FileOutputNode-test_player_rtspurl_video.dat]\n
 *                  Audio[FileOutputNode-test_player_rtspurl_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 20 sec.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_streaming_bitstream_switch: public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_streaming_bitstream_switch(PVPlayerAsyncTestParam aTestParam,
                PVMFFormatType aVideoSinkFormat,
                PVMFFormatType aAudioSinkFormat,
                uint32 aTestID,
                bool aWaitForEOS)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iStreamDataSource(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oPauseResumeEnable = false;
            oSeekEnable = false;
            oWaitForEOS = aWaitForEOS;
            iTestID = aTestID;
        }

        ~pvplayer_async_test_streaming_bitstream_switch() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_QUERYINTERFACE,
            STATE_ADDDATASOURCE,
            STATE_INIT,
            STATE_SET_STREAM_SPEED,
            STATE_GETMETADATAKEYLIST,
            STATE_GETMETADATAVALUELIST,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_PREPARE,
            STATE_START,
            STATE_SWITCH_STREAMS,
            STATE_SWITCH_STREAMS_DONE,
            STATE_SETPLAYBACKRANGE,
            STATE_PAUSE,
            STATE_RESUME,
            STATE_EOSNOTREACHED,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_WAIT_FOR_ERROR_HANDLING,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr);

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oPauseResumeEnable;
        bool oSeekEnable;
        bool oWaitForEOS;
        uint32 iTestID;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;

        void PrintMetadataInfo();

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<64> iKeyStringSetAsync;

        bool oLiveSession;
        uint32 iSessionDuration;

        PVMFStreamingDataSource* iStreamDataSource;
};

/*!
 *  A test case to test stop->prepare behavior when playing an rtsp url
 *  - Data Source: RTSP URL
 *  - Data Sink(s): Video[FileOutputNode-test_player_rtspurl_video.dat]\n
 *                  Audio[FileOutputNode-test_player_rtspurl_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 20 sec.
 *             -# Stop()
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 20 sec.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_streamingopenplaystoppreparelaystop : public pvplayer_async_test_base
{
    public:
        pvplayer_async_test_streamingopenplaystoppreparelaystop(PVPlayerAsyncTestParam aTestParam,
                PVMFFormatType aVideoSinkFormat,
                PVMFFormatType aAudioSinkFormat,
                uint32 aTestID,
                bool aPauseResumeEnable,
                bool aSeekEnable,
                bool aWaitForEOS,
                bool aCloaking,
                bool aCancelDuringPrepare,
                bool aForwardEnable)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iCancelAllCmdId(0)
                , iSeekAfterEOSEnable(false)
                , iSessionDuration(0)
                , bcloaking(aCloaking)
                , oLiveSession(false)
                , iProtocolRollOver(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iStreamDataSource(NULL)
                , iSourceContextData(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oPauseResumeEnable = aPauseResumeEnable;
            oSeekEnable = aSeekEnable;
            oWaitForEOS = aWaitForEOS;
            oCancelDuringPrepare = aCancelDuringPrepare;
            oForwardEnable = aForwardEnable;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
            iNumPause = 0;
            iTargetNumPause = 1;
        }

        ~pvplayer_async_test_streamingopenplaystoppreparelaystop() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_QUERYINTERFACE,
            STATE_ADDDATASOURCE,
            STATE_CONFIGPARAMS,
            STATE_INIT,
            STATE_GETMETADATAKEYLIST,
            STATE_GETMETADATAVALUELIST,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_PREPARE,
            STATE_CANCELALL,
            STATE_WAIT_FOR_CANCELALL,
            STATE_START,
            STATE_SETPLAYBACKRANGE,
            STATE_PAUSE,
            STATE_RESUME,
            STATE_EOSNOTREACHED,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_WAIT_FOR_ERROR_HANDLING,
            STATE_CLEANUPANDCOMPLETE,
            STATE_SETFORWARD,
            STATE_PREPARE1,
            STATE_START1,
            STATE_STOP1
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourceURL* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;
        PVCommandId iCancelAllCmdId;

        void setMultiplePlayMode(uint32 aNum)
        {
            iMultiplePlay = true;
            iTargetNumPlay = aNum;
        }

        void setMultiplePauseMode(uint32 aNum)
        {
            iMultiplePause = true;
            iTargetNumPause = aNum;
        }

        void setProtocolRollOverMode()
        {
            iProtocolRollOver = true;
        }

        void setProtocolRollOverModeWithUnknownURL()
        {
            iProtocolRollOverWithUnknownURLType = true;
        }

        void setPlayListMode()
        {
            iPlayListURL = true;
        }

        void setSeekAfterEOSMode()
        {
            iSeekAfterEOSEnable = true;
        }

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr);


        void PrintMetadataInfo();

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oPauseResumeEnable;
        bool oSeekEnable;
        bool oCancelDuringPrepare;
        bool oWaitForEOS;
        uint32 iTestID;
        bool iSeekAfterEOSEnable;
        bool oForwardEnable;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;

        bool iMultiplePause;
        int32 iNumPause;
        uint32 iTargetNumPause;

        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;

        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<128> iKeyStringSetAsync;

        PVMFStreamingDataSource* iStreamDataSource;

        PVMFSourceContextData* iSourceContextData;
};

/*!
 *  A test case to test the normal engine sequence of playing an sdp file
 *  that uses the RTP packet source node as a data source
 *  TO DO - UPDATE DESCRIPTION TO MATCH DVB TEST BEHAVIOR
 *  - Data Source: sdp file
 *  - Data Sink(s): Video[FileOutputNode-test_player_rtspurl_video.dat]\n
 *                  Audio[FileOutputNode-test_player_rtspurl_audio.dat]
 *  - Sequence:
 *             -# CreatePlayer()
 *             -# AddDataSource()
 *             -# Init()
 *             -# AddDataSink() (video)
 *             -# AddDataSink() (audio)
 *             -# Prepare()
 *             -# Start()
 *             -# WAIT 20 sec.
 *             -# Stop()
 *             -# RemoveDataSink() (video)
 *             -# RemoveDataSink() (audio)
 *             -# Reset()
 *             -# RemoveDataSource()
 *             -# DeletePlayer()
 *
 */
class pvplayer_async_test_dvbh_streamingopenplaystop : public pvplayer_async_test_base// , pvplayer_async_test_streamingopenplaystop
{
    public:
        pvplayer_async_test_dvbh_streamingopenplaystop(PVPlayerAsyncTestParam aTestParam,
                PVMFFormatType aVideoSinkFormat,
                PVMFFormatType aAudioSinkFormat,
                uint32 aTestID,
                bool aPauseResumeEnable,
                bool aSeekEnable,
                bool aWaitForEOS,
                bool aCloaking,
                bool aCancelDuringPrepare)
                : pvplayer_async_test_base(aTestParam)
                , iPlayer(NULL)
                , iDataSource(NULL)
                , iDataSinkVideo(NULL)
                , iDataSinkAudio(NULL)
                , iIONodeVideo(NULL)
                , iIONodeAudio(NULL)
                , iMIOFileOutVideo(NULL)
                , iMIOFileOutAudio(NULL)
                , iCurrentCmdId(0)
                , iCancelAllCmdId(0)
                , iSessionDuration(0)
                , bcloaking(aCloaking)
                , oLiveSession(false)
                , iProtocolRollOver(false)
                , iProtocolRollOverWithUnknownURLType(false)
                , iPlayListURL(false)
                , iStreamDataSource(NULL)
                , iSourceContextData(NULL)
        {
            iVideoSinkFormatType = aVideoSinkFormat;
            iAudioSinkFormatType = aAudioSinkFormat;
            oPauseResumeEnable = aPauseResumeEnable;
            oSeekEnable = aSeekEnable;
            oWaitForEOS = aWaitForEOS;
            oCancelDuringPrepare = aCancelDuringPrepare;
            iTestID = aTestID;
            iNumPlay = 0;
            iTargetNumPlay = 1;
        }

        ~pvplayer_async_test_dvbh_streamingopenplaystop() {}

        void StartTest();
        void Run();

        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PVTestState
        {
            STATE_CREATE,
            STATE_QUERYINTERFACE,
            STATE_ADDDATASOURCE,
            STATE_CONFIGPARAMS,
            STATE_INIT,
            STATE_GETMETADATAKEYLIST,
            STATE_GETMETADATAVALUELIST,
            STATE_ADDDATASINK_VIDEO,
            STATE_ADDDATASINK_AUDIO,
            STATE_PREPARE,
            STATE_CANCELALL,
            STATE_WAIT_FOR_CANCELALL,
            STATE_START,
            STATE_SETPLAYBACKRANGE,
            STATE_PAUSE,
            STATE_RESUME,
            STATE_EOSNOTREACHED,
            STATE_STOP,
            STATE_REMOVEDATASINK_VIDEO,
            STATE_REMOVEDATASINK_AUDIO,
            STATE_RESET,
            STATE_REMOVEDATASOURCE,
            STATE_WAIT_FOR_ERROR_HANDLING,
            STATE_CLEANUPANDCOMPLETE
        };

        PVTestState iState;

        PVPlayerInterface* iPlayer;
        PVPlayerDataSourcePacketSource* iDataSource;
        PVPlayerDataSink* iDataSinkVideo;
        PVPlayerDataSink* iDataSinkAudio;
        PVMFNodeInterface* iIONodeVideo;
        PVMFNodeInterface* iIONodeAudio;
        PvmiMIOControl* iMIOFileOutVideo;
        PvmiMIOControl* iMIOFileOutAudio;
        PVCommandId iCurrentCmdId;
        PVCommandId iCancelAllCmdId;

        void setMultiplePlayMode(uint32 aNum)
        {
            iMultiplePlay = true;
            iTargetNumPlay = aNum;
        }

        void setProtocolRollOverMode()
        {
            iProtocolRollOver = true;
        }

        void setProtocolRollOverModeWithUnknownURL()
        {
            iProtocolRollOverWithUnknownURLType = true;
        }

        void setPlayListMode()
        {
            iPlayListURL = true;
        }

    private:
        void HandleSocketNodeErrors(int32 aErr);
        void HandleRTSPNodeErrors(int32 aErr);
        void HandleStreamingManagerNodeErrors(int32 aErr);
        void HandleJitterBufferNodeErrors(int32 aErr);
        void HandleMediaLayerNodeErrors(int32 aErr);
        void HandleProtocolEngineNodeErrors(int32 aErr);


        void PrintMetadataInfo();

        PVMFFormatType iVideoSinkFormatType;
        PVMFFormatType iAudioSinkFormatType;
        OSCL_wHeapString<OsclMemAllocator> wFileName;
        oscl_wchar output[512];
        bool oPauseResumeEnable;
        bool oSeekEnable;
        bool oCancelDuringPrepare;
        bool oWaitForEOS;
        uint32 iTestID;

        PVPMetadataList iMetadataKeyList;
        Oscl_Vector<PvmiKvp, OsclMemAllocator> iMetadataValueList;
        int32 iNumValues;

        uint32 iSessionDuration;
        bool bcloaking;
        bool oLiveSession;

        bool iMultiplePlay;
        uint32 iNumPlay;
        uint32 iTargetNumPlay;

        bool iProtocolRollOver;
        bool iProtocolRollOverWithUnknownURLType;
        bool iPlayListURL;
        int32 iPriority;
        PvmiCapabilityAndConfig* iPlayerCapConfigIF;
        PvmiKvp* iErrorKVP;
        PvmiKvp iKVPSetAsync;
        OSCL_StackString<128> iKeyStringSetAsync;

        PVMFStreamingDataSource* iStreamDataSource;

        PVMFSourceContextData* iSourceContextData;
};



#endif // TEST_PV_PLAYER_ENGINE_TESTSET6_H_INCLUDED

