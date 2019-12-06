/*#include "../../../include/grp/grp_mksofs.h"

#include "../../../include/rawdisk.h"
#include "../../../include/core.h"
#include "../../../include/bin/bin_mksofs.h"
*/

#include "grp_mksofs.h"

#include "rawdisk.h"
#include "core.h"
#include "bin_mksofs.h"

#include <string.h>
#include <inttypes.h>


namespace sofs19
{
    /*
       filling in the contents of the root directory:
       the first 2 entries are filled in with "." and ".." references
       the other entries are empty.
       */
    void grpFillRootDir(uint32_t itotal)
    {
        soProbe(606, "%s(%u)\n", __FUNCTION__, itotal);

        /* change the following line by your code */
        //return binFillRootDir(itotal);

        SODirEntry direntry[DPB];            //SODirEntry buffer

        int firstblock = 1 + itotal/8;

        memset(direntry,0,BlockSize);              //field name filled with zeros

        for(uint32_t i=2; i<DPB; i++) {      //the other slots must be cleaned: field inode filled with NullReference
            direntry[i].in = NullReference;
        }

        memcpy(direntry[0].name, ".", 2);       //the first two entries are filled in with "." and ".." directory entries
        memcpy(direntry[1].name, "..", 2); 

        soWriteRawBlock(firstblock,direntry);            //[in] n   -> physical number of the block to be written into
                                                //[in] buf -> pointer to the buffer containing the data to be written from 
                                                //formata o bloco com os valores do dir
        
    }
};

