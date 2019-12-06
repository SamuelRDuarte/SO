/*
 *  \author António Rui Borges - 2012-2015
 *  \authur Artur Pereira - 2016-2019
 */

//#include "../../../include/grp/grp_freelists.h"
#include "grp_freelists.h"

#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

//#include "../../../include/core.h"
//#include "../../../include/dal.h"
//#include "../../../include/freelists.h"
//#include "../../../include/bin/bin_freelists.h"
#include "core.h"
#include "dal.h"
#include "freelists.h"
#include "bin_freelists.h"

namespace sofs19
{
    void grpFreeInode(uint32_t in)
    {
        soProbe(402, "%s(%u)\n", __FUNCTION__, in);

        //binFreeInode(in);

        /* saber qual o tail da lista IT, informaçao da tail está no superblock
        abrir esse inode, e meter como next o inode que está a ser libertado, 
        e o next do inode que está a ser libertado é NULLREFERENCE
        */

        SOSuperBlock *sbp = soGetSuperBlockPointer();

        //if(in > sbp->itotal){                     // caso esteja a tentar libertar um inode superior aos disponiveis
        //    throw SOException(ENOSPC, __FUNCTION__);
        //}

        // 'LIMPAR' O INODE QUE VAI SER LIBERTADO
        int ih = soOpenInode(in);                   // abrir o inode que está a ser libertado
        SOInode *ip = soGetInodePointer(ih);        // pointer para o inode que está a ser libertado

        ip ->mode = INODE_FREE;                     // inode mode: it stores the file type and permissions
        ip ->size = 0;                              // file size in bytes
        ip ->owner = 0;                             // user ID of the file owner
        ip ->group = 0;                             // group ID of the file owner
        ip ->mtime = ip -> ctime = 0;               // time of last change to file and inode information 
        ip ->next = NullReference;                  // next free inode

        soSaveInode(ih);
        soCloseInode(ih);

        // CASO NÃO HAJA FREE INODES, O QUE VAI SER LIBERTADO VAI SER O UNICO, LOGO SERÁ O PRIMEIRO E O ULTIMO FREE INODE
        if(sbp->ifree == 0){                        // if number of free inodes == 0
            sbp->ihead = sbp->itail = in;           // number of first and last free inode
        }

        // CASO HAJA FREE INODES, O QUE VAI SER LIBERTADO 
        else{                                       // caso haja free inodes
            int ih2 = soOpenInode(sbp->itail);      // ih2 = last free inode
            SOInode *ip2 = soGetInodePointer(ih2); 
            ip2->next = ih;                         // o next do last free inode vai ser o que está a ser libertado
            soSaveInode(ih2);                       
            soCloseInode(ih2);
            sbp->itail = in;                        // update, last free inode é agora o inode que foi libertado [ihead ..... ih2 ih]

        }
        sbp->ifree++;

        soSaveSuperBlock();

    }

};

