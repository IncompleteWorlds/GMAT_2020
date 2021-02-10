/**
 *  CubeGS
 *  An online Ground Segment for Cubesats and Small Sats
 *  (c) 2017 Incomplete Worlds
 *
 */

#ifndef __LOG_MANAGER_H__
#define __LOG_MANAGER_H__

#include <string>

using namespace std;

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>


class LogManager 
{
    public:
        // Singleton
        static LogManager& getInstance()
        {
            static LogManager instance;

            return instance;
        }

        LogManager();
        ~LogManager();
        
        int init(const string& inConfigFileName);
      
        void debug(const string& inMessage);
        void debug(const char *inMessage);
        
        void info(const string& inMessage);
        void info(const char * inMessage);
        
        void warning(const string& inMessage);
        void warning(const char * inMessage);
        
        void error(const string& inMessage);
        void error(const char *inMessage);
        
        
    protected:
      
};

#endif  // __LOG_MANAGER_H__