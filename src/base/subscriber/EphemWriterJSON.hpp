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
 * JSON format.
 */
//------------------------------------------------------------------------------
#ifndef EphemWriterJSON_hpp
#define EphemWriterJSON_hpp

#include "EphemWriterWithInterpolator.hpp"
#include "JsonEphemerisFile.hpp"


/*
{
    "version"           : "json.1.0",
    "mission_id"        : "mission name",
    "satellite_id"      : "satellite_id_1",

    "reference_frame"   : "EarthMJ2000Eq",
    "epoch_format"      : "UTCGregorian",
    
    "CentralBody"       : "Earth", 
    "CoordinateSystem"  : "Cartesian",
    "DistanceUnit",     : "meter",
    
    "ephemeris" :  [
        {
            "time"      : "2020-05-15T11:30:00.000",
            "position"  : [  0.0, 1.0, 2.0 ],
            "velocity"  : [  0.0, 1.0, 2.0 ]
        },

        {
            "time"      : "2020-05-15T11:30:05.000",
            "position"  : [  0.01, 1.02, 2.03 ],
            "velocity"  : [  0.01, 1.02, 2.03 ]
        }
    ] 
}
*/

class GMAT_API EphemWriterJSON : public EphemWriterWithInterpolator
{
public:
   EphemWriterJSON(const std::string &name, const std::string &type = "EphemWriterJSON");
   virtual ~EphemWriterJSON();
   EphemWriterJSON(const EphemWriterJSON &);
   EphemWriterJSON& operator=(const EphemWriterJSON&);
   
   virtual bool             Initialize();
   virtual EphemerisWriter* Clone(void) const;
   virtual void             Copy(const EphemerisWriter* orig);
   
   void         SetDistanceUnit(const std::string &dU);
   void         SetIncludeEventBoundaries(bool iEB);

protected:
   
   JsonEphemerisFile *jsonEphemFile; // owned object
   std::string        jsonVersion;
   bool               jsonWriteFailed;
   std::string        distanceUnit;
   bool               includeEventBoundaries;
   
   // Abstract methods required by all subclasses
   virtual void BufferOrbitData(Real epochInDays, const Real state[6], const Real cov[21]);
   
   // Initialization
   virtual void CreateEphemerisFile(bool useDefaultFileName,
                                    const std::string &stType,
                                    const std::string &outFormat,
                                    const std::string &covFormat);
   void         CreateJsonEphemerisFile();
   
   // Data
   virtual bool NeedToHandleBackwardProp();
   
   virtual void HandleOrbitData();
   virtual void StartNewSegment(const std::string &comments,
                                bool saveEpochInfo,
                                bool writeAfterData,
                                bool ignoreBlankComments);
   virtual void FinishUpWriting();
   virtual void CloseEphemerisFile(bool done = true, bool writeMetaData = true);
   
   void         HandleJsonOrbitData(bool writeDatda, bool timeToWrite);
   void         FinishUpWritingJson();
   
   // Json file writing
   void         WriteJsonOrbitDataSegment(bool canFinish);
   void         FinalizeJsonEphemeris();
};

#endif // EphemWriterJSON_hpp
