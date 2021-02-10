//
// Copyright Incomplete Worlds (c) 2020
// author: Alberto Fernandez
//

#include <ctime>
#include <cstdlib>
#include <thread>
 
using namespace std;

#include "APIFunctions.hpp"

#include "LogManager.h"
#include "GSException.h"

#include "Utils.h"



string CreateErrorJsonReply(const string& inMsgCodeId, const string& inMsgId, const int inExecutionId, 
    const string& inBuffer, const int inErrorCode)
{
    json11::Json errorJson = json11::Json::object {
        { "code",               inErrorCode },
        { "message",            inBuffer },
    };

    json11::Json::object responseJson = json11::Json::object {
        { "msg_id",             inMsgId },
        { "msg_code_id",        inMsgCodeId },
        { "result",             json11::Json::NUL },
        { "error",              errorJson },
    };

    json11::Json outputJson = json11::Json::object {
        { "execution_id",       inExecutionId },
        { "wait_flag",          false },
        { "response",           responseJson },
    };
    
    return outputJson.dump();
}

string CreateJsonReply(const string& inMsgCodeId, const string& inMsgId, const int inExecutionId, const json11::Json::object& inJsonObject)
{
    // current date/time based on current system
    //time_t now = time(0);

    json11::Json::object responseJson = json11::Json::object {
        { "msg_id",              inMsgId },
        { "msg_code_id",         inMsgCodeId },
        //{ "result",              inJsonObject },
        //{ "error",               json11::Json::NUL },
    };

    responseJson.insert(inJsonObject.cbegin(), inJsonObject.cend());

    json11::Json outputJson = json11::Json::object {
        { "execution_id",        inExecutionId },
        { "wait_flag",           false },
        { "response",            responseJson },
    };
    
    return outputJson.dump();
}

/**
 * @Return - true - if there is an error, false otherwise
 */
bool InternalRunScript(const string& inScriptText, Moderator *inMod, string &outputString)
{
    // Check parameters
    if (inMod == nullptr) {
        string errorMessage = "Moderator is a null pointer";
        
        LogManager::getInstance().error(errorMessage);
        outputString = errorMessage;
        return true;
    }

    if (inScriptText.empty() == true) {
        // Nothing to be done
        LogManager::getInstance().info("Empty script. Ignored");
        outputString = "";
        return false;
    }

    // check Moderator is initialised
    if (inMod->IsInitialized() == false) {
        string errorMessage = "Moderator is not initialised";
        
        LogManager::getInstance().error(errorMessage);
        outputString = errorMessage;
        return true;
    }

    // Run script
    // Clear
    inMod->ClearScript();
    
    // Open
    istringstream   strStream{ inScriptText + "\n" };
    
    //strStream << inScriptText << endl;
     
    // flag to clear objects and mission sequence
    bool mPassedInterpreter = inMod->InterpretScript(&strStream, true);
    if (mPassedInterpreter == false) {
        string errorMessage = "ERROR: Interpreting the script";
        
        LogManager::getInstance().error(errorMessage);
        outputString = errorMessage;
        return true;
    }
    /*
     * @return  1 if run was successful
     *         -1 if sandbox number is invalid
     *         -2 if exception thrown during sandbox initialization
     *         -3 if unknown error occurred during sandbox initialization
     *         -4 if execution interrupted by user
     *         -5 if exception thrown during the sandbox execution
     *         -6 if unknown error occurred during sandbox execution
     */
    int returnCode = inMod->RunScript();
    
    if (returnCode != 1) {
        string errorMessage = "ERROR: Running the script: " + std::to_string(returnCode);
        
        LogManager::getInstance().error(errorMessage);
        outputString = errorMessage;
        return true;
    }

    // // Wait for answer
    // Gmat::RunState state = Gmat::RUNNING;

    // while (state == Gmat::RUNNING) {
    //     this_thread::sleep_for(chrono::milliseconds(50));
         
    //     state = inMod->GetRunState();
    // }

    // TODO or FIXME: How to retrieve the output string from the execution
    // Is it possible?
    
    outputString = ::GetRunSummary();
    return false;
}

