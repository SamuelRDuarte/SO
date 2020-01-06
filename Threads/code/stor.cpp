#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>       //number of items stored; conditionalvariable
#include <errno.h>
//#include <bits/types.h>
#include <pthread.h>
#include "thread.h"

typedef __uint32_t uint32_t;

typedef struct ServiceRequest
{
    char data[100];
} ServiceRequest;

typedef struct ServiceResponse{
    uint32_t byte_count;
    uint32_t alpha_count;
    uint32_t digit_count;
} ServiceResponse;

typedef struct ServiceStruct
{
    ServiceRequest request;
    ServiceResponse response;
} ServiceStruct;

#define POOL_SIZE 10

class FIFO
{
private:
    uint32_t ids[POOL_SIZE];
    uint32_t ii; //point of insertion
    uint32_t ri; //point of retrival
    uint32_t cnt;
    pthread_cond_t notEmpty;
    pthread_mutex_t locker;

    void lock(){
        pthread_mutex_lock(&locker);
    }
    
    void unlock(){
        pthread_mutex_unlock(&locker);
    }

public:
    FIFO(){
        ii = ri = cnt = 0;
        locker = PTHREAD_MUTEX_INITIALIZER;
        notEmpty = PTHREAD_COND_INITIALIZER;
        unlock();
    } 

    void insert(uint32_t id){
        lock();

        ids[ii] = id;
        ii = (ii + 1) % POOL_SIZE;
        cnt++;
        pthread_cond_signal(&notEmpty);
        unlock();
    }  

    uint32_t retrieve(){
        lock();

        /*wait while FIFO is empty*/
        while(cnt == 0){
            pthread_cond_wait(&notEmpty, &locker);
        }

        uint32_t id = ids[ri];
        ids[ri] = 0xFFFFFFFF;
        ri = (ri + 1) % POOL_SIZE;
        cnt--;
        unlock();
        return id;
    }
};

class Service
{
private:
    ServiceStruct pool[POOL_SIZE];
    bool hasResponse[POOL_SIZE];
    pthread_cond_t sync[POOL_SIZE];
    pthread_mutex_t locker;
    FIFO freeSlots;
    FIFO pendingRequests;
    uint32_t ptime;
public:
    Service( uint32_t _ptime = 10){

        locker = PTHREAD_MUTEX_INITIALIZER;
        for(int i = 0; i < POOL_SIZE; i++){
            freeSlots.insert(i);
            hasResponse[i] = false;
            sync[i] = PTHREAD_COND_INITIALIZER;
        }
        ptime = _ptime;
        unlock();
    }

    void lock(){
        pthread_mutex_lock(&locker);
    }

    void unlock(){
        pthread_mutex_unlock(&locker);
    }

    /* client wait for a response */
    void waitForResponse(uint32_t id){
        lock();
        while (hasResponse[id] == false)
        {
            pthread_cond_wait(&sync[id], &locker);
        }
        hasResponse[id] = false;
        unlock();
        
    }
    //server notifies that a response is available
    void signalResponseIsAvailable(uint32_t id){
        lock();
        hasResponse[id] = true;
        pthread_cond_signal(&sync[id]);
        unlock();
    }

    //function called by function processService to compute the response to a requested service
    void computeResponse(ServiceRequest &req, ServiceResponse &res){
        //clear responding structure
        res.byte_count = res.alpha_count = res.digit_count = 0;
        
        //compute the response
        for (char *p = req.data; *p != '\0'; p++)
        {
            res.byte_count++;
            if(isdigit(*p)) res.digit_count++;
            else if(isalpha(*p)) res.alpha_count++;
        }
        
    }

    //FUNCTION CALLED BY CLIENTS TO PROCESS A REQUEST
    void callService(uint32_t cid, ServiceStruct & data){
        fprintf(stderr, "Client %u: callService with string \"%s\"\n",cid, data.request.data);

        //get a free slot in the FIFO
        uint32_t id = freeSlots.retrieve();
        //store the request in the pool of requests
        pool[id].request = data.request;
        // insert the slot id in the FIFO of pending requests
        pendingRequests.insert(id);
        //wait for the response
        waitForResponse(id);
        data.response = pool[id].response;
        //free the slot
        freeSlots.insert(id);
    }

    /* FUNCTION CALLED BY SERVERS TO PROCESS REQUESTED SERVICES */
    void processService(uint32_t sid){
        fprintf(stderr, "Server %u\n", sid);
        //retrieve a pending request from the FIFO
        uint32_t id = pendingRequests.retrieve();
        //compute the response
        computeResponse(pool[id].request, pool[id].response);
        //signal the client that the response is available
        signalResponseIsAvailable(id);

        fprintf(stderr, "[s, %u] processed slot %u\n",sid,id);
    }
};




