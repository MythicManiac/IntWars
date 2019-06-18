#include "Logger.h"

#include <errno.h>
#include <string.h>

#include <stdarg.h>

#include <time.h>

const std::string Logger::CurrentDateTime() 
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	return buf;
}

const std::string Logger::CurrentTime()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%X", &tstruct);

	return buf;
}

Logger::Logger()
{
    m_pLogFile = stdout;
}

Logger::~Logger()
{
	if (m_pLogFile != stdout)
	{
		if (isHTML) fprintf(m_pLogFile, "</table>");
		fclose(m_pLogFile);
	}
}

void Logger::setLogFile(const char *filename, bool plainText, bool showOnScreen)
{
    m_pLogFile = loadFileStream(m_pLogFile, filename);
	 isHTML = !plainText;
	 printToScreen = showOnScreen;

	 if (isHTML)
		 fprintf(m_pLogFile, "<h1>Log file (%s)</h1><table><tr><td><b>Tag</b></td><td><b>Time</b></td><td><b>File</b></td><td><b>Function</b></td><td><b>Message</b></td></tr>", CurrentDateTime().c_str());
}

void Logger::log(const std::string &tag, const char *funcName,
                 const char *sourceFile, unsigned int lineNum, 
					  const std::string& fmt, ...)
{
	std::string fileBuffer;

	fillFileBuffer(fileBuffer, tag, fmt, funcName, sourceFile, lineNum);

	// Print to file
	va_list args;
	const char* bufferCStr = fileBuffer.c_str();
	va_start(args, bufferCStr);
	vfprintf(m_pLogFile, bufferCStr, args);
	va_end(args);

	if (printToScreen)
	{
		std::string outputBuffer;
		fillOutputBuffer(outputBuffer, tag, fmt, funcName, sourceFile, lineNum);
		// Print to screen
		const char* bufferCStr = outputBuffer.c_str();
		va_start(args, bufferCStr);
		vfprintf(stdout, bufferCStr, args);
		va_end(args);
	}
}

void Logger::flush()
{
    // Flush output buffer
    fflush(m_pLogFile);
}

void Logger::fillOutputBuffer(std::string &outputBuffer, const std::string &tag, const std::string &msg,
	const char *funcName, const char *sourceFile, unsigned int lineNum)
{
	// Old format
	if(!tag.empty())
		outputBuffer = "[" + tag + "] ";
	else outputBuffer = "[NOTAG] ";

	outputBuffer += " ";
	outputBuffer += msg;
	outputBuffer += "\n";
}

void Logger::fillFileBuffer(std::string &outputBuffer, const std::string &tag, const std::string &msg,
	const char *funcName, const char *sourceFile, unsigned int lineNum)
{
	// Old format
	if (!isHTML)
	{
		if (!tag.empty())
			outputBuffer = "[" + tag + "] ";
		else outputBuffer = "[NOTAG] ";

		if (sourceFile != NULL)
		{
			outputBuffer += sourceFile;
			outputBuffer += " ";
		}

		if (funcName != NULL && lineNum != 0)
		{
			outputBuffer += funcName;
			outputBuffer += ":";
			// Convert int to char
			outputBuffer += std::to_string(lineNum);
		}

		outputBuffer += ": ";
		outputBuffer += msg;
		outputBuffer += "\n";
	}
	else
	{
		outputBuffer = "<tr>";
		if (!tag.empty())
			outputBuffer += "<td>" + tag + "</td>";
		else outputBuffer = "<td>NOTAG</td>";

		outputBuffer += "<td>" + CurrentTime() + "</td>";

		if (sourceFile != NULL)
		{
			outputBuffer += "<td>";
			outputBuffer += sourceFile;
			outputBuffer += "</td>";
		}

		if (funcName != NULL && lineNum != 0)
		{
			outputBuffer += "<td>";
			outputBuffer += funcName;
			outputBuffer += ":";
			// Convert int to char
			outputBuffer += std::to_string(lineNum);
			outputBuffer += "</td>";
		}

		outputBuffer += "<td>";
		outputBuffer += msg;
		outputBuffer += "</td></tr>";
	}
}

FILE *Logger::loadFileStream(FILE *stream, const char *filename)
{
    // overwrite previous logs
    FILE* newStream = fopen(filename, "w");

    if(newStream == NULL)
    {

        CORE_ERROR(
                   ("Failed to open logger file: " + std::string(filename) +
                   ". Using console output...\n\t" +
                   strerror(errno)).c_str()
        );

        // Use previous stream
        return stream;
    }

    return newStream;
}

