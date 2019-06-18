#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <string>
#include <stdio.h>

class Logger
{
public:
    static Logger& instance()
    {
        static Logger instance;
        return instance;
    }

    ~Logger();
	 
	 static const std::string CurrentDateTime();
	 static const std::string CurrentTime();
	 void setLogFile(const char* filename, bool isPlainText = false, bool showOnScreen = false);

    void log(const std::string& tag, const char* funcName, const char* sourceFile, 
        unsigned int lineNum, const std::string& fmt, ...);

    /**
     * @brief Flushes output buffer. Called for critical events
     */
    void flush();

    // Used for error logging
    //static void error(bool isFatal, const std::string& msg,
    //     const char* funcName, const char* sourceFile, unsigned int lineNum);

protected:
    Logger();

private:
    Logger(const Logger&);
    Logger& operator=(const Logger&);

	 void fillOutputBuffer(std::string& outputBuffer,
		 const std::string& tag, const std::string& msg,
		 const char* funcName, const char* sourceFile, unsigned int lineNum);
	 void fillFileBuffer(std::string& outputBuffer,
		 const std::string& tag, const std::string& msg,
		 const char* funcName, const char* sourceFile, unsigned int lineNum);

    inline FILE* loadFileStream(FILE* stream, const char* filename);

    FILE* m_pLogFile;
	 bool isHTML = false;
	 bool printToScreen = false;
};


#ifndef NDEBUG // If in debug mode

// Define debugger break symbols
#ifdef __unix__ // Linux & BSD

// For SIGTRAP signal
#include <signal.h>

#define DEBUG_BREAK raise(SIGTRAP)

#elif defined(_MSC_VER) // Visual studio 

#define DEBUG_BREAK __debugbreak();

#elif defined(__GNUG__)
//#include <csignal>
//#define DEBUG_BREAK raise(SIGINT);
#define DEBUG_BREAK __debugbreak(); // This one works fine
#else

#warning "Unknown Operating System, debug break support will not be included"

// Remove support for debug break on other platforms
#define DEBUG_BREAK 0

#endif

// Note, the do-while removes the ; at the end of the call
/*#define CORE_ASSERT(expr) \
	do \
	{ \
		if(!(expr)) \
		{ \
			Logger::instance().log("ASSERT", __FUNCTION__, __FILE__, __LINE__, #expr); \
			Logger::instance().flush(); \
            DEBUG_BREAK; \
		} \
	} \
	while(0)\
/*
#define CORE_FATAL(...) \
    do \
    { \
        Logger::instance().log("FATAL", __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);\
        Logger::instance().flush(); \
        DEBUG_BREAK; \
        std::terminate(); \
    } \
    while(0)\

#define CORE_ERROR(...) \
    do \
    { \
        Logger::instance().log("ERROR", __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
        Logger::instance().flush(); \
    } \
    while(0)\


#define CORE_WARNING(...) \
    do \
    { \
        Logger::instance().log("WARNING", __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);\
        Logger::instance().flush(); \
    } \
    while(0)\


#define CORE_DEBUG(...) \
    do \
	 { \
        Logger::instance().log("DEBUG", __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
        Logger::instance().flush(); \
    } \
    while(0) \

#define CORE_LOG(tag, ...) \
    do \
	 { \
        Logger::instance().log(tag, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
        Logger::instance().flush(); \
    } \
    while(0) \


#define CORE_VERBOSE(...) \
    do \
    { \
        Logger::instance().log("VERBOSE", __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
    } \
    while(0) \

#else // Not in debug mode

// Keep errors and warnings, just exclude func, file and line info
#define CORE_FATAL(...) \
    do \
    { \
        Logger::instance().log("FATAL", NULL, NULL, 0, __VA_ARGS__); \
        Logger::instance().flush(); \
        std::terminate(); \
    } \
    while(0)\


#define CORE_ERROR(...) \
    do \
    { \
        Logger::instance().log("ERROR", NULL, NULL, 0, __VA_ARGS__); \
        Logger::instance().flush(); \
    } \
    while(0)\


#define CORE_WARNING(...) \
    do \
    { \
        Logger::instance().log("WARNING", NULL, NULL, 0, __VA_ARGS__);\
    } \
    while(0)\

// Release mode definitions of macros. Defined in such a way as to be
// ignored completelly by the compiler
#define CORE_DEBUG(...) do { (void)sizeof(__VA_ARGS__); } while(0)
#define CORE_VERBOSE(...) do { (void)sizeof(__VA_ARGS__); } while(0)
#define CORE_LOG(tag, ...) do { (void)sizeof(__VA_ARGS__); } while(0)
#define CORE_ASSERT(expr) do { (void)sizeof(expr); } while(0)

#endif

#define CORE_INFO(...) \
    do \
	 { \
        Logger::instance().log("INFO", __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__); \
    } \
    while(0) \*/


// TODO : Logger::instance().log prints incorrect arguments
// btw. why don't we just..
// on windows: app.exe > log.txt
// on linux: screen -L // ./app

#define CORE_DEBUG(...) { printf("\n[DEBUG] ");printf(__VA_ARGS__); }
#define CORE_LOG(tag, ...) { printf("\n[%s] ", tag);printf(__VA_ARGS__); }
#define CORE_VERBOSE(...) { printf("\n[VERBOSE] ");printf(__VA_ARGS__); }

#define CORE_FATAL(...) { printf("\n[FATAL] ");printf(__VA_ARGS__);DEBUG_BREAK;std::terminate(); }

#else

#define CORE_DEBUG(...) ;
#define CORE_LOG(tag, ...) ;
#define CORE_VERBOSE(...) ;

#define CORE_FATAL(...) { printf("\n[FATAL] ");printf(__VA_ARGS__);std::terminate(); }

#endif

#define CORE_ERROR(...) { printf("\n[ERROR] ");printf(__VA_ARGS__); }
#define CORE_WARNING(...) { printf("\n[WARNING] ");printf(__VA_ARGS__); }
#define CORE_INFO(...) { printf("\n[INFO] ");printf(__VA_ARGS__); }



#endif
