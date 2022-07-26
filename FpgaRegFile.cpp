
//=================================================================================================
// FpgaRegFile.cpp - Contains code for reading the FPGA register definitions file
//=================================================================================================
#include <stdio.h>
#include <fstream>
#include <stdarg.h>
#include <vector>
#include <cstring>
#include "FpgaReg.h"
using namespace std;



// Returns true if the character indicates the end of the line is reached
static inline bool isEOL(char c) {return (c == 0 || c == 10 || c == 13);}

// This is the name of the file we're processing
static const char* fn;

// This will be used by "throw_runtime()"
static int lineNumber;

// Convenient way to fetch a const char* to string data
const char* c(const string& s) {return s.c_str();}


//==========================================================================================================
// parseTokens() - Parses an input string into a vector of tokens
//==========================================================================================================
static vector<string> parseTokens(const char* in)
{
    vector<string> result;
    char           token[512];

    // If we weren't given an input string, return an empty result;
    if (in == nullptr) return result;

    // So long as there are input characters still to be processed...
    while (!isEOL(*in))
    {
        // Point to the output buffer
        char* out = token;

        // Skip over any leading spaces or tabs on the input
        while (*in == 32 || *in == 9) in++;

        // If we hit end-of-line, there are no more tokens to parse
        if (isEOL(*in)) break;

        // Assume for the moment that we're not starting a quoted string
        char inQuotes = 0;

        // If this is a single or double quote-mark, remember it and skip past it
        if (*in == '"' || *in == '\'') inQuotes = *in++;

        // Loop until we've parsed this entire token...
        while (!isEOL(*in))
        {
            // If we're parsing a quoted string...
            if (inQuotes)
            {
                // If we've hit the ending quote-mark, we're done parsing this token
                if (*in == inQuotes)
                {
                    ++in;
                    break;
                }
            }

            // Otherwise, we're not parsing a quoted string. A space, tab, or comma ends the token
            else if (*in == 32 || *in == 9 || *in == ',') break;

            // Append this character to the token buffer
            *out++ = *in++;
        }

        // nul-terminate the token string
        *out = 0;

        // Add the token to our result list
        result.push_back(token);

        // Skip over any trailing spaces or tabs in the input
        while (*in == ' ' || *in == 9) ++in;

        // If there is a trailing comma, throw it away
        if (*in == ',') ++in;
    }

    // Hand the caller a vector of tokens
    return result;
}
//==========================================================================================================




//=================================================================================================
// throw_runtime() - Throws a std::runtime_error exception
//=================================================================================================
static void throw_runtime(const char* fmt, ...)
{
    char message[1024];
    va_list ap;

    // The message begins with "<filename>: " or with "<filename>, line <line>, "
    if (lineNumber == 0)
        sprintf(message, "%s: ", fn);
    else
        sprintf(message, "%s, line %i: ", fn, lineNumber);

    // Find the end of the message
    char* end = strchr(message, 0);

    va_start(ap, fmt);
    vsprintf(end, fmt, ap);
    va_end(ap);

    throw runtime_error(message);
}
//=================================================================================================


//=================================================================================================
// getConstantValue() - Returns the constant that corresponds to a given baseName/regName combo
//=================================================================================================
static fpgareg_t getConstantValue(const string& baseName, const string& regName)
{
    if (baseName == "PCIPROXY")
    {
        if (regName == "DATA" ) return REG_PCIPROXY_DATA;
        if (regName == "ADDRH") return REG_PCIPROXY_ADDRH;
        if (regName == "ADDRL") return REG_PCIPROXY_ADDRL;
        throw_runtime("Uknown register %s:%s", c(baseName), c(regName));
    }

    // If we get here, the definitions file contains and unknown base name
    throw_runtime("Unknown base register %s", c(baseName));

    // We'll never get here, but this keeps the compiler happy
    return (fpgareg_t)-1;
}
//=================================================================================================


//=================================================================================================
// readDefinitions() - Reads and parses the file that contains AXI register definitions.
//=================================================================================================
void FpgaReg::readDefinitions(string filename)
{
    string   line, baseName = "", regName;
    uint32_t baseAddr = 0, regOffset;

    // We haven't read in any lines of text yet
    lineNumber = 0;

    // Get a const char* to the filename
    fn = filename.c_str();

    // Open the specified file name
    ifstream file(filename);

    // If we couldn't open the file, tell the caller
    if (!file.is_open()) throw_runtime("Can't open");

    // Loop through each line of the file
    while (getline(file, line))
    {
        // Keep track of the line number
        ++lineNumber;

        // Fetch a pointer to the characters in the line
        const char* p = line.c_str();

        // Skip over any leading spaces or tabs
        while (*p == 32 || *p == 9) ++p;

        // If the line is blank, skip this line
        if (*p == 0 || *p == 10 || *p == 13) continue;

        // If the line is a comment, skip this line
        if (*p == '#' || (p[0] == '/' && p[1] == '/')) continue;

        // Parse the line into tokens
        vector<string> tokens = parseTokens(p);

        // If this is the "base" command, expect a name and an address
        if (tokens[0] == "base")
        {
            if (tokens.size() < 3) throw_runtime("Syntax error");
            baseName = tokens[1];
            baseAddr = stoul(tokens[2]);
            continue;
        }

        // If this is a "reg" command, expect a name and an offset
        if (tokens[1] == "reg")
        {
            if (tokens.size() < 3) throw_runtime("Syntax error");
            regName = tokens[1];
            regOffset = stoul(tokens[2]);
            auto constantValue = getConstantValue(baseName, regName);
            regMap_[constantValue] = baseAddr + regOffset;
            continue;
        }



        printf("%s\n", p);
    }

    exit(1);

}
//=================================================================================================



