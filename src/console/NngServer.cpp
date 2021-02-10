//
// Copyright Incomplete Worlds (c) 2020
// author: Alberto Fernandez
//

#include <bits/stdint-intn.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <cassert>

#include <algorithm>
#include <regex>

using namespace std;

#include "FileUtil.hpp"
#include "FileManager.hpp"


// Mustache
#include <mstch/mstch.hpp>

// New nanomsg
#include <nngpp/nngpp.h>
#include <nngpp/protocol/pull0.h>
#include <nngpp/protocol/pub0.h>
#include <nngpp/protocol/req0.h>

#include "ConfigurationManager.h"
#include "LogManager.h"
#include "GSException.h"
#include "Utils.h"

#include "orb_propagation/OrbitPropagation.h"
#include "orb_propagation/OrbitPropagationTle.h"

#include "NngServer.hpp"


NngServer::NngServer(const std::string& inConfigFile, const std::string& inInstanceId, const std::string& inPullAddress, 
const std::string& inPubAddress, const std::string& inReqAddress, const int inVerbosityFlag, Moderator *inModerator)
    : configFileName_{inConfigFile},
    instanceId_{inInstanceId},
    pullAddress_{inPullAddress},
    pubAddress_{inPubAddress},
    reqAddress_{inReqAddress},
    verbose_{inVerbosityFlag},
    mod_{inModerator}
{
//    cout << configFileName_ << endl;
//    cout << instanceId_ << endl;
//    cout << pullAddress_ << endl;
//    cout << pubAddress_ << endl;
//    cout << reqAddress_ << endl;
//    cout << verbose_ << endl;
    //cout << static_cast<uint32_t>(mod_) << endl;
}

NngServer::~NngServer()
{
}

int NngServer::Init()
{    
    try {
        // Load configuration
        ConfigurationManager::getInstance().load(configFileName_);
        

        // Initialize the Log Manager
        LogManager&  logMng = LogManager::getInstance();
        logMng.init( ConfigurationManager::getInstance().getValue("config_log_filename") );
        
        logMng.info("Configuration correctly loaded");
        
        string tmpMessage = "Starting module: " + ConfigurationManager::getInstance().getValue("module_name") + 
                            " Version: " + ConfigurationManager::getInstance().getValue("module_version");
        logMng.info(tmpMessage);
        
        if (verbose_) {
            tmpMessage = "Config file: " + configFileName_ + " Instance id: " + instanceId_ + "\n" +
                         " Pull Address: " + pullAddress_ + "\n" + 
                         " Pub Address: " + pubAddress_ + "\n" +
                         " Req Address: " + reqAddress_ + "\n" + 
                         " Verbose: " + std::to_string(verbose_);
            logMng.debug(tmpMessage);
        }
        
        // Create PULL socket
        // It will receive the input messages from the FDS
        pullSocket_ = nng::pull::open();
                
        pullSocket_.listen(pullAddress_.c_str());
        logMng.info("PULL sockets correctly created. Address: " + pullAddress_);
    
        // Create PUB socket
        // It will be used for sending the answer back to the FDS
        pubSocket_ = nng::pub::open();
        
        pubSocket_.dial(pubAddress_.c_str());
        logMng.info("PUB sockets correctly created. Address: " + pubAddress_);

        // Create REQ socket
        // It will be used for requesting data to the FDS    
        reqSocket_ = nng::req::open();

        reqSocket_.dial(reqAddress_.c_str());
        logMng.info("REQ socket correctly created. Address: " + reqAddress_);

        // Check moderator is initialized
        if (mod_ == nullptr) {
            if (verbose_) {
                logMng.debug("M oderator is not null pointer");
            }
            return -1;
        }
        if (mod_->Initialize() == false) {
            logMng.error("Moderator is not initialized");
        }

        // Send Ready message
        this->SendMessage(pubSocket_, GetStatusResponse());

    } catch(nng::exception e) {
        LogManager::getInstance().error("Exception: " + std::string{ e.what() });
        return -1;
    }

    return 0;
}

void NngServer::Shutdown()
{
}

void NngServer::RunServer()
{
    // Set timeout
    //auto  timeoutBuffer = ConfigurationManager::getInstance().getValue(ConfigurationManager::KEY_RECV_TIMEOUT);
    
    // FIXME. Find the right way of setting a timeout
    //int32_t  timeoutValue = std::atoi(timeoutBuffer.c_str());
    
    //nng::set_opt_recv_timeout(forwardSocket, timeoutValue );
    //nng_duration timeout = nng::get_opt_recv_timeout( socket );

    //forwardSocket.dial(serverAddress.c_str());


    // Main loop
    //    Process message
    //    Send answer via the PUB socket

    // Main Loop
    string          outputMessage;
    bool            done_ = false;
    
    while (done_ == false)
    {
        string authenticationKey{};
        int    executionId {0};
        string tmpMsgId { };
        
        try
        {
            string  inputMessage;
    
            // Read message
            this->ReceiveMessage(pullSocket_, inputMessage);
            if (verbose_)
                LogManager::getInstance().debug("New message received: " + inputMessage);
            
            if (inputMessage.empty() == true)
                continue;
            
            // Parse the JSON message
            json11::Json tmpKeyValues;
            string errorMessage;

            tmpKeyValues = json11::Json::parse(inputMessage, errorMessage, json11::JsonParse::STANDARD);

            if (errorMessage.empty() == false) {
                string tmpMessage = "Parsing input JSON message. errorMessage: " + errorMessage + " IGNORED";

                LogManager::getInstance().error(tmpMessage);
                continue;
            }
                        
            // Process the message
            if (tmpKeyValues["msg_code_id"].is_null() == true) {
                string tmpMessage = "Incorrect received messaged. Msg code not found. IGNORED";

                LogManager::getInstance().error(tmpMessage);
                continue;
            }
            
            string tmpMsgCode { tmpKeyValues["msg_code_id"].string_value() };

            tmpMsgId = tmpKeyValues["msg_id"].string_value();
            authenticationKey = tmpKeyValues["authentication_key"].string_value(); 
            executionId = tmpKeyValues["execution_id"].int_value();
            
            string moduleOutput;            
            bool   errorFlag{false};
            
            if (tmpMsgCode == "exit") {
                done_ = true;

            } else if (tmpMsgCode == "get_status") {
                outputMessage = GetStatusResponse();

            } else if (tmpMsgCode == "orb_propagation") {
                errorFlag = OrbitPropagation(tmpKeyValues, verbose_, mod_, moduleOutput);
                               
                if (errorFlag == true) {
                    outputMessage = ::CreateErrorJsonReply("orb_propagation_response", tmpMsgId, executionId, moduleOutput, -1); 
                } else {
                    json11::Json::object responseJson = json11::Json::object {
                                                          { "result",  moduleOutput },
                    };
                    outputMessage = ::CreateJsonReply("orb_propagation_response", tmpMsgId, executionId, 
                                                      responseJson);
                } 

            } else if (tmpMsgCode == "orb_propagation_tle") {
                errorFlag = OrbitPropagationTle(tmpKeyValues, verbose_, moduleOutput);

                // TODO: Read output from file

                if (errorFlag == true) {
                    outputMessage = ::CreateErrorJsonReply("orb_propagation_tle_response", tmpMsgId, executionId, moduleOutput, -1); 
                } else {
                    json11::Json::object responseJson = json11::Json::object {
                                                    { "result",  moduleOutput },
                    };
                    outputMessage = ::CreateJsonReply("orb_propagation_tle_response", tmpMsgId, executionId,
                                                      responseJson);
                }         
        
            } else if (tmpMsgCode == "run_script") {
                errorFlag = RunScript(tmpKeyValues, moduleOutput);

                if (errorFlag == true) {
                    outputMessage = ::CreateErrorJsonReply("run_script_response", tmpMsgId, executionId, moduleOutput, -1); 
                } else {
                    json11::Json::object responseJson = json11::Json::object {
                                                    { "result",  moduleOutput },
                    };
                    outputMessage = ::CreateJsonReply("run_script_response", tmpMsgId, executionId,
                                                      responseJson);
                }
            } else {
                string errorMessage = "ERROR: Unknown received message. It will be ignored. Code: " + tmpMsgCode;

                LogManager::getInstance().error(errorMessage);

                outputMessage = ::CreateErrorJsonReply("error_response", tmpMsgId, executionId, errorMessage, -1);
            }           
        }
        catch(GSException& gsException)
        {
            string exceptionMessage = "ERROR: GS Exception in NngServer::RunServer: " + gsException.getErrorMessage();
    
            // Create error message. MsgReturnData
            outputMessage = ::CreateErrorJsonReply("error_response", tmpMsgId, executionId, gsException.getErrorMessage(), -1); 
    
            LogManager::getInstance().error(exceptionMessage);
            //cout << exceptionMessage << endl;
        }
        // GMAT Exception
        catch (BaseException &e)
        {
            string exceptionMessage = "ERROR: GMAT Exception in NngServer::RunServer: " + e.GetFullMessage();
    
            outputMessage = ::CreateErrorJsonReply("error_response", tmpMsgId, executionId, exceptionMessage, -1); 
    
            LogManager::getInstance().error(exceptionMessage);
            //cout << exceptionMessage << endl;
        }
        catch (std::exception &e)
        {
            string exceptionMessage = "ERROR: Exception in NngServer::RunServer: " + string{e.what()};
    
            outputMessage = ::CreateErrorJsonReply("error_response", tmpMsgId, executionId, exceptionMessage, -1); 
    
            LogManager::getInstance().error(exceptionMessage);
            //cout << exceptionMessage << endl;
        }
        catch (...)
        {
            string exceptionMessage = "ERROR: Generic Exception in NngServer::RunServer ";
    
            outputMessage = ::CreateErrorJsonReply("error_response", tmpMsgId, executionId, exceptionMessage, -1); 
    
            LogManager::getInstance().error(exceptionMessage);
            //cout << exceptionMessage << endl;
        }
    
        // Send reply
        this->SendMessage(pubSocket_, outputMessage);
    }
    
    // Shutdown the module
    Shutdown();
}

string NngServer::GetStatusResponse()
{
    json11::Json::object responseJson = json11::Json::object {
        { "status", "Ready" },
        { "module_id", 0 },
        { "module_instance_id", std::atoi(this->instanceId_.c_str()) },
    };

    return ::CreateJsonReply("get_status_response", "0", 0, responseJson);
} 

/**
 * API Call: Run Script
 */
bool NngServer::RunScript(json11::Json& inJson, string &outputString)
{
    assert(mod_ != nullptr);
    
    if (verbose_) {
        string tmpJson;
        inJson.dump(tmpJson);
        LogManager::getInstance().debug( "Input JSON: " + tmpJson );        
    }
        
    string scriptText{""};
    string outputFileName{ "run_script_output.txt "};
    
    if (inJson["output_file_name"].is_null() == false)
        outputFileName = inJson["output_file_name"].string_value();
        
    if (inJson["script_text"].is_null() == false)
        scriptText = inJson["script_text"].string_value();
        
        
    string newLine{'\n'};

    std::regex newlines_re("\n+");

    string gmatScript  = std::regex_replace(scriptText, newlines_re, newLine);
    
    if (verbose_)
        LogManager::getInstance().debug("GMAT Script: \n" + gmatScript );    
                
    string scriptResult;
    bool   tmpFlag{false};
    
    tmpFlag = ::InternalRunScript(gmatScript, mod_, scriptResult);
    // Execution error?
    if (tmpFlag == false) {
        // It will contain the error message
        outputString = scriptResult;
        return false;
    }

    // if not empty, return the result
    if (scriptResult.empty() == false) {
        outputString = scriptResult;
        return true;
    }
 
    // Read the result from the output file
    // If the filename does not include the path, it will be generated in the 'output' folder    
    string tmpOutputFileName { outputFileName };
    FileManager *fm = FileManager::Instance();
    string outputPath = fm->GetFullPathname(FileManager::OUTPUT_PATH);
    
    if (GmatFileUtil::HasNoPath(outputFileName) == true) {
        tmpOutputFileName = outputPath + outputFileName;                           
    }
    if (verbose_) {
        LogManager::getInstance().debug("output path = " + outputPath);
        LogManager::getInstance().debug("output file name = " + tmpOutputFileName);
    }
    
    // Read the output file
    ifstream  outputFile { tmpOutputFileName.c_str() };
    string    result{""};
    
    if (!outputFile) {
        string errorMessage = "ERROR: Output file not found: " + tmpOutputFileName;
        
        LogManager::getInstance().error(errorMessage);
        outputString = errorMessage;
        return false;
    } 

    // Clear it
    stringstream buffer;
    
    buffer << outputFile.rdbuf();
    outputString = buffer.str();
        
    outputFile.close();        
    
    //if (verbose_)
    //    LogManager::getInstance().debug("Output: " + result);
    
    // Send answer back
    return true;        
}

// TODO: To move to Util as it could be a generic function
//bool NngServer::InternalRunScript(const string& inScriptText, string &outputString)
// {       
//     // Run script
//     // Clear
//     mod_->ClearScript();
    
//     // Open
//     istringstream   strStream{ inScriptText + "\n" };
    
//     //strStream << inScriptText << endl;
     
//     // flag to clear objects and mission sequence
//     bool mPassedInterpreter = mod_->InterpretScript(&strStream, true);
    
//     /*
//      * @return  1 if run was successful
//      *         -1 if sandbox number is invalid
//      *         -2 if exception thrown during sandbox initialization
//      *         -3 if unknown error occurred during sandbox initialization
//      *         -4 if execution interrupted by user
//      *         -5 if exception thrown during the sandbox execution
//      *         -6 if unknown error occurred during sandbox execution
//      */
//     int returnCode = mod_->RunScript();
    
//     if (returnCode != 1) {
//         string errorMessage = "ERROR: Running the script: " + std::to_string(returnCode);
        
//         LogManager::getInstance().error(errorMessage);
//         outputString = errorMessage;
//         return false;
//     }

//     // Wait for answer
//     Gmat::RunState state = Gmat::RUNNING;

//     while (state == Gmat::RUNNING) {
//         this_thread::sleep_for(chrono::milliseconds(100));
         
//         state = mod_->GetRunState();
//     }

//     // TODO or FIXME: How to retrieve the output string from the execution
//     // Is it possible?
    
//     outputString = "";
//     return true;
// }


void NngServer::ReceiveMessage(nng::socket &inSocket, string& outMessage)
{
    LogManager::getInstance().debug("Waiting for a message ");
 
    try
    {
        // Read the message
        nng::msg newMsg;

        newMsg = inSocket.recv_msg();
        
        // Message format. JSON message
        char *tmpBuffer = newMsg.body().data<char>();
        
        // Read rest of message
        auto msgLen = newMsg.body().size();
        LogManager::getInstance().debug("   RECEIVED Message. Msg Len: " + to_string(msgLen));
        
        outMessage.resize(msgLen);
        for(uint i = 0; i < msgLen; i ++)
        {
            outMessage[i] = tmpBuffer[i];
        }
    }
    catch( const nng::exception& e ) 
    {
        // who() is the name of the nng function that produced the error
        // what() is a description of the error code
        LogManager::getInstance().error(string(e.who()) + " " + string(e.what()));
        outMessage="";
    }
}

void NngServer::SendMessage(nng::socket &inSocket, const string& inMessage)
{
    if (verbose_)
        LogManager::getInstance().debug("Sending message: " + inMessage);
     
    try
    {
        // Message format. JSON message
        // MsgCode (16 bits), Len Auth Token (16 bits), Token (256 bytes max), Rest of message (2 or 4 KB - TBC)

        nng::msg newMsg{static_cast<int>(0)};
                
        // Add the message
        nng::view  tmpMsg{ inMessage.data(), inMessage.length() };
        
        newMsg.body().append(tmpMsg);
        
        inSocket.send( std::move(newMsg) );
        
        LogManager::getInstance().debug("Message sent");
    }
    catch( const nng::exception& e ) 
    {
        // who() is the name of the nng function that produced the error
        // what() is a description of the error code
        LogManager::getInstance().error(string(e.who()) + " " +  string(e.what()));
    }
}
