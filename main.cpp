#include <unistd.h>
#include <stdio.h>
#include "PciDevice.h"
#include "PhysMem.h"
PciDevice pci;
PhysMem   mem;

int main()
{
   if (!pci.open(0x10ee, 0x903f))
   {
      printf("Error : %s\n", pci.error());

   }
   
   if (!mem.map())
   {
      printf("Error : %s\n", mem.error());
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
