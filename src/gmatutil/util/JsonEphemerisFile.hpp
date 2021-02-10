//------------------------------------------------------------------------------
//                                  JsonEphemerisFile
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
#ifndef JsonEphemerisFile_hpp
#define JsonEphemerisFile_hpp

#include <fstream>

#include "json11.hpp"

#include "utildefs.hpp"
#include "Ephemeris.hpp"
#include "Rvector6.hpp"
#include "TimeSystemConverter.hpp"   // for TimeSystemConverter

/*
{
    "version"           : "json.1.0",
    "mission_id"        : "mission name",
    "satellite_id"      : "satellite_id_1",

    "reference_frame"   : "EarthMJ2000Eq",
    "epoch_format"      : "UTCGregorian",

    "central_body"      : "Earth", 
    "coordinate_system" : "Cartesian",
    "distance_unit",    : "meter",
    
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

class GMATUTIL_API JsonEphemerisFile : public Ephemeris
{
public:
   /// class methods
   JsonEphemerisFile();
   JsonEphemerisFile(const JsonEphemerisFile &copy);
   JsonEphemerisFile& operator=(const JsonEphemerisFile &copy);
   virtual ~JsonEphemerisFile();

   
   struct GMATUTIL_API EphemData
   {
      Real timeFromEpoch;
      Real theState[6];
   };

   void InitializeData(); // Move to protected?
   
   /// Open the ephemeris file for reading/writing
   bool OpenForRead(const std::string &filename, const std::string &ephemType);
      //   const std::string &ephemCovType = "");
   bool OpenForWrite(const std::string &filename, const std::string &ephemType);
                     // const std::string &ephemCovType = "");
   void CloseForRead();
   void CloseForWrite();

   bool ReadDataRecords(int logOption = 0);
   void GetStartAndEndEpochs(GmatEpoch &startEpoch, GmatEpoch &endEpoch,
                           std::vector<EphemData> **records);

   // For ephemeris file reading
   bool GetInitialAndFinalStates(Real &initialA1Mjd, Real &finalA1Mjd,
                                 Rvector6 &initialState, Rvector6 &finalState,
                                 std::string &cbName, std::string &csName);
   
   // For ephemeris file writing
   void SetVersion(const std::string &version);
   void SetInterpolationOrder(Integer order);
   bool SetHeaderForWriting(const std::string &fieldName,
                            const std::string &value);
   
   bool WriteHeader();
   // ajfg
   //bool WriteBlankLine();
   //bool WriteString(const std::string &str);
   bool WriteDataSegment(const EpochArray &epochArray, const StateArray &stateArray,
                         const std::vector<Rvector*> &covArray, bool canFinalize = false);
   void FinalizeEphemeris();
   
   std::string GetDistanceUnit();
   void SetDistanceUnit(const std::string &dU);

   bool GetIncludeEventBoundaries();
   void SetIncludeEventBoundaries(bool iEB);

   // MOVE TO Ephemeris base class!!!
   std::string GetCentralBody();

protected:

   bool        firstTimeWriting;
   bool        openForTempOutput;
   
   bool        includeEventBoundaries;
   bool        writeFinalized;

   Real        scenarioEpochA1Mjd;
   Real        coordinateSystemEpochA1Mjd;
   Real        beginSegmentTime;
   Real        lastEpochWritten;
   // If we need to compare full states, this field is a start
//   Rvector6  lastDataWritten;
   
   RealArray   beginSegmentArray;
   Integer     numberOfEphemPoints;
   Integer     numberOfCovPoints;
   Integer     interpolationOrder;
   
   // Header fields
   std::string    jsonVersion;
   std::string    satelliteId;
   std::string    missionId;
   std::string    referenceFrame;
   std::string    epochFormat;

   std::string scenarioEpochUtcGreg; // Required
   std::string interpolationMethod;
   std::string centralBody;
   std::string coordinateSystem;
   std::string coordinateSystemEpochStr;
   std::string distanceUnit;
   
   // The file name for read/write
   std::string jsonFileNameForRead;
   std::string jsonFileNameForWrite;
   // ajfg
   // std::string jsonTempFileName;
   // std::string jsonTempCovFileName;
   
   // Ephemeris type for read/write
   std::string ephemTypeForRead;
   std::string ephemTypeForWrite;
   std::string ephemCovTypeForWrite;
   // ajfg
   //bool writeCov;
   
   // File position for updating number of ephem points
   std::ofstream::pos_type numEphemPointsBegPos;
   
    // File input/output streams
   std::ifstream jsonInStream;
   std::ofstream jsonOutStream;
   // ajfg
   // No covariance
   // std::ifstream jsonCovInStream;
   // std::ofstream jsonCovOutStream;
   
   // Epoch and state buffer for read/write
   std::vector<EphemData> ephemRecords;
   
   // Initial/Final epochs and states read from file
   Real          initialSecsFromEpoch;
   Real          finalSecsFromEpoch;
   Rvector6      initialState;
   Rvector6      finalState;
   
   /// Time converter singleton
   TimeSystemConverter *theTimeConverter;

   // ajfg
   json11::Json::array   jsonEphemeris;

   // For ephemeris reading
   bool          GetEpochAndState(const std::string &line, Real &epoch, Rvector6 &state);
   // std::string   GetLastLine();
   // std::istream& IgnoreLine(std::ifstream::pos_type& pos);
   
   // For ephemeris writing
   void WriteTimePosVel(const EpochArray &epochArray, const StateArray &stateArray);
   void WriteTimePosVel(Real time, const Rvector6 *state);
   void WriteTimePos(const EpochArray &epochArray, const StateArray &stateArray);
   void WriteTimePos(Real time, const Rvector6 *state);

   // void WriteCovTimePosVel(const EpochArray &epochArray, const std::vector<Rvector*> &covArray);
   // void WriteCovTimePosVel(Real time, const Rvector *cov);
   // void WriteCovTimePos(const EpochArray &epochArray, const std::vector<Rvector*> &covArray);
   // void WriteCovTimePos(Real time, const Rvector *cov);
   
   // Time conversion
   std::string A1ModJulianToUtcGregorian(Real epochInDays, Integer format);
   bool        UTCGregorianToA1ModJulian(const std::string &utcGreg, Real &a1Mjd);
};

#endif // JsonEphemerisFile_hpp
