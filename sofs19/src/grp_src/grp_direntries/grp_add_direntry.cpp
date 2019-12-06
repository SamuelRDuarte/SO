#include "direntries.h"

#include "core.h"
#include "dal.h"
#include "fileblocks.h"
#include "bin_direntries.h"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

namespace sofs19
{
    void grpAddDirEntry(int pih, const char *name, uint32_t cin)
    {
        soProbe(202, "%s(%d, %s, %u)\n", __FUNCTION__, pih, name, cin);

        if (!strcmp(name, "")) {
            throw SOException(EINVAL, __FUNCTION__);
        }

        if (strlen(name) > SOFS19_MAX_NAME) {
            throw SOException(ENAMETOOLONG, __FUNCTION__);
        }

        SOInode* p = soGetInodePointer(pih);
        if(!S_ISDIR(p->mode)){
            throw SOException(ENOTDIR,__FUNCTION__);
        }

        int rowVazia = -1;
        int BIVazia = -1; //Block Index
        SODirEntry rowBlockVazia[DPB];


        SODirEntry d[DPB];
        uint32_t i = 0;
        for (; i < (p->size / BlockSize); i++ ) {

            soReadFileBlock(pih, i, d);

            uint32_t j = 0;
            for (; j < DPB; j++) {
                if (rowVazia < 0 && d[j].name[0] == '\0') {
                    soReadFileBlock(pih, i, rowBlockVazia);
                    BIVazia = i;
                    rowVazia = j;
                }
                
                //Error EEXIST should be thrown if name already exists 
                if (!strcmp(d[j].name, name)) {
                    throw SOException(EEXIST,__FUNCTION__);
                }
            }
        }

        if (rowVazia >= 0) {

            memcpy(rowBlockVazia[rowVazia].name, name, SOFS19_MAX_NAME+1);
            memcpy(&rowBlockVazia[rowVazia].in, &cin, sizeof(uint32_t));
            soWriteFileBlock(pih, BIVazia, rowBlockVazia);
        }			
        else {

            if (soGetFileBlock(pih, i) == NullReference){
                soAllocFileBlock(pih, i);
            }

            SODirEntry dirEntry[DPB];
            memset(dirEntry,0,BlockSize);
            for(uint32_t i = 0; i < DPB; i++){
                dirEntry[i].in = NullReference;
            }
            memcpy(dirEntry[0].name, name, SOFS19_MAX_NAME+1);
            dirEntry[0].in = cin;
            p->size += BlockSize;

            soWriteFileBlock(pih, i, dirEntry);
        }
    }
};


