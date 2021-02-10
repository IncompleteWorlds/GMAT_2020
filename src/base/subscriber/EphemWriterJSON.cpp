//$Id$
//------------------------------------------------------------------------------
//                                  EphemWriterJSON
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002 - 2020 United States Government as represented by the
// Administrator of the National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); 
// You may not use this file except in compliance with the License. 
// You may obtain a copy of the License at:
// http://www.apache.org/licenses/LICENSE-2.0. 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
// express or implied.   See the License for the specific language
// governing permissions and limitations under the License.
//
// Author: ajfg
// Created: 2021.01.27
//
/**
 * Writes a spacecraft orbit states or attitude to an ephemeris file in
 * Json EphemerisTimePosVel format.
 */
//------------------------------------------------------------------------------

#include "EphemWriterJSON.hpp"
#include "SubscriberException.hpp"   // for exception
#include "MessageInterface.hpp"


//#define DEBUG_EPHEMFILE_INSTANCE
//#define DEBUG_EPHEMFILE_INIT
//#define DEBUG_EPHEMFILE_CREATE
//#define DEBUG_EPHEMFILE_JSON
//#define DEBUG_JSON_DATA_SEGMENT
//#define DEBUG_EPHEMFILE_BUFFER
//#define DEBUG_EPHEMFILE_WRITE
//#define DEBUG_EPHEMFILE_FINISH
//#define DEBUG_EPHEMFILE_RESTART
//#define DEBUG_EPHEMFILE_READBACK
//#define DEBUG_DISTANCEUNIT

//#ifndef DEBUG_MEMORY
//#define DEBUG_MEMORY
//#endif

#ifdef DEBUG_MEMORY
#include "MemoryTracker.hpp"
#endif

//---------------------------------
// static data
//---------------------------------


//------------------------------------------------------------------------------
// EphemWriterJSON(const std::string &name, const std::string &type = "EphemWriterJSON")
//------------------------------------------------------------------------------
/**
 * Default constructor
 */
//------------------------------------------------------------------------------
EphemWriterJSON::EphemWriterJSON(const std::string &name, const std::string &type) :
   EphemWriterWithInterpolator(name, type),
   jsonEphemFile     (NULL),
   jsonVersion       ("json.1.0"),
   jsonWriteFailed   (true)
{
   fileType = JSON_TYPE;
}


//------------------------------------------------------------------------------
// ~EphemWriterJSON()
//------------------------------------------------------------------------------
/**
 * Destructor
 */
//------------------------------------------------------------------------------
EphemWriterJSON::~EphemWriterJSON()
{
   #ifdef DEBUG_EPHEMFILE_INSTANCE
            MessageInterface::ShowMessage("EphemWriterJSON::~EphemWriterJSON() <%p>'%s' entered\n", 
                    this, GetName().c_str());
   #endif
      
   // Delete Json ephemeris
   if (jsonEphemFile)
      delete jsonEphemFile;
   
   #ifdef DEBUG_EPHEMFILE_INSTANCE
   MessageInterface::ShowMessage("EphemWriterJSON::~EphemWriterJSON() <%p>'%s' leaving\n", 
            this, GetName().c_str());
   #endif
}


//------------------------------------------------------------------------------
// EphemWriterJSON(const EphemWriterJSON &ef)
//------------------------------------------------------------------------------
/**
 * Copy constructor
 */
//------------------------------------------------------------------------------
EphemWriterJSON::EphemWriterJSON(const EphemWriterJSON &ef) :
   EphemWriterWithInterpolator(ef),
   jsonEphemFile     (NULL),
   jsonVersion       (ef.jsonVersion),
   jsonWriteFailed   (ef.jsonWriteFailed)
{
   coordConverter = ef.coordConverter;
}


//------------------------------------------------------------------------------
// EphemWriterJSON& EphemWriterJSON::operator=(const EphemWriterJSON& ef)
//------------------------------------------------------------------------------
/**
 * The assignment operator
 */
//------------------------------------------------------------------------------
EphemWriterJSON& EphemWriterJSON::operator=(const EphemWriterJSON& ef)
{
   if (this == &ef)
      return *this;
   
   EphemWriterWithInterpolator::operator=(ef);
   
   jsonEphemFile     = NULL;
   jsonVersion       = ef.jsonVersion;
   jsonWriteFailed   = ef.jsonWriteFailed;
   
   return *this;
}


//------------------------------------------------------------------------------
// virtual bool Initialize()
//------------------------------------------------------------------------------
bool EphemWriterJSON::Initialize()
{
    #ifdef DEBUG_EPHEMFILE_INIT
        MessageInterface::ShowMessage("EphemWriterJSON::Initialize() <%p>'%s' entered, spacecraftName='%s', "
            "isInitialized=%d\n   ephemType='%s', stateType='%s', "
            "outputFormat='%s'\n", this, ephemName.c_str(), spacecraftName.c_str(), 
            isInitialized, ephemType.c_str(), stateType.c_str(),
            outputFormat.c_str());
    #endif
    
    if (isInitialized)
    {
        #ifdef DEBUG_EPHEMFILE_INIT
            MessageInterface::ShowMessage("EphemWriterJSON::Initialize() <%p>'%s' is already initialized so just returning true\n",
                this, ephemName.c_str());
        #endif
        return true;
    }
    
    EphemWriterWithInterpolator::Initialize();
    
    // Set maximum segment size
    //maxSegmentSize = 1000;
    maxSegmentSize = 5000;
    
    // Check if interpolator needs to be created
    if (useFixedStepSize || interpolateInitialState || interpolateFinalState)
        createInterpolator = true;
    else
        createInterpolator = false;
    
    // Create interpolator if needed
    if (createInterpolator)
        CreateInterpolator();
    
    #ifdef DEBUG_EPHEMFILE_INIT
        MessageInterface::ShowMessage("EphemWriterJSON::Initialize() <%p>'%s' returning true, writeOrbit=%d, writeAttitude=%d, "
            "useFixedStepSize=%d,\n   writeDataInDataCS=%d, initialEpochA1Mjd=%.9f, "
            "finalEpochA1Mjd=%.9f, stepSizeInSecs=%.9f\n", this, ephemName.c_str(), writeOrbit,
            writeAttitude, useFixedStepSize, writeDataInDataCS, initialEpochA1Mjd, finalEpochA1Mjd,
            stepSizeInSecs);
    #endif
    
    return true;
}


//------------------------------------------------------------------------------
//  EphemerisWriter* Clone(void) const
//------------------------------------------------------------------------------
/**
 * This method returns a clone of the EphemWriterJSON.
 *
 * @return clone of the EphemWriterJSON.
 *
 */
//------------------------------------------------------------------------------
EphemerisWriter* EphemWriterJSON::Clone(void) const
{
   return (new EphemWriterJSON(*this));
}


//---------------------------------------------------------------------------
// void Copy(const EphemerisWriter* orig)
//---------------------------------------------------------------------------
/**
 * Sets this object to match another one.
 * 
 * @param orig The original that is being copied.
 */
//---------------------------------------------------------------------------
void EphemWriterJSON::Copy(const EphemerisWriter* orig)
{
   operator=(*((EphemWriterJSON *)(orig)));
}


//--------------------------------------
// protected methods
//--------------------------------------

//------------------------------------------------------------------------------
// void BufferOrbitData(Real epochInDays, const Real state[6], const Real cov[21])
//------------------------------------------------------------------------------
void EphemWriterJSON::BufferOrbitData(Real epochInDays, const Real state[6], const Real cov[21])
{
    #ifdef DEBUG_EPHEMFILE_BUFFER
            MessageInterface::ShowMessage("BufferOrbitData() <%p>'%s' entered, epochInDays=%.15f, state[0]=%.15f\n",
                this, ephemName.c_str(), epochInDays, state[0]);
            DebugWriteTime("   epochInDays = ", epochInDays, true, 2);
    #endif
    
    // if buffer is full, dump the data
    if (a1MjdArray.size() >= maxSegmentSize)
        WriteJsonOrbitDataSegment(false);
    
    // Add new data point
    A1Mjd *a1mjd = new A1Mjd(epochInDays);
    Rvector6 *rv6 = new Rvector6(state);
    Rvector *rv = new Rvector(21);
    rv->Set((Real *)cov, 21);
    a1MjdArray.push_back(a1mjd);
    stateArray.push_back(rv6);
    covArray.push_back(rv);
    
    #ifdef DEBUG_EPHEMFILE_BUFFER
        MessageInterface::ShowMessage("BufferOrbitData() <%p>'%s' leaving, there is(are) %d data point(s)\n",
            this, ephemName.c_str(), a1MjdArray.size());
    #endif
} // BufferOrbitData()


//------------------------------------------------------------------------------
// void CreateEphemerisFile(bool useDefaultFileName, const std::string &stType,
//                          const std::string &outFormat, const std::string &covFormat)
//------------------------------------------------------------------------------
/**
 * Creates ephemeris file writer.
 */
//------------------------------------------------------------------------------
void EphemWriterJSON::CreateEphemerisFile(bool useDefaultFileName,
                                         const std::string &stType,
                                         const std::string &outFormat,
                                         const std::string &covFormat)
{
   EphemerisWriter::CreateEphemerisFile(useDefaultFileName, stType, outFormat, covFormat);
   CreateJsonEphemerisFile();
   isEphemFileOpened = true;
}


//------------------------------------------------------------------------------
// void CreateJsonEphemerisFile()
//------------------------------------------------------------------------------
/**
 * Creates ephemeris file writer and sets Json file header values.
 */
//------------------------------------------------------------------------------
void EphemWriterJSON::CreateJsonEphemerisFile()
{
    #ifdef DEBUG_EPHEMFILE_CREATE
        MessageInterface::ShowMessage("\nEphemWriterJSON::CreateJsonEphemerisFile() this=<%p>'%s' entered, jsonEphemFile=<%p>,"
            " outputFormat='%s'\n", this, ephemName.c_str(), jsonEphemFile, outputFormat.c_str());
    #endif
    
    // If jsonEphemFile is not NULL, delete it first
    if (jsonEphemFile != NULL)
    {
        #ifdef DEBUG_MEMORY
            MemoryTracker::Instance()->Remove(jsonEphemFile, "jsonEphemFile", "EphemWriterJSON::CreateJsonEphemerisFile()",
                "deleting local jsonEphemFile");
        #endif
        delete jsonEphemFile;
        jsonEphemFile = NULL;
    }

    std::string jsonVersion {"json.1.0"};
    std::string missionId {""};
    std::string satelliteId {""};
    std::string referenceFrame {"MJ2000Eq"};
    std::string epochFormat{"UTCGregorian"};
    
    std::string axisTypeName = (outCoordSystem->GetAxisSystem())->GetTypeName(); 
    std::string coordBaseSystem = outCoordSystem->GetBaseSystem();
    
    // Figure out what cs type to write (ICRF, J2000, Fixed)
    std::string csTypeName = axisTypeName;
    if (axisTypeName == "MJ2000Eq")
        csTypeName = "J2000";
    else if (axisTypeName == "BodyFixed")
        csTypeName = "Fixed";

        #ifdef DEBUG_EPHEMFILE_CREATE
            MessageInterface::ShowMessage("   Creating JsonEphemerisFile with timeSystem='%s', centralBody='%s', "
                "coordSystemName='%s', axisTypeName='%s', csTypeName='%s', coordBaseSystem='%s'\n",
                timeSystem.c_str(), centralBody.c_str(), coordSystemName.c_str(), axisTypeName.c_str(),
                csTypeName.c_str(), coordBaseSystem.c_str());
        #endif
   
   std::string timeSystem  = "UTC";   // Figure out time system here
   std::string centralBody = outCoordSystem->GetOriginName();   
   std::string coordSystemName = outCoordSystem->GetName(); 

    // Determine what covariance to write
    std::string ephemCovType = "";

    if (covarianceFormat == "Position")
        ephemCovType = "TimePos";
    else if (covarianceFormat == "PositionAndVelocity")
        ephemCovType = "TimePosVel";
   
    try
    {
        jsonEphemFile = new JsonEphemerisFile;
        //std::string distanceUnit = jsonEphemFile->GetDistanceUnit();

        #ifdef DEBUG_DISTANCEUNIT
            MessageInterface::ShowMessage("   Retrieved the following distanceUnit: %s", 
                     distanceUnit.c_str());
        #endif

        if (jsonEphemFile->OpenForWrite(fullPathFileName, "TimePosVel"))
        {
            jsonEphemFile->SetVersion(jsonVersion);

            jsonEphemFile->SetHeaderForWriting("mission_id",       missionId);
            jsonEphemFile->SetHeaderForWriting("satellite_id",     satelliteId);
            jsonEphemFile->SetHeaderForWriting("reference_frame",  referenceFrame);
            jsonEphemFile->SetHeaderForWriting("epoch_format",     epochFormat);


            if (useFixedStepSize)
            {
                //jsonEphemFile->SetHeaderForWriting("InterpolationMethod", interpolatorName);
                jsonEphemFile->SetInterpolationOrder(interpolationOrder);
            }
            jsonEphemFile->SetHeaderForWriting("central_body",      centralBody);
            jsonEphemFile->SetHeaderForWriting("coordinate_system", csTypeName);
            jsonEphemFile->SetHeaderForWriting("distance_unit",     distanceUnit);

            jsonEphemFile->SetIncludeEventBoundaries(includeEventBoundaries);
        }
        else
        {
            SubscriberException se;
            se.SetDetails("Unable to open Json ephemeris file: '%s'\n", fullPathFileName.c_str());
            throw se;
        }
    }
    catch (BaseException &e)
    {
        // Keep from getting a compiler warning about e not being used
        e.GetMessageType();
        
        #ifdef DEBUG_EPHEMFILE_CREATE
            MessageInterface::ShowMessage("Error opening JsonEphemerisFile: %s", (e.GetFullMessage()).c_str());
        #endif
        throw;
    }
    
    #ifdef DEBUG_MEMORY
        MemoryTracker::Instance()->Add(jsonEphemFile, "jsonEphemFile", "EphemWriterJSON::CreateJsonEphemerisFile()",
            "jsonEphemFile = new SpiceOrbitKernelWriter()");
    #endif
    
    #ifdef DEBUG_EPHEMFILE_CREATE
        MessageInterface::ShowMessage("EphemWriterJSON::CreateJsonEphemerisFile() <%p>'%s' leaving, jsonEphemFile=<%p>\n",
            this, ephemName.c_str(), jsonEphemFile);
    #endif
}


//------------------------------------------------------------------------------
// bool NeedToHandleBackwardProp()
//------------------------------------------------------------------------------
/**
 * Checks if backward prop is allowed or don't need special handling.
 *
 * @return  false  if no special handling is needed
 *          true   if need to procced to next step
 */
//------------------------------------------------------------------------------
bool EphemWriterJSON::NeedToHandleBackwardProp()
{
    #if DBGLVL_EPHEMFILE_DATA
            MessageInterface::ShowMessage("EphemWriterJSON::NeedToHandleBackwardProp() entered\n");
    #endif
    
    // Throw an exception for Json as backward prop is not allowed
    throw SubscriberException(GetBackwardPropWarning());
}


//------------------------------------------------------------------------------
// void HandleOrbitData()
//------------------------------------------------------------------------------
/** Handles writing orbit data includes checking epoch to write if writing at
 * fixed step size.
 */
//------------------------------------------------------------------------------
void EphemWriterJSON::HandleOrbitData()
{
    // Check user defined initial and final epoch
    bool processData = CheckInitialAndFinalEpoch();
    
    #if DBGLVL_EPHEMFILE_DATA
        MessageInterface::ShowMessage("EphemWriterJSON::HandleOrbitData() checked initial and final epoch\n");
    #endif
    
    // Check if it is time to write
    bool timeToWrite = IsTimeToWrite(currEpochInSecs, currState, currCov);
    
    #if DBGLVL_EPHEMFILE_DATA > 0
        MessageInterface::ShowMessage("   Start writing data, currEpochInDays=%.15f, currEpochInSecs=%.15f, %s\n"
            "   writeOrbit=%d, writeAttitude=%d, processData=%d, timeToWrite=%d\n",
            currEpochInDays, currEpochInSecs, ToUtcGregorian(currEpochInSecs).c_str(),
            writeOrbit, writeAttitude, processData, timeToWrite);
    #endif
    
    // For now we only write Orbit data
    HandleJsonOrbitData(processData, timeToWrite);
}


//------------------------------------------------------------------------------
// void StartNewSegment(const std::string &comments, bool saveEpochInfo,
//                      bool writeAfterData, bool ignoreBlankComments)
//------------------------------------------------------------------------------
/**
 * Finishes writing remaining data and resets flags to start new segment.
 */
//------------------------------------------------------------------------------
void EphemWriterJSON::StartNewSegment(const std::string &comments,
                                         bool saveEpochInfo, bool writeAfterData,
                                         bool ignoreBlankComments)
{
    #ifdef DEBUG_EPHEMFILE_RESTART
        MessageInterface::ShowMessage("===== EphemWriterJSON::StartNewSegment() entered\n   comments='%s'\n   "
            "saveEpochInfo=%d, writeAfterData=%d, ignoreBlankComments=%d, canFinalize=%d, firstTimeWriting=%d\n",
            comments.c_str(), saveEpochInfo, writeAfterData, ignoreBlankComments, canFinalize, firstTimeWriting);
    #endif
    
    // If no first data has written out yet, just return
    if (firstTimeWriting)
    {
        #ifdef DEBUG_EPHEMFILE_RESTART
            MessageInterface::ShowMessage("EphemWriterJSON::StartNewSegment() returning, no first data written out yet\n");
            #endif
        return;
    }
    
    #ifdef DEBUG_EPHEMFILE_RESTART
        MessageInterface::ShowMessage("EphemWriterJSON::StartNewSegment() Calling FinishUpWriting(), canFinalize=%d\n",
            canFinalize);
    #endif
    
    // Write data for the rest of times on waiting
    FinishUpWriting();
    
    // Set comments
    writeCommentAfterData = writeAfterData;
    currComments = comments;
    
    #ifdef DEBUG_EPHEMFILE_RESTART
        WriteComments(comments, ignoreBlankComments);
    #endif
    
    if (jsonEphemFile != NULL)
        WriteJsonOrbitDataSegment(false);
    
    // Initialize data
    InitializeData(saveEpochInfo);
    
    #ifdef DEBUG_EPHEMFILE_RESTART
        MessageInterface::ShowMessage
            ("===== EphemWriterJSON::StartNewSegment() leaving\n");
    #endif
}


//------------------------------------------------------------------------------
// void FinishUpWriting()
//------------------------------------------------------------------------------
/*
 * Finishes up writing data at epochs on waiting.
 */
//------------------------------------------------------------------------------
void EphemWriterJSON::FinishUpWriting()
{
    #ifdef DEBUG_EPHEMFILE_FINISH
        MessageInterface::ShowMessage("EphemWriterJSON::FinishUpWriting() '%s' entered, canFinalize=%d, isFinalized=%d, "
            "firstTimeWriting=%d\n   interpolatorStatus=%d, isEndOfRun=%d\n",
            ephemName.c_str(), canFinalize, isFinalized, firstTimeWriting, interpolatorStatus,
            isEndOfRun);
        DebugWriteTime("    lastEpochWrote = ", lastEpochWrote);
        DebugWriteTime("   currEpochInSecs = ", currEpochInSecs);
        #ifdef DEBUG_EPHEMFILE_FINISH_MORE
            DebugWriteEpochsOnWaiting("   ");
        #endif
        MessageInterface::ShowMessage("   There is(are) %d data point(s) in the buffer\n", a1MjdArray.size());
    #endif
    
    if (!isFinalized)
    {
        #ifdef DEBUG_EPHEMFILE_FINISH
            MessageInterface::ShowMessage("   It is not finalized yet, so trying to write the remainder of data\n");
        #endif
        
        FinishUpWritingJson();
        
        if (canFinalize)
        {
            if (isEndOfRun)
            {
                #ifdef DEBUG_EPHEMFILE_FINISH
                    MessageInterface::ShowMessage("   It is end of run, so closing ephemeris file\n");
                #endif

                CloseEphemerisFile();
                
                // Check for user defined final epoch (GMT-4108 fix)
                if (finalEpochA1Mjd != -999.999)
                {
                    if (currEpochInDays < finalEpochA1Mjd)
                    {
                        MessageInterface::ShowMessage("*** WARNING *** Run ended at %f before the user defined final epoch of %f\n",
                            currEpochInDays, finalEpochA1Mjd);
                    }
                }
            }
            
            isFinalized = true;
        }
    }
    
    #ifdef DEBUG_EPHEMFILE_FINISH
        MessageInterface::ShowMessage("EphemWriterJSON::FinishUpWriting() leaving, isFinalized=%d\n", isFinalized);
    #endif
}


//------------------------------------------------------------------------------
// void CloseEphemerisFile(bool done = true, writeMetaData = true)
//------------------------------------------------------------------------------
void EphemWriterJSON::CloseEphemerisFile(bool done, bool writeMetaData)
{
    FinalizeJsonEphemeris();
}

//------------------------------------------------------------------------------
// void HandleJsonOrbitData(bool writeData, bool timeToWrite)
//------------------------------------------------------------------------------
void EphemWriterJSON::HandleJsonOrbitData(bool writeData, bool timeToWrite)
{
    #ifdef DEBUG_EPHEMFILE_JSON
    MessageInterface::ShowMessage("EphemWriterJSON::HandleJsonOrbitData() <%p>'%s' entered, writeData=%d, "
        "currEpochInDays = %.13lf, \n   firstTimeWriting=%d, writingNewSegment=%d\n",
        this, ephemName.c_str(), writeData, currEpochInDays, firstTimeWriting, writingNewSegment);
    #endif
    
    // LagrangeInterpolator's maximum buffer size is set to 80 which can hold
    // 80 min of data assuming average of 60 sec data interveval.
    // Check at least 10 min interval for large step size, since interpolater
    // buffer size is limited
    if (!timeToWrite)
    {
        if ((currEpochInSecs - prevProcTime) > 600.0)
        {
            #ifdef DEBUG_EPHEMFILE_JSON
            MessageInterface::ShowMessage
                ("   ==> 10 min interval is over, so setting timeToWrite to true\n");
            #endif
            
            timeToWrite = true;
        }
    }
    
    #ifdef DEBUG_EPHEMFILE_JSON
        MessageInterface::ShowMessage("   timeToWrite=%d, writingNewSegment=%d\n", timeToWrite, writingNewSegment);
    #endif
    
    if (timeToWrite)
        prevProcTime = currEpochInSecs;
    
    //------------------------------------------------------------
    // write data to file
    //------------------------------------------------------------
    // Now actually write data
    if (writeData && timeToWrite)
    {
        if (writingNewSegment)
        {
            //WriteComments("********** NEW SEGMENT **********");
            #ifdef DEBUG_EPHEMFILE_WRITE
                DebugWriteTime("********** WRITING NEW SEGMENT AT currEpochInSecs = ", currEpochInSecs, false, 2);
            #endif
            
            WriteJsonOrbitDataSegment(false);
        }
        
        if (writeOrbit)
            HandleWriteOrbit();
        
        if (firstTimeWriting)
            firstTimeWriting = false;
        
        if (writingNewSegment)
            writingNewSegment = false;
    }
    
    #ifdef DEBUG_EPHEMFILE_JSON
        MessageInterface::ShowMessage("EphemWriterJSON::HandleJsonOrbitData() leaving, firstTimeWriting=%d, writingNewSegment=%d\n",
            firstTimeWriting, writingNewSegment);
    #endif
}


//------------------------------------------------------------------------------
// void FinishUpWritingJson()
//------------------------------------------------------------------------------
/**
 * Writes final data segment.
 */
//------------------------------------------------------------------------------
void EphemWriterJSON::FinishUpWritingJson()
{
    #ifdef DEBUG_EPHEMFILE_FINISH
        MessageInterface::ShowMessage("EphemWriterJSON::FinishUpWritingJson() entered, canFinalize=%d\n", canFinalize);
    #endif
    
    if (interpolator != NULL && useFixedStepSize)
    {
        #ifdef DEBUG_EPHEMFILE_FINISH
        MessageInterface::ShowMessage("===> FinishUpWritingJson() checking for not enough data points\n"
            "   currEpochInSecs='%s'\n", ToUtcGregorian(currEpochInSecs).c_str());
        #endif
        
        // First check for not enough data points for interpolation
        if (canFinalize && interpolatorStatus == -1)
        {
            isFinalized = true;
            std::string ephemMsg, errMsg;
            FormatErrorMessage(ephemMsg, errMsg);
            throw SubscriberException(errMsg);
        }
        
        // Process final data on waiting to be output
        ProcessFinalDataOnWaiting();
    }
    
    // Write final data
    if (jsonEphemFile != NULL)
    {
        WriteJsonOrbitDataSegment(canFinalize);
    }
    else
    {
        if (a1MjdArray.size() > 0)
        {
            throw SubscriberException("*** INTERNAL ERROR *** jsonEphemFile is NULL in "
                "EphemWriterJSON::FinishUpWritingJson()\n");
        }
    }
    
    #ifdef DEBUG_EPHEMFILE_FINISH
        MessageInterface::ShowMessage("EphemWriterJSON::FinishUpWritingJson() leaving\n");
    #endif
}


//------------------------------------------------------------------------------
// void WriteJsonOrbitDataSegment(bool canFinish)
//------------------------------------------------------------------------------
/**
 * Writes orbit data segment to Json file and deletes data array
 */
//------------------------------------------------------------------------------
void EphemWriterJSON::WriteJsonOrbitDataSegment(bool canFinish)
{
    #ifdef DEBUG_JSON_DATA_SEGMENT
        MessageInterface::ShowMessage("=====> WriteJsonOrbitDataSegment() <%p>'%s' entered, canFinish=%d, "
            "isEndOfRun=%d, a1MjdArray.size()=%d, stateArray.size()=%d\n", this,
            ephemName.c_str(), canFinish, isEndOfRun, a1MjdArray.size(), stateArray.size());
    #endif
    
    if (a1MjdArray.size() > 0)
    {
        if (jsonEphemFile == NULL)
            throw SubscriberException("*** INTERNAL ERROR *** Json Ephem Writer is NULL in "
                "EphemWriterJSON::WriteJsonOrbitDataSegment()\n");
        
        #ifdef DEBUG_JSON_DATA_SEGMENT
            A1Mjd *start = a1MjdArray.front();
            A1Mjd *end   = a1MjdArray.back();
            MessageInterface::ShowMessage("   Writing start=%.15f, end=%.15f\n", start->GetReal(), end->GetReal());
            MessageInterface::ShowMessage("There are %d epochs and states:\n", a1MjdArray.size());
            #ifdef DEBUG_JSON_DATA_SEGMENT_MORE
                for (unsigned int ii = 0; ii < a1MjdArray.size(); ii++)
                {
                    A1Mjd *tm = a1MjdArray[ii];
                    Real time = tm->GetReal();
                    Rvector6 *sv = stateArray[ii];
                    std::string format = "% 1.15e  ";
                    MessageInterface::ShowMessage("[%3d] %12.10f  %s  %s\n", ii, time, ToUtcGregorian(time, true).c_str(),
                        (sv->ToString(format, 6)).c_str());
                }
            #endif
        #endif
        
        jsonWriteFailed = false;
        try
        {
            #ifdef DEBUG_JSON_DATA_SEGMENT
                MessageInterface::ShowMessage(".....Calling jsonEphemFile->WriteDataSegment() jsonEphemFile=<%p>, "
                    "isEndOfRun=%d, epochsOnWaiting.size()=%d\n", jsonEphemFile, isEndOfRun,
                    epochsOnWaiting.size());
                if (!epochsOnWaiting.empty())
                    DebugWriteTime("   ", epochsOnWaiting.back());
            #endif
            // Check if Json ephemeris file can be finalized (GMT-4060 fix)
            bool finalize = isEndOfRun && canFinish;
            jsonEphemFile->WriteDataSegment(a1MjdArray, stateArray, covArray, finalize);
            ClearOrbitData();
        }
        catch (BaseException &e)
        {
            ClearOrbitData();
            jsonWriteFailed = true;
            #ifdef DEBUG_JSON_DATA_SEGMENT
                MessageInterface::ShowMessage("**** ERROR **** " + e.GetFullMessage());
            #endif
            e.SetFatal(true);
            throw;
        }
    }
    
    #ifdef DEBUG_JSON_DATA_SEGMENT
        MessageInterface::ShowMessage("=====> WriteJsonOrbitDataSegment() leaving\n");
    #endif
}


//------------------------------------------------------------------------------
// FinalizeJsonEphemeris()
//------------------------------------------------------------------------------
void EphemWriterJSON::FinalizeJsonEphemeris()
{
    #ifdef DEBUG_EPHEMFILE_FINISH
        MessageInterface::ShowMessage("EphemWriterJSON::FinalizeJsonEphemeris() entered\n");
    #endif
    
    if (jsonEphemFile == NULL)
        throw SubscriberException("*** INTERNAL ERROR *** Json Ephem Writer is NULL in "
            "EphemWriterJSON::FinalizeJsonEphemeris()\n");
    
    // Write any final data
    jsonEphemFile->FinalizeEphemeris();
    
    #ifdef DEBUG_EPHEMFILE_READBACK
        if (isEndOfRun)
        {
            MessageInterface::ShowMessage
                ("===> EphemWriterJSON::FinalizeJsonEphemeris() calling jsonEphemFile "
                "for debug output\n   fullPathFileName = '%s'\n", fullPathFileName.c_str());
            jsonEphemFile->OpenForRead(fullPathFileName, 1, 1);
            //@todo Add more code to read data
            jsonEphemFile->CloseForRead();
        }
    #endif
    
    // Close Json ephemeris file
    jsonEphemFile->CloseForWrite();
    
    #ifdef DEBUG_EPHEMFILE_FINISH
        MessageInterface::ShowMessage("EphemWriterJSON::FinalizeJsonEphemeris() leaving\n");
    #endif
}

void EphemWriterJSON::SetDistanceUnit(const std::string &dU)
{
   distanceUnit = dU;
}

void EphemWriterJSON::SetIncludeEventBoundaries(bool iEB)
{
   includeEventBoundaries = iEB;
}
