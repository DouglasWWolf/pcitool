
//=================================================================================================
// FpgaReg.h - Defines a class that manages FPGA registers
//=================================================================================================
#pragma once
#include <stdint.h>
#include <map>
#include <string>



//=================================================================================================
// The AXI address of every FPGA register in the system
//=================================================================================================
enum fpgareg_t
{
    REG_PCIPROXY_ADDRH,
    REG_PCIPROXY_ADDRL,
    REG_PCIPROXY_DATA,
    REG_COUNT
};
//=================================================================================================



class FpgaReg
{
public:

    // Set the base address of the PCI region as mapped into user-space
    static void setUserspaceAddr(uint8_t* userspaceAddress);

    // Reads the file that defines the addresses and field info about AXI registers
    static void readDefinitions(std::string filename);

    // Constructor requires the AXI address of the register
    FpgaReg(fpgareg_t axiRegister);

    // Reads the register (and internally saves the returned value);
    uint32_t    read();



protected:

    // The base address of registers, as mapped into userspace
    static uint8_t* userspaceBaseAddress_;

    // This maps a REG_xxxx constant to an AXI address
    static std::map<fpgareg_t, int32_t> regMap_;

    // This is the AXI address of this register
    uint32_t axiAddress_;


};