//
// Copyright Incomplete Worlds (c) 2020
// author: Alberto Fernandez
//

#ifndef __NNG_SERVER_H__
#define __NNG_SERVER_H__

#include <string>

using namespace std;

#include <nngpp/nngpp.h>

#include "json11.hpp"

#include "Moderator.hpp"
#include "GmatBase.hpp"


class NngServer
{
    public:
        NngServer(const std::string& inConfigFile, 
                  const std::string& inInstanceId, 
                  const std::string& inPullAddress, 
                  const std::string& inPubAddress, 
                  const std::string& inReqAddress,
                  const int inVerbosityFlag, 
                  Moderator *inModerator);
        ~NngServer();
        
        // Load configuration file
        // Create soctkets
        int Init();
        
        void RunServer();

    private:
        string     configFileName_;
        string     instanceId_;
        string     pullAddress_;
        string     pubAddress_;
        string     reqAddress_;
        int        verbose_;
        Moderator *mod_;
        
        // Receive message from FDS
        nng::socket   pullSocket_;
        // Publish answer to FDS
        nng::socket   pubSocket_;
        // Request data to FDS
        nng::socket   reqSocket_;
        
        void ReceiveMessage(nng::socket &inSocket, string& inMessage);
        void SendMessage(nng::socket &inSocket, const string& inMessage);
        
        void Shutdown();
        
        string GetStatusResponse();

        bool RunScript(json11::Json& inJson, string &outputString);
        //bool InternalRunScript(const string& inScriptText, string &outputString);
};

#endif // __NNG_SERVER_H__
