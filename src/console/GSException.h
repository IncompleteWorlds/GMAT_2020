/**
 *  CubeGS
 *  An online Ground Segment for Cubesats and Small Sats
 *  (c) 2016 Incomplete Worlds
 *
 */

#ifndef __GSEXCEPTION_H_INCLUDED__
#define __GSEXCEPTION_H_INCLUDED__

#include <iostream>
#include <exception>

using namespace std;

class GSException : public exception
{
    public:
        GSException(const char *inMessage)
            : errorCode{-1}, exceptionMessage{inMessage}  {}

        GSException(const string& inMessage)
            : errorCode{-1}, exceptionMessage{inMessage}  {}

        GSException(const int inCode, const char *inMessage)
            : errorCode{inCode}, exceptionMessage{inMessage}  {}

        GSException(const int inCode, const string& inMessage)
            : errorCode{inCode}, exceptionMessage{inMessage}  {}

        virtual const char* what() const throw()
        {
            return exceptionMessage.c_str();
        }

        int getErrorCode() {
            return errorCode;
        }

        string getErrorMessage() {
            return exceptionMessage;
        }

    private:
        int    errorCode;
        string exceptionMessage;
};

#endif // GSEXCEPTION_H_INCLUDED
