//=================================================================================================
// PciDevice.h - Defines a generic class for mapping PCIe devices into user-space
//=================================================================================================
#pragma once
#include <string>

class PciDevice
{
public:
    
    // Constructor
    PciDevice();
    
    // Destructor
    ~PciDevice() {close();}

    // No copy or assignment constructor - objects of this class can't be copied
    PciDevice (const PciDevice&) = delete;
    PciDevice& operator= (const PciDevice&) = delete;

    // Opens a connection to a PCIe device
    bool    open(int vendorID, int deviceID, int barCount, std::string deviceDir = "");

    // Stop access to the PCI device
    void    close();

    // Fetches an error message after a failed "open()"
    const char* error() {return errorMsg_;}

protected:

    // A PCIe device can have a maximum of 3 64-bit base-address registers    
    static const int MAX_BARS = 3;

    // This gets called in order to memory map PCI device resources into user-space
    bool    mapResource(std::string deviceName, int index);

    // Contains the error message after a call to "open" fails
    char    errorMsg_[256];

    // These each describe a memory mapped resource from a PCI device
    struct {uint8_t* baseAddress; size_t size;} resource_[MAX_BARS]; 
};
