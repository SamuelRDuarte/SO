#include "grp_mksofs.h"

#include "core.h"
#include "bin_mksofs.h"

#include <inttypes.h>
#include <iostream>

namespace sofs19
{
    void grpComputeStructure(uint32_t ntotal, uint32_t & itotal, uint32_t & nbref)
    {

        int iVar;
        uint aux1;
        uint aux2;
        
        soProbe(601, "%s(%u, %u, ...)\n", __FUNCTION__, ntotal, itotal);
        if (itotal == 0) {
            itotal = ntotal >> 4;  // divisão por 16
        }

        if (ntotal >> 2 < itotal) {  
            itotal = ntotal >> 2;  // divisão por 16
        }

        if (itotal == 0) {
            itotal = 8;
        }

        if ((itotal & 7) != 0) {
            itotal = (itotal & 0xfffffff8) + 8;
        }

        iVar = ntotal - (itotal >> 3);
        if (iVar - 2U < 0x41) {
            aux1 = 0;
        }else {
            aux1 = iVar - 0x42;
        }
        if ((aux1 & 0xff) == 0) {
            aux2 = aux1 >> 8;
        }else {
            if ((aux1 & 0xff) == 1) {
            aux2 = aux1 >> 8;
            itotal = itotal + 8;
            }
            else {
            aux2 = (aux1 >> 8) + 1;
            }
        }
        nbref = aux2;
        return;
        /* change the following line by your code */
        //binComputeStructure(ntotal, itotal, nbref);
    }
};
