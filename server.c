#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>

typedef struct thData
{
    int idThread;
    int cl;
} thData;

void ProgramTrenuri(void *arg)
{   
    struct thData tdL;
    tdL = *((struct thData *)arg);
    char msg[300];
    strcpy(msg,"Program Trenuri");
    if (write(tdL.cl, msg, 300) <= 0)
    {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
    
};
void StatusSosiri(void *arg)
{   
    struct thData tdL;
    tdL = *((struct thData *)arg);
    char msg[300];
    strcpy(msg,"Comanda 1");
    if (write(tdL.cl, msg, 300) <= 0)
    {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
}
void StatusPlecari(void *arg)
{   
    struct thData tdL;
    tdL = *((struct thData *)arg);
    char msg[300];
   strcpy(msg,"Comanda 2");
    if (write(tdL.cl, msg, 300) <= 0)
    {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
}
static void *treat(void *arg)
{  
    struct thData tdL;
    int nr;
    while (1)
    {

        tdL = *((struct thData *)arg);
        if (read(tdL.cl, &nr, sizeof(int)) <= 0)
        {
            printf("[Thread %d]\n", tdL.idThread);
            perror("Eroare la read() de la client.\n");
        }
        if(nr==1)
        {
            StatusSosiri((struct thData *)arg);
   
        }
       else if(nr==2)
        {
            StatusPlecari((struct thData *)arg);
       
        }
        else if(nr==3){
            ProgramTrenuri((struct thData*)arg);
        }
       else if(nr==4)
        {
            shutdown(tdL.cl, SHUT_RDWR);
            close((intptr_t)arg);
            return (NULL);
        }
        
    }
    close((intptr_t)arg);
    return (NULL);
};

int main()
{   int PORT;
    int i = 0;
    printf("\nIntroduceti PORT-ul dorit pentru a porni serverul: ");
    scanf("%d", &PORT);
    struct sockaddr_in server;
    struct sockaddr_in from;
    int sd; //socket
    pthread_t th[100];

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Eroare la socket().\n");
        return errno;
    }
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server]Eroare la bind().\n");
        return errno;
    }
    if (listen(sd, 2) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return errno;
    }

    while (1)
    {
        int client;

        thData *td;
        socklen_t length = sizeof(from);
        printf("Port->%d\n", PORT);
        fflush(stdout);

        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }
        td = (struct thData *)malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;
        pthread_create(&th[i], NULL, &treat, td);

    }
}
