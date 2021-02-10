/**
 *  CubeGS
 *  An online Ground Segment for Cubesats and Small Sats
 *  (c) 2017 Incomplete Worlds
 *
 */

#include <fstream>
#include <sstream>

using namespace std;

#include "GSException.h"

#include "LogManager.h"
#include "ConfigurationManager.h"

const string ConfigurationManager::GROUP_ZMQ = "ZMQ";
const string ConfigurationManager::GROUP_NNG = "NNG";
const string ConfigurationManager::GROUP_GENERAL = "General";
const string ConfigurationManager::GROUP_HTTP = "HTTP";

const string ConfigurationManager::KEY_GEN_MODULE_NAME = "module_name";
const string ConfigurationManager::KEY_GEN_LOG_FILE = "log_file";

//TODO: Rename to mission_name
const string ConfigurationManager::KEY_MISSION_ID = "mission_id";

const string ConfigurationManager::KEY_SERVER_ADDRESS = "server_address";
const string ConfigurationManager::KEY_PUB_ADDRESS = "publisher_address";
const string ConfigurationManager::KEY_SUB_ADDRESS = "subscriber_address";
const string ConfigurationManager::KEY_RECV_TIMEOUT = "receive_msg_timeout";

// Nanomsg Buses
const string ConfigurationManager::KEY_BUS_ADDRESS = "bus_address";
const string ConfigurationManager::KEY_EVENT_BUS_ADDRESS = "event_bus_address";
const string ConfigurationManager::KEY_LOG_BUS_ADDRESS = "log_bus_address";

// HTTP
const string ConfigurationManager::KEY_HTTP_SERVER_ADDRESS = "http_server_address";


// MCS
const string ConfigurationManager::KEY_MCS_GS_MANAGER_SERVER_ADDRESS = "gs_manager_server_address";
const string ConfigurationManager::KEY_MCS_SERVER_ADDRESS = "mcs_server_address";
const string ConfigurationManager::KEY_MCS_ARCHIVE_DATABASE = "archive_database_name";


// GS Manager
const string ConfigurationManager::KEY_GS_MANAGER_DATABASE = "gs_database_name";
const string ConfigurationManager::KEY_GS_TM_PROCESSOR_ADDRESS = "tm_processor_address";

// Tools
const string ConfigurationManager::KEY_TOOLS_DATABASE = "tools_database_name";
const string ConfigurationManager::KEY_TOOLS_SERVER_ADDRESS = "tools_server_address";

// FDS
const string ConfigurationManager::KEY_FDS_DATABASE = "fds_database_name";
const string ConfigurationManager::KEY_FDS_SERVER_ADDRESS = "fds_server_address";
const string ConfigurationManager::KEY_FDS_EXECUTABLE_NAME = "executable_name";

const string ConfigurationManager::KEY_FDS_MODULE_ADDRESS = "module_address";

const string ConfigurationManager::KEY_FDS_PUB_ADDRESS = "fds_publisher_address";

// MCC
const string ConfigurationManager::KEY_MCC_SERVER_ADDRESS = "mcc_server_address";


// Protected constructor
ConfigurationManager::ConfigurationManager()
	: keyValues{}
{
}

ConfigurationManager::~ConfigurationManager()
{
}

void ConfigurationManager::load(const string& inConfigFileName)
{
	cout << "DEBUG: Loading configuration file: " << inConfigFileName << endl;

	if (inConfigFileName.empty() == true) {
		throw GSException("Invalid configuration file name");
	}

	// Open file
	ifstream configFile(inConfigFileName);

	if (configFile.is_open() == false ) {
		// Error
		string tmpMessage = "Error: While opening configuration file: " + inConfigFileName;
		LogManager::getInstance().error(tmpMessage);

		throw GSException(tmpMessage);
	}

	// Read the whole file as a string
	string inputBuffer;
	string tmpLine;

	LogManager::getInstance().debug("DEBUG: Config file: ");
	while (configFile.eof() == false ) {
		getline (configFile,tmpLine);
		inputBuffer += tmpLine;

		LogManager::getInstance().debug(tmpLine);
		//cout << tmpLine << endl;
	}
	//cout << "DEBUG: END Config file" << endl;

	configFile.close();

	// Read key values and store them
	json11::Json tmpKeyValues;
	string errorMessage;

	tmpKeyValues = json11::Json::parse(inputBuffer, errorMessage, json11::JsonParse::STANDARD);

	if (errorMessage.empty() == false) {
		string tmpMessage = "Parsing config JSON file. errorMessage: " + errorMessage;

		LogManager::getInstance().error(tmpMessage);
		throw GSException(tmpMessage);
	}
    
    if (tmpKeyValues["documentation"].is_null() == false) {
        keyValues["documentation"] = tmpKeyValues["documentation"].string_value();
    }
    
    if (tmpKeyValues["module_name"].is_null() == false) {
        keyValues["module_name"] = tmpKeyValues["module_name"].string_value();
    }

    if (tmpKeyValues["module_version"].is_null() == false) {
        keyValues["module_version"] = tmpKeyValues["module_version"].string_value();
    }

    if (tmpKeyValues["config_log_filename"].is_null() == false) {
        keyValues["config_log_filename"] = tmpKeyValues["config_log_filename"].string_value();
    }

    if (tmpKeyValues["template_path"].is_null() == false) {
        keyValues["template_path"] = tmpKeyValues["template_path"].string_value();
    }
}

string ConfigurationManager::getValue(const string &inKey)
{
	string output{""};

	//cout << "DEBUG:   inKey = " << inKey << endl;

	if (inKey.empty() == false) {
		output = keyValues[ inKey ];

		//LogManager::getInstance().debug("keyValues[ " + inKey + " ] = " + keyValues[ inKey ] );
		//cout << "DEBUG:    keyValues[ " << inKey << " ] = " << keyValues[ inKey ] << endl;
	} else {
		throw GSException("Configruation parameter not found: " + inKey);
	}

	return output;
}

// It returns the list of strings contained in the value part
vector<string> ConfigurationManager::getListValues(const string &inKey)
{
	vector<string>   output;

	if (inKey.empty() == false) {
		string tmpBuffer = getValue(inKey);

		output = move(splitStrings(tmpBuffer));
	}

	return output;
}

// Merge a set of strings into a single string separated by comma
string ConfigurationManager::mergeStrings(const vector<json11::Json> inArray)
{
	string output;

	if (inArray.empty() == false) {
		bool firstItem = true;

		for (auto current: inArray ) {
			// Only add first comma
			if (firstItem == false) {
				output.append(",");
			} else {
				firstItem = false;
			}
			output.append(current.string_value());
		}

	}
	return output;
}


// Split a string into a set of strings separated by comma
vector<string> ConfigurationManager::splitStrings(const string inString)
{
	vector<string> output;

	if (inString.empty() == false) {
		// Alternative
		//split(tmbBuffer, ",", output);

		stringstream ss;
		char delimiter = ',';

		ss.str(inString);
		string item;

		while (getline(ss, item, delimiter)) {
			if (item.empty() == false) {
				// There is no straightforward of doing trim
				output.push_back(item);
			}
		}

	}
	return output;
}
