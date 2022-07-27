//=================================================================================================
// PciDevice.cpp - Implements a generic class for mapping PCIe devices into user-space
//=================================================================================================
#include <unistd.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "PciDevice.h"
using namespace std;


//=================================================================================================
// mapResources() - Maps each memory-mappable resource for this device into user-space
//
// On Entry: resource_ = list of memory-mappable resources (the phys addr and the size)
//
// On Exit:  resource_ = each entry has userspace "baseAddr" filled in
//=================================================================================================
bool PciDevice::mapResources()
{
    const char* filename = "/dev/mem";

    // Assume for a moment that this routine will succeed
    bool result = true;

    // These are the memory protection flags we'll use when mapping the device into memory
    const int protection = PROT_READ | PROT_WRITE;

    // Open the /dev/mem device
    int fd = ::open(filename, O_RDWR| O_SYNC);

    // If that open failed, we're done here
    if (fd < 0)
    {
        close();
        sprintf(errorMsg_, "Can't open %s", filename);
        return false;        
    }

    // Loop through each entry in the list of memory-mappable resources for this PCI device
    for (auto& bar : resource_)
    {
        // Map the resources of this PCI device's BAR into our user-space memory map
        void* ptr = ::mmap(0, bar.size, protection, MAP_SHARED, fd, bar.physAddr);

        // If a mapping error occurs, unmap anything we already have mapped
        if (ptr == MAP_FAILED)
        {
            sprintf(errorMsg_, "mmap failed on 0x%lx for size 0x%lx", bar.physAddr, bar.size);
            close();
            result = false;
            break;
        }

        // Otherwise, save the user-space address that our PCI resource is mapped to
        else bar.baseAddr = (uint8_t*)ptr;
    }

    // Clean up after ourselves
    ::close(fd);

    // And tell the caller whether all of this device's PCI resources got mapped into user-space
    return result;
}
//=================================================================================================


//=================================================================================================
// getIntegerFromFile() - Opens the specified file, reads the first line, expects to find an 
//                        integer encoded as an ASCII string, and returns the value of that string
//=================================================================================================
static int getIntegerFromFile(string filename)
{
    string line;
   
    // Open the specified file.  It will contain a line of ASCII data
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
// close() - Unmap any memory mapped resources from this PCI device
//=================================================================================================
void PciDevice::close()
{
    // Loop through each resource slot, and if it's memory mapped, unmap it
    for (auto& resource : resource_)
    {
        if (resource.baseAddr) munmap(resource.baseAddr, resource.size); 
    }

    // Delete the list of memory-mapped resources
    resource_.clear();
}
//=================================================================================================



//=================================================================================================
// getResourceList() - Returns a vector of resource_t entries that describe each memory-mappable
//                     resource (i.e., BAR) that this PCI device supports
//
// On Entry: deviceDir = the name of the device directory that contains the "resource" file
//
// If this routine returns an empty vector, errorMsg_ will contain the appropriate error message
//
// Notes: The resource file will contain 1 line of ASCII data for each potential mappable resource.
//        Each line contains 3 fields separated one space character:
//           (1) The physical starting address of the memory mapped resource
//           (2) The physical ending address of the memory mapped resource
//           (3) A set of flags that we don't care about    
//=================================================================================================
std::vector<PciDevice::resource_t> PciDevice::getResourceList(std::string deviceDir)
{
    string             line;
    vector<resource_t> result;
    
    // This file will contain 1 line per potential resource
    string filename = deviceDir + "/resource";

    // Open the specified file  
    ifstream file(filename);

    // If we couldn't open the file, hand the caller an invalid value   
    if (!file.is_open()) 
    {
        sprintf(errorMsg_, "Can't open %s", filename.c_str());
        return result;
    }
    
    // Loop through each line of the file...
    while (getline(file, line))
    {
        // Get pointers to the 1st and 2nd text fields of that line
        const char* p1 = line.c_str();
        const char* p2 = strchr(p1, ' ');
        
        // Parse the physical starting and ending address of this memory-mappable resource
        off_t starting_address = strtoll(p1, 0, 0);
        off_t ending_address   = strtoll(p2, 0, 0);

        // A starting address of 0 means "this line doesn't define a memory-mappable resource"
        if (starting_address == 0) continue;

        // Compute how many bytes long that memory region is
        size_t size = ending_address - starting_address + 1;

        // Append the description of this mappable resource into our result vector        
        result.push_back({0, size, starting_address});
    }

    // If there are no memory-mappable resources, create an error message
    if (result.empty()) sprintf(errorMsg_, "Device contains no memory-mappable resources");

    // Hand the caller the list of resources that can be memory mapped for this PCI device
    return result;
}
//=================================================================================================




//=================================================================================================
// open() - Opens a connection to the specified PCIe device
//
// Passed: vendorID  = The vendor ID of the PCIe device we're looking for
//         deviceID  = The device ID of the PCIe device we're looking for
//         deviceDir = Name of the file-system directory where PCI device information can
//                     be found.   If empty-string, a sensible default is used
//
// Returns: true, if the operation is a success, else false
//
// Note:    If this routine returns 'false', an ASCII error message is in errorMsg_
//=================================================================================================
bool PciDevice::open(int vendorID, int deviceID, string deviceDir)
{
    string  dirName;

    // We have not yet found the device that we're looking for
    bool found = false;

    // If we already have a PCIe device mapped, unmap it
    close();

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

    // Fetch the physical address and size of each resource (i.e. BAR) that device supports
    resource_ = getResourceList(dirName);

    // If there are no memory-mappable resources for this PCI device, tell the caller
    if (resource_.empty()) return false;

    // Memory map each of the PCI device resources the caller asked us to
    return mapResources();
}
//=================================================================================================
