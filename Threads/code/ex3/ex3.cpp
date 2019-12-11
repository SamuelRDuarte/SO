
#define PoolSize 10

typedef struct ServiceRequest{
    char data[100];
} ServiceRequest;

typedef struct ServiceResponse{
    unsigned int size;
    unsigned int numbers;
    unsigned int letras;
} ServiceResponse;

typedef struct Service{
    ServiceRequest resquest;
    ServiceResponse response;
} Service;

Service pool[PoolSize];

class Fifo
{
private:
    unsigned int id;
    unsigned int size;
public:
    Fifo(unsigned int size);
    ~Fifo();
    insert(unsigned int id);
    retrived();
};

Fifo::Fifo(unsigned int size)
{
}

Fifo::~Fifo()
{
}


void callService(ServiceRequest & req, ServiceResponse & res){

}




