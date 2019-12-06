//#include "../../../include/grp/grp_fileblocks.h"
#include "grp_fileblocks.h"

/*
#include "../../../include/freelists.h"
#include "../../../include/dal.h"
#include "../../../include/core.h"
#include "../../../include/bin/bin_fileblocks.h"
*/
#include "freelists.h"
#include "dal.h"
#include "core.h"
#include "bin_fileblocks.h"


#include <inttypes.h>
#include <errno.h>
#include <assert.h>

namespace sofs19
{
//#if false
    /* free all blocks between positions ffbn and RPB - 1
     * existing in the block of references given by i1.
     * Return true if, after the operation, all references become NullReference.
     * It assumes i1 is valid.
     */
    static bool grpFreeIndirectFileBlocks(SOInode * ip, uint32_t i1, uint32_t ffbn);

    /* free all blocks between positions ffbn and RPB**2 - 1
     * existing in the block of indirect references given by i2.
     * Return true if, after the operation, all references become NullReference.
     * It assumes i2 is valid.
     */
    static bool grpFreeDoubleIndirectFileBlocks(SOInode * ip, uint32_t i2, uint32_t ffbn);
//#endif

    /* ********************************************************* */
    /*

    Free all file blocks from the given position on.

    Parameters
        ih	inode handler
        ffbn	first file block number

    Remarks

        Assumes ih is a valid handler of an inode in use opened;
        data blocks used to store references that become empty are also freed;
        when calling a function of any layer, the version with prefix so is used.
    */
    uint32_t removedBlocks = 0;                                 // contar numero de blocos removidos para depois adicionalos na tail cache

    void grpFreeFileBlocks(int ih, uint32_t ffbn)
    {
        soProbe(303, "%s(%d, %u)\n", __FUNCTION__, ih, ffbn);

        /* change the following line by your code */
        //binFreeFileBlocks(ih, ffbn);

        SOInode* ip = soGetInodePointer(ih);                      // inodepointer to inodehandler                                                      
        uint32_t RPBI2 = RPB * RPB;                                 // references per block em duplamente indireto (i2 guarda RPB blocos i1, que cada um guarda RPB blocos)
        uint32_t i2Start = N_INDIRECT * RPB + N_DIRECT;             // i2 começa no fim do i1, que é N_INDIRECT (blocos no i1) * RPB (nº blocos em cada bloco do i1) + d[*]
        uint32_t i2End = RPBI2 * N_DOUBLE_INDIRECT + i2Start;       // i2 acaba no inicio do i2 (i2start) + N_DOUBLE_INDIRECT (nºblocos no i2) * RPBI2 (nºblocos dentro de cada i2)

        // Error \c EINVAL is thrown if \c fbn is not valid; - fileblocks.h
        if (ffbn < 0 || ffbn >= i2End) {
			throw SOException(EINVAL, __FUNCTION__);
		}

        // Caso ffbn < N_DIRECT => estamos a apagar blocos no d[*]
        if (ffbn < N_DIRECT) {
        	for (int i = ffbn; i < N_DIRECT; i++){
        		if (ip->d[i] != NullReference) {                    // caso esta posição tenha conteudo, irei na proxima linha apagar o conteudo colocando NullReference
                    soFreeDataBlock(ip->d[i]);
                    ip->d[i]=NullReference;
                    removedBlocks++;
				}
            }
            ffbn = N_DIRECT;                                         // depois de apagar os diretos, vamos apagar os i1
        }

        if (ffbn >= N_DIRECT && ffbn < i2Start) {

            // free indirect list
                                                                // i1[*] = (nil) 30 31 (nil) (nil) (nil) (nil) (nil)
            
            uint32_t index = (ffbn - N_DIRECT) / RPB;
            uint32_t resto = (ffbn - N_DIRECT) % RPB;
            
            for(int i=index; i < N_INDIRECT; i++){
                bool apagarBlock = false;
                if(ip->i1[i] != NullReference){
                    if(i == index){
                        apagarBlock = grpFreeIndirectFileBlocks(ip, ip->i1[i], resto);
                    }
                    else{
                        apagarBlock = grpFreeIndirectFileBlocks(ip, ip->i1[i], 0);
                    }
                    if(apagarBlock){
                        soFreeDataBlock(ip->i1[i]);
                        ip->i1[i] = NullReference;
                        removedBlocks++;
                    }                      
                }
            }
            ffbn = i2Start;                                         // depois de apagar os i1[*], vamos apagar os i2[*]
        }

        if (ffbn >= i2Start && ffbn < i2End) {
            // free double indirect list

            uint32_t index = (ffbn-N_DIRECT-N_INDIRECT*RPB)/(RPBI2);
            uint32_t resto = (ffbn-N_DIRECT-N_INDIRECT*RPB)%(RPBI2);
            for(int i=index ; i<N_DOUBLE_INDIRECT ; i++){
                bool apagarBlock = false;
                if(ip->i2[i] != NullReference){
                    if(i == index){
                        apagarBlock = grpFreeDoubleIndirectFileBlocks(ip,ip->i2[i],resto);
                    }
                    else{
                        apagarBlock = grpFreeDoubleIndirectFileBlocks(ip,ip->i2[i],0);
                    }
                    if(apagarBlock){
                        soFreeDataBlock(ip->i2[i]);
                        ip->i2[i] = NullReference;
                        removedBlocks++;
                    }
                }
            }
        }
        
        ip->blkcnt -= removedBlocks;
        soSaveInode(ih);

    }
 
    /* ********************************************************* */

//#if false
    static bool grpFreeIndirectFileBlocks(SOInode * ip, uint32_t i1, uint32_t ffbn)
    {
        soProbe(303, "%s(..., %u, %u)\n", __FUNCTION__, i1, ffbn);

        /* change the following line by your code */
        //throw SOException(ENOSYS, __FUNCTION__); 
        bool apagarBlock = true;

        uint32_t buffer[RPB];
        soReadDataBlock(i1,buffer);                          // leio conteudo do i1 para um buffer
        for(int i=0 ; i<RPB ; i++){
            if(i>=ffbn){                                    // apenas apago para a frente do ffbn
                if(buffer[i] != NullReference){             // se a referencia dentro do i1 nao for nullreference
                    soFreeDataBlock(buffer[i]);
                    buffer[i] = NullReference;
                    removedBlocks++;
                }
            }
            if(buffer[i] != NullReference){                 // caso o ffbn seja maior que 0, irá haver blocos não null
                apagarBlock = false;                        // por isso não podemos apagar o bloco
            }
        }
        soWriteDataBlock(i1,buffer);                         // atualizo o conteudo do bloco
        return apagarBlock;
        
    }
//#endif

    /* ********************************************************* */

//#if false
    static bool grpFreeDoubleIndirectFileBlocks(SOInode * ip, uint32_t i2, uint32_t ffbn)
    {
        soProbe(303, "%s(..., %u, %u)\n", __FUNCTION__, i2, ffbn);

        /* change the following line by your code */
        //throw SOException(ENOSYS, __FUNCTION__); 
        bool apagarBlocki2 = true;
        uint32_t buffer[RPB];
        soReadDataBlock(i2,buffer);

        uint32_t index = ffbn/RPB;                         // para saber em qual i1 começar
        uint32_t resto = ffbn%RPB;                         // saber em qual posiçao do index começar a apagar
        for(int i=0 ; i<RPB ; i++){                   // i1's guardados dentro do i2
            if(buffer[i] != NullReference){       // se o i1 nao for null
                bool apagarBlocki1 = false;
                if(i==index){                                                               // se este i1 for onde temos de começar a remover blocks
                    apagarBlocki1 = grpFreeIndirectFileBlocks(ip,buffer[i],ffbn%RPB);
                }
                if(i>=index){                                                               // se este for um i1 que esteja depois do i1 onde começamos a remover blocks
                    apagarBlocki1 = grpFreeIndirectFileBlocks(ip,buffer[i],0);
                }
                if(apagarBlocki1){                                                          // se o i1 ficou vazio, podemos apagar esse i1                     
                    soFreeDataBlock(buffer[i]);
                    buffer[i] = NullReference;
                    removedBlocks++;
                }
                if(buffer[i] != NullReference){                                             // caso em que começamos a remover blocks, por exemplo, no segundo i1 dentro do i2
                    apagarBlocki2 = false;                                                  // se o primeiro i1 não for nullreference, ele nao irá entrar em nenhum dos 3 ifs acima
                }                                                                           // e como ele nao fica null, o i2 em si não ficará vazio, logo nao podemos libertar o i2
            }
            
        }
        soWriteDataBlock(i2,buffer);
        return apagarBlocki2;
    }
//#endif

    /* ********************************************************* */
};

