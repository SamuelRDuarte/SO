#include "grp_fileblocks.h"

#include "dal.h"
#include "core.h"
#include "bin_fileblocks.h"

#include <errno.h>

namespace sofs19
{
    /* ********************************************************* */


    static uint32_t grpGetIndirectFileBlock(SOInode * ip, uint32_t fbn);
    static uint32_t grpGetDoubleIndirectFileBlock(SOInode * ip, uint32_t fbn);


    /* ********************************************************* */

    uint32_t grpGetFileBlock(int ih, uint32_t fbn)
    {
        soProbe(301, "%s(%d, %u)\n", __FUNCTION__, ih, fbn);

        /* change the following line by your code */
        //return binGetFileBlock(ih, fbn);
        SOInode* ip = soGetInodePointer(ih);

        uint32_t IndirectEnd = N_DIRECT * RPB;
        uint32_t DoubleIndirectEnd = N_DOUBLE_INDIRECT *RPB*RPB;

        if(fbn < N_DIRECT)
            return ip->d[fbn];

        fbn -= N_DIRECT;

        if(fbn < IndirectEnd)
            return grpGetIndirectFileBlock(ip,fbn);

        fbn -= IndirectEnd;

        if(fbn < DoubleIndirectEnd)
            return grpGetDoubleIndirectFileBlock(ip,fbn);
        
        throw SOException(EINVAL,__FUNCTION__);



    }

    /* ********************************************************* */


    static uint32_t grpGetIndirectFileBlock(SOInode * ip, uint32_t afbn)
    {
        soProbe(301, "%s(%d, ...)\n", __FUNCTION__, afbn);

        uint32_t ref_block = afbn / RPB;
        uint32_t data_index = afbn % RPB;
        uint32_t db[RPB];

        if(ip->i1[ref_block] == NullReference){
            return NullReference;
        }else{
            soReadDataBlock(ip->i1[ref_block], &db);
            return db[data_index];
        }
        
        
    }


    /* ********************************************************* */


    static uint32_t grpGetDoubleIndirectFileBlock(SOInode * ip, uint32_t afbn)
    {
        soProbe(301, "%s(%d, ...)\n", __FUNCTION__, afbn);

        uint32_t db[RPB];
        uint32_t ref_block1 = afbn /(RPB*RPB);
        uint32_t ref_block2 = afbn /RPB -(ref_block1*RPB);
        uint32_t data_index = afbn % RPB;
        
        if(ip->i2[ref_block1] == NullReference)
            return NullReference;
        else{
            soReadDataBlock(ip->i2[ref_block1],&db);
            if(db[ref_block2] == NullReference){
                return NullReference;
            }else{
                soReadDataBlock(db[ref_block2],&db);
                return db[data_index];
            }
            
        }
        
    }

};

