//=================================================================================================
// class FileDes - A standard Unix file-descriptor that gets closed when it goes out of scope
//=================================================================================================
#pragma once
#include <unistd.h>

class FileDes
{
public:
    // Constructor with no value
    FileDes() {fd = -1;}

    // Constructor with a file descriptor
    FileDes(int value) {fd = value;}

    // No copy or assignment constructor - objects of this class can't be copied
    FileDes (const FileDes&) = delete;
    FileDes& operator= (const FileDes&) = delete;

    // Destructor - Closes the file descriptor
    ~FileDes() {if (fd != -1) ::close(fd);}

    // Assignment from an int
    FileDes& operator=(int value) {fd=value; return *this;} 

    // Conversion to an int
    operator int() {return fd;}

    // The actual file descriptor
    int fd;
};

