#include "direntries.h"

#include "core.h"
#include "dal.h"
#include "fileblocks.h"
#include "bin_direntries.h"

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

namespace sofs19
{
void grpRenameDirEntry(int pih, const char *name, const char *newName)
{
    soProbe(204, "%s(%d, %s, %s)\n", __FUNCTION__, pih, name, newName);

    /* change the following line by your code */
    //binRenameDirEntry(pih, name, newName);

    SOInode *ip = soGetInodePointer(pih);

    if (!strcmp(newName, ""))
    {
            throw SOException(EINVAL,__FUNCTION__));
    }

    if (strlen(newName) > SOFS19_MAX_NAME)
    {
        throw SOException(ENAMETOOLONG, __FUNCTION__);
    }

    if (!S_ISDIR(ip->mode))
    {
        throw SOException(ENOTDIR, __FUCTION__);
    }

    int toName = -1;
    int toNamei = -1;

    SODirEntry rblock[DirentriesPerBlock];
    SODirEntry d[DirentriesPerBlock];

    uint32_t i = 0;
    for (; i < (ip->size / BlockSize); i++){

        soReadFileBlock(pih, i, d);

        uint32_t j = 0;
        for (; j < DirentriesPerBlock; j++)
        {
            if (toName < 0 && strcmp(d[j].name, name) == 0)
            {
                soReadFileBlock(pih, i, rblock);
                toNamei = i;
                toName = j;
            }

            if (strcmp(d[j].name, newName) == 0)
            {
                throw SOException(EEXIST, __FUNCTION__);
            }
        }
    }

    if (toName >= 0)
    {

        memcpy(rblock[toName].name, name, SOFS19_MAX_NAME);
        soWriteFileBlock(pih, toNamei, rblock);
    }

    if (toName < 0)
    {
        throw SOException(ENOENT, __FUNCTION__);
    }
}
} // namespace sofs19