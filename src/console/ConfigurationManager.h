/**
 *  CubeGS
 *  An online Ground Segment for Cubesats and Small Sats
 *  (c) 2017 Incomplete Worlds
 *
 */

#ifndef __CONFIGURATIONMANAGER_H__
#define __CONFIGURATIONMANAGER_H__

#include <map>
#include <string>
#include <vector>

using namespace std;

#include "json11.hpp"


/**
 * Read a set of pair (key,value) from a JSON file
 * Store them in a map
 *
 */
class ConfigurationManager
{
    public:
        // Public keywords
        static const string GROUP_ZMQ;
        static const string GROUP_NNG;
        static const string GROUP_GENERAL;
		static const string GROUP_HTTP;

        // General
        static const string KEY_GEN_MODULE_NAME;
        static const string KEY_GEN_LOG_FILE;
        static const string KEY_MISSION_ID;

        // ZeroMQ
        static const string KEY_SERVER_ADDRESS;
        static const string KEY_PUB_ADDRESS;
        static const string KEY_SUB_ADDRESS;
        static const string KEY_RECV_TIMEOUT;

        // NANOMSG
        static const string KEY_BUS_ADDRESS;
        static const string KEY_EVENT_BUS_ADDRESS;
        static const string KEY_LOG_BUS_ADDRESS;
		
		// HTTP Server
	    static const string KEY_HTTP_SERVER_ADDRESS;
        
        // Only MCS
        static const string KEY_MCS_GS_MANAGER_SERVER_ADDRESS;
        static const string KEY_MCS_SERVER_ADDRESS;
        static const string KEY_MCS_ARCHIVE_DATABASE;

        // Only GS Manager
        static const string KEY_GS_MANAGER_DATABASE;
        static const string KEY_GS_TM_PROCESSOR_ADDRESS;

        // Only Tools
        static const string KEY_TOOLS_DATABASE;
        static const string KEY_TOOLS_SERVER_ADDRESS;
        
        // Only FDS
        static const string KEY_FDS_DATABASE;
        static const string KEY_FDS_SERVER_ADDRESS;
        static const string KEY_FDS_EXECUTABLE_NAME;
        
        // FDS Modules
        static const string KEY_FDS_MODULE_ADDRESS;
        static const string KEY_FDS_PUB_ADDRESS;
        
        // MCC 
        static const string KEY_MCC_SERVER_ADDRESS;
        


        // Singleton
        static ConfigurationManager& getInstance()
        {
            // Since it's a static variable, if the class has already been created,
            // It won't be created again.
            // And it **is** thread-safe in C++11.
            static ConfigurationManager instance;

            return instance;
        }

        virtual ~ConfigurationManager();

        // delete copy and move constructors and assign operators
        ConfigurationManager(ConfigurationManager const&) = delete;             // Copy construct
        ConfigurationManager(ConfigurationManager&&) = delete;                  // Move construct
        ConfigurationManager& operator=(ConfigurationManager const&) = delete;  // Copy assign
        ConfigurationManager& operator=(ConfigurationManager &&) = delete;      // Move assign

        // Operations
        void load(const string& inConfigFileName);

        // Getter and setters
        map<string, string> getKeyValues()
        {
            return keyValues;
        }
        void setKeyValues(map<string, string> val)
        {
            keyValues = val;
        }

        string getValue(const string &inKey);

        vector<string> getListValues(const string &inKey);


    protected:
        ConfigurationManager();

    private:
        map<string, string> keyValues;

        string mergeStrings(const vector<json11::Json> inStrings);
        vector<string> splitStrings(const string inString);
};

#endif // __CONFIGURATIONMANAGER_H__

