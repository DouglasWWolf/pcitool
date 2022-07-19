#include <unistd.h>
#include <stdio.h>
#include "PciDevice.h"

PciDevice pci;

int main()
{
   if (!pci.open(0x10ee, 0x903f, 2))
   {
      printf("Error : %s\n", pci.error());

   }
   exit(0);
}
