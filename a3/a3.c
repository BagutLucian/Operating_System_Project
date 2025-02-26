#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define RESP "RESP_PIPE_36235"
#define REQ "REQ_PIPE_36235"


int main() 
{
    // voi crea pipe-ul cu nume 
    if (mkfifo(RESP, 0664) == -1) 
    {
        perror("Eroare la crearea pipe-ului cu nume RESP");
        printf("ERROR\n");
        return 1;
    }
    else
    printf("S-a creat resp\n");

    // deschidem pipe-ul cu nume REQ pentru citire (cel de la tester)
    int req = open(REQ, O_RDONLY);
    if (req == -1) 
    {
        perror("Nu s-a putut deschide pipe-ul req");
        printf("ERROR\n");
        unlink(RESP); //dam unlink daca nu am putut deschide req 
        return 1;
    }
    else
    printf("s-a deschis req pentru citire\n");

    // deschidem pipe-ul resp (response de la implementare) pentru citire 
    int resp = open(RESP, O_WRONLY);
    if (resp == -1) 
    {
        perror("Nu s-a putut deschide pipe-ul resp pentru citire din implementare");
        printf("ERROR\n");
        close(req); //inchidem req ca nu mai avem la ce sa dam request
        unlink(RESP);
        return 1;
    }
    else
    printf("S-a deschis resp pt scriere\n");

    // scriem CONNECT in pipe-ul resp
    
    if (write(resp, "CONNECT$", strlen("CONNECT$")) == -1) 
    {
        perror("Nu s-a putut scrie mesajul CONNECT in pipe");
        printf("ERROR\n");
        close(req);
        close(resp);
        unlink(RESP);
        return 1;
    }
    else
    printf("am scris CONNECT cu succes\n");

    printf("SUCCESS\n");

    unsigned int *ptr = NULL;

    // loop pentru bucla ce repeta procesul de request-response
    while (1) 
    {
        char buffer[256];
        read(req, buffer, sizeof(buffer));

        if(strncmp(buffer,"PING$",sizeof("PING$"))==0)
        {
          write(resp,"PING$",5);
          unsigned int aux=36235;
          write(resp,&aux,sizeof(unsigned int));
          write(resp,"PONG$",5);
        }
       else if (strncmp(buffer, "CREATE_SHM$", strlen("CREATE_SHM$")) == 0)
        {
            write(resp,"CREATE_SHM$",11);
            int fd;
            char success[] = "SUCCESS$";
            char error[] = "ERROR$";
            unsigned int size = 2890165;

            fd = shm_open("/erxWVnRU", O_CREAT | O_RDWR, 0664);
            int rez = ftruncate(fd, size);
            int *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

            if (fd == -1)
            {
                write(resp, error, strlen(error));
            }
            else if (rez == -1)
            {
                write(resp, error, strlen(error));
            }

            else if (&ptr == MAP_FAILED)
            {
                write(resp, error, strlen(error));
            }
            
            write(resp, success, strlen(success));
            return 1;
        }else if (strncmp(buffer, "WRITE_TO_SHM$", strlen("WRITE_TO_SHM$")) == 0) 
        {
            write(resp, "WRITE_TO_SHM$", 13);

            unsigned int offset, value;
            unsigned int size = 2890165;
            read(req, &offset, sizeof(unsigned int));
            read(req, &value, sizeof(unsigned int));

            if (offset < size && (offset + sizeof(unsigned int)) <= size) 
            {
                ptr[offset / sizeof(unsigned int)] = value;
                write(resp, "SUCCESS$", 8);
            } else {
                write(resp, "ERROR$", 6);
            }
        }
       else if(strncmp(buffer,"EXIT$",sizeof("EXIT$"))==0)
       {
         // la final , inchidem toate pipe-urile
         close(req);
         close(resp);
         unlink(RESP);
         break;
       }
    }
    

    return 0;
}

