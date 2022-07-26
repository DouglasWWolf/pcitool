
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

    // Allow "regVariableName = <value>"
    FpgaReg&    operator=(uint32_t value) {write(value); return *this;};
    
    // Allow "uint32_t variableName = regVariableName"
    operator uint32_t() {return read();}

    // Reads the register (and internally saves the returned value);
    uint32_t    read();

    // Writes a value to the register
    void        write(uint32_t value);

    // Returns the AXI address of this register
    uint32_t    axiAddress();


protected:

    // The base address of registers, as mapped into userspace
    static uint8_t* userspaceBaseAddress_;

    // This maps a REG_xxxx constant to an AXI address
    static std::map<fpgareg_t, int32_t> regMap_;

    // The REG_xxxx constant that programmers use to identify a register
    fpgareg_t regIndex_;

    // This is the AXI address of this register
    uint32_t axiAddress_;

    // This is the value after the last read() or setField()
    uint32_t regValue_;


};