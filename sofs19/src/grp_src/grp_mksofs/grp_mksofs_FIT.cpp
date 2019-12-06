#include "grp_mksofs.h"

#include "rawdisk.h"
#include "core.h"
#include "bin_mksofs.h"

#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <dal.h>

namespace sofs19
{
    void grpFillInodeTable(uint32_t itotal, bool set_date)
    {
        soProbe(604, "%s(%u)\n", __FUNCTION__, itotal);

        uint32_t  nBlocks = itotal/IPB;
        SOInode inode[IPB];

        for (uint32_t blkID= 1; blkID < nBlocks+1; ++blkID) {
            for (int i = 0; i < IPB; ++i) {
                inode[i].mode = INODE_FREE;
                inode[i].owner = 0;
                inode[i].group = 0;
                inode[i].size = 0;
                inode[i].blkcnt = 0;
                inode[i].lnkcnt = 0;

                for (int j = 0; j < N_DIRECT; ++j) {
                    inode[i].d[j] = NullReference;
                }
                for (int j = 0; j < N_INDIRECT; ++j) {
                    inode[i].i1[j] = NullReference;
                }
                for (int j = 0; j < N_DOUBLE_INDIRECT; ++j) {
                    inode[i].i2[j] = NullReference;
                }

                if (blkID == 1){
                    inode[0].mode = S_IFDIR | S_IREAD | S_IRWXG | S_IROTH | S_IXOTH;
                    inode[0].owner = getuid();
                    inode[0].group = getgid();
                    inode[0].size = BlockSize;
                    inode[0].blkcnt = 1;
                    inode[0].lnkcnt = 2;

                    if(set_date)
                        inode[0].atime = inode[0].ctime = inode[0].mtime = time(NULL);
                    else
                        inode[0].atime = inode[0].ctime = inode[0].mtime = 0;

                    inode[0].d[0] = 0;

                }


            }
            soWriteRawBlock(blkID, &inode);
        }

    }
};

