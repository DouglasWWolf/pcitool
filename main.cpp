#include <unistd.h>
#include <stdio.h>
#include "PciDevice.h"
#include "PhysMem.h"
#include "FpgaReg.h"
PciDevice pci;
PhysMem   mem;


FpgaReg pciProxyAddrH(REG_PCIPROXY_ADDRH);

int main()
{
   if (!pci.open(0x10ee, 0x903f))
   {
      printf("Error : %s\n", pci.error());
      exit(1);

   }

   printf("A\n");

   // Set the user-space address where AXI registers live
   FpgaReg::setUserspaceAddr(pci.resourceList()[0].baseAddr);

   printf("B\n");
   try
   {
      FpgaReg::readDefinitions("register.def");
   }
   catch(std::runtime_error& ex)
   {
      
      fprintf(stderr, "Caught: %s\n", ex.what());
      exit(1);
   }
   
   printf("About to read\n");

   printf("AXI 0x%0x = 0x%x\n", pciProxyAddrH.axiAddress(), pciProxyAddrH.read());

   exit(1);

   if (!mem.map())
   {
      printf("Error : %s\n", mem.error());
      exit(1);
   }

   uint32_t* p = (uint32_t*)mem.vptr();

   *p = 0x1234F0F0;
   exit(1);


   #if 0
   auto bar = pci.resourceList();
   uint32_t* control = (uint32_t*)bar[0].baseAddr;

   while (true)
   {
      printf("on\n");
      *control = 0xFFFFFFFF;
      usleep(500000);
      
      printf("off\n");
      *control = 0;
      usleep(500000);
   }

   #endif
}
