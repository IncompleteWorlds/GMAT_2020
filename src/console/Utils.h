//
// Copyright Incomplete Worlds (c) 2020
// author: Alberto Fernandez
//

#ifndef __UTILS_H__
#define __UTILS_H__

#include <iostream>
#include <string>

using namespace std;

#include "json11.hpp"

#include "Moderator.hpp"
#include "GmatBase.hpp"


string CreateErrorJsonReply(const string& inMsgCodeId, const string& inMsgId, const int inExecutionId, 
                            const string& inBuffer, const int inErrorCode);

string CreateJsonReply(const string& inMsgCodeId, const string& inMsgId, const int inExecutionId, 
                       const json11::Json::object& inJsonObject);

bool InternalRunScript(const string& inScriptText, Moderator *inMod, string &outputString);
#endif   //__UTILS_H__
