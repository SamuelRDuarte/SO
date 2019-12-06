/*
 *  \author António Rui Borges - 2012-2015
 *  \authur Artur Pereira - 2016-2019
 */

#include "grp_freelists.h"

#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include <iostream>

#include "core.h"
#include "dal.h"
#include "freelists.h"
#include "bin_freelists.h"

namespace sofs19
{
    uint32_t grpAllocInode(uint32_t type, uint32_t perm)
    {
        soProbe(401, "%s(0x%x, 0e%03o)\n", __FUNCTION__, type, perm);

        if((type != S_IFREG ) and (type != S_IFDIR) and (type != S_IFLNK))
            throw SOException(EINVAL, __FUNCTION__);

        if ((perm | 0x777) != 0x777)
            throw SOException(EINVAL, __FUNCTION__);
        
        SOSuperBlock *sbp = soGetSuperBlockPointer();

        if (sbp->ifree == 0)
            throw SOException(ENOSPC, __FUNCTION__);

        uint32_t in = sbp->ihead;

        int ih = soOpenInode(in);
        SOInode *ip = soGetInodePointer(in);

        sbp->ifree++;
        sbp->ihead = ip->next;
        if(sbp->ifree == 0)
            sbp->itail = NullReference;

        soSaveSuperBlock();

        memset(ip, 0, sizeof(SOInode));
        ip->mode = type | perm;
        ip->owner = getuid();
        ip->group = getgid();
        ip->atime = ip->atime = ip->mtime = time(NULL);
        for(unsigned int i = 0; i< N_DIRECT; i++)
            ip->d[i] = NullReference;
        for(unsigned int i = 0; i< N_INDIRECT; i++)
            ip->i1[i] = NullReference;
        for(unsigned int i = 0; i< N_DOUBLE_INDIRECT; i++)
            ip->i2[i] = NullReference;

        return in;
    }
};

