/**
 *  CubeGS
 *  An online Ground Segment for Cubesats and Small Sats
 *  (c) 2017 Incomplete Worlds
 *
 */
 
#include "LogManager.h"
 
 
LogManager::LogManager()
{
}

LogManager::~LogManager()
{
}

int LogManager::init(const string& inConfigFileName)
{
    log4cpp::PropertyConfigurator::configure(inConfigFileName);

    return 0;
}

void LogManager::debug(const string& inMessage)
{
    if (inMessage.empty() == false) {
        log4cpp::Category& log_ = log4cpp::Category::getRoot();

        log_.debug(inMessage);
    }
}

void LogManager::debug(const char* inMessage)
{
    if (inMessage != nullptr) {
        log4cpp::Category& log_ = log4cpp::Category::getRoot();

        log_.debug(inMessage);
    }
}

void LogManager::info(const string& inMessage)
{
    if (inMessage.empty() == false) {
        log4cpp::Category& log_ = log4cpp::Category::getRoot();

        log_.info(inMessage);
    }
}

void LogManager::info(const char* inMessage)
{
    if (inMessage != nullptr) {
        log4cpp::Category& log_ = log4cpp::Category::getRoot();

        log_.info(inMessage);
    }
}


void LogManager::warning(const string& inMessage)
{
    if (inMessage.empty() == false) {
        log4cpp::Category& log_ = log4cpp::Category::getRoot();

        log_.warn(inMessage);
    }
}

void LogManager::warning(const char* inMessage)
{
    if (inMessage != nullptr) {
        log4cpp::Category& log_ = log4cpp::Category::getRoot();

        log_.warn(inMessage);
    }
}

void LogManager::error(const string& inMessage)
{
    if (inMessage.empty() == false) {
        log4cpp::Category& log_ = log4cpp::Category::getRoot();

        log_.error(inMessage);
    }
}

void LogManager::error(const char* inMessage)
{
    if (inMessage != nullptr) {
        log4cpp::Category& log_ = log4cpp::Category::getRoot();

        log_.error(inMessage);
    }
}
