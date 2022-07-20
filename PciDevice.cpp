//=================================================================================================
// PciDevice.cpp - Implements a generic class for mapping PCIe devices into user-space
//=================================================================================================
#include <unistd.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "PciDevice.h"
using namespace std;

//=================================================================================================
// mapResource() - Maps a PCI resource (which corresponse to a PCI BAR) into our memory space
//
// On Exit: resource_[index].baseAddress = The base address in user-space of this resource 
//          resource_[index].size        = The size (in bytes) of that memory-mapped space
//
// If this function returns false, an error string is in errorMsg_
//=================================================================================================
bool PciDevice::mapResource(string deviceName, int index)
{
    struct stat sb;
   
    // Build the filename for this PCI resource
    string filename = deviceName + "/resource" + (char)(index + '0');

    // Fetch the filename as a const char*
    const char* fn = filename.c_str();

    // Fetch status information about the file,
    if (stat(fn, &sb) < 0) 
    {
        sprintf(errorMsg_, "%s not found", fn);
        return false;        
    }

    // Fetch the size of the PCIe resource that we are going to memory map
    size_t resourceSize = sb.st_size;

    // Open the device resource file
    int fd = ::open(fn, O_RDWR| O_SYNC);

    // If that open failed, we're done here
    if (fd < 0)
    {
        sprintf(errorMsg_, "Can't open %s", fn);
        return false;        
    }

    // Map the resources of this PCI device's BAR into our user-space memory map
    void* baseAddr = ::mmap(0, resourceSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // We don't need the file descriptor any more
    ::close(fd);

    // If that call to mmap failed, tell the caller
    if (baseAddr == (void*)(-1))
    {
        sprintf(errorMsg_, "mmap failed on %s", fn);
        return false;
    }

    //------------------------------------------------------------------------------
    // If we get here, we have succeeded in memory mapping this PCI device resource
    //------------------------------------------------------------------------------
    
    // Save the information about this PCI device resource for future use
    resource_[index] = {(uint8_t*)baseAddr, resourceSize};

    printf("Mapped 0x%lX bytes at 0x%lX\n", resourceSize, (unsigned long)baseAddr);

    // Tell the caller that all is well
    return true;
}
//=================================================================================================



//=================================================================================================
// getIntegerFromFile() - Opens the specified file, reads the first line, expects to find an 
//                        integer encoded as an ASCII string, and returns the value of that string
//=================================================================================================
static int getIntegerFromFile(string filename)
{
    string line;
   
    // Open the vendor file for this PCI device
    ifstream file(filename);

    // If we couldn't open the file, hand the caller an invalid value   
    if (!file.is_open()) return -1;
    
    // Fetch the first line of the file
    getline(file, line);

    // And hand the caller that line, decoded as an integer
    return stoi(line, 0, 0);
}
//=================================================================================================



//=================================================================================================
// Constructor
//=================================================================================================
PciDevice::PciDevice()
{
    // Mark the memory mapped PCIe resources as "released"
    for (auto& resource : resource_) resource.baseAddress = nullptr;
}
//=================================================================================================


//=================================================================================================
// close() - Unmap any memory mapped resources from this PCI device
//=================================================================================================
void PciDevice::close()
{
    // Loop through each resource slot...
    for (auto& resource : resource_)
    {
        // If this resource is memory mapped, un-map it
        if (resource.baseAddress)
        {
            munmap(resource.baseAddress, resource.size); 
            resource.baseAddress = nullptr;           
        }        
    }
}
//=================================================================================================


//=================================================================================================
// open() - Opens a connection to the specified PCIe device
//
// Passed: vendorID  = The vendor ID of the PCIe device we're looking for
//         deviceID  = The device ID of the PCIe device we're looking for
//         barCount  = The number of BARs (Base Address Registers) this device supports
//         deviceDir = Name of the file-system directory where PCI device information can
//                     be found.   If empty-string, a sensible default is used
//
// Returns: true, if the operation is a success, else false
//
// Note:    If this routine returns 'false', an ASCII error message is in errorMsg_
//=================================================================================================
bool PciDevice::open(int vendorID, int deviceID, int barCount, string deviceDir)
{
    string  dirName;

    // We have not yet found the device that we're looking for
    bool found = false;

    // If we already have a PCIe device mapped, unmap it
    close();

    // Ensure that the number of BARs we should discover is reasonable
    if (barCount < 1 || barCount > MAX_BARS)
    {
        sprintf(errorMsg_, "Invalid BAR count (%i)", barCount);
        return false;    
    }

    // If the caller didn't specify a device-directory, use the default
    if (deviceDir.empty()) deviceDir = "/sys/bus/pci/devices";

    // Loop through entry for each device in the specified directory...
    for (auto const& entry : filesystem::directory_iterator(deviceDir)) 
    {
        // Ignore any directory entry that isn't itself a directory
        if (!entry.is_directory()) continue;

        // Fetch the name of the directory that we're about to examine
        dirName = entry.path().string();

        // Fetch the vendor ID and device ID of this device
        int thisVendorID = getIntegerFromFile(dirName + "/vendor");
        int thisDeviceID = getIntegerFromFile(dirName + "/device");

        // If this vendor ID and device ID match the caller's, we have found 
        // the droid we're looking for.
        if (thisVendorID == vendorID && thisDeviceID == deviceID)
        {
            found = true;
            break;
        }
    }

    // If we couldn't find a device with that vendor ID and device ID, complain
    if (!found)
    {
        sprintf(errorMsg_, "No PCI device found for vendor=0x%X, device=0x%X", vendorID, deviceID);
        return false;
    }

    // Memory map each of the PCI device resources the caller asked us to
    for (int i=0; i<barCount; ++i) 
    {
        if (!mapResource(dirName, i)) return false;
    }

    // Tell the caller that all is well
    return true;
}
//=================================================================================================
