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
#include "pugixml.hpp"

typedef struct threadInfo
{
    int idThread;
    int client;
} threadInfo;

void ProgramTrenuri(void *arg)
{
    printf("program\n");
    struct threadInfo thread;
    thread = *((struct threadInfo *)arg);
    char msg[10000];
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("program.xml");
    // printf("xml1\n");
    strcpy(msg, "Program Trenuri:\n");
    if (result)
    {

        for (pugi::xml_node tren = doc.child("Program").child("Tren"); tren; tren = tren.next_sibling("Tren"))
        {
            strcat(msg, "Tren ");
            strcat(msg, tren.attribute("name").value());
            strcat(msg, " :\n");

            for (pugi::xml_node statie = tren.child("Statii").child("Statie"); statie; statie = statie.next_sibling("Statie"))
            {
                strcat(msg, "Statia: ");
                strcat(msg, statie.attribute("name").value());
                strcat(msg, ", Sosire: ");
                strcat(msg, statie.attribute("oraS").value());
                strcat(msg, ", Plecare: ");
                strcat(msg, statie.attribute("oraP").value());
                strcat(msg, ", Intarziere: ");
                strcat(msg, statie.attribute("Tintarziere").value());
                strcat(msg, "\n");
            }

            strcat(msg, "\n");
        }

        strcat(msg, "\0");
        if (write(thread.client, msg, sizeof(msg)) <= 0)
        {
            printf("[Thread %d] ", thread.idThread);
            perror("[Thread]Eroare la write() catre client.\n");
        }
    }
    else
    {
         printf("XML [ ] parsed with errors, attr value: %s\n" ,doc.child("node").attribute("attr").value());
         printf("Error description: %s",result.description() );
       
    }
}
void StatusSosiri(void *arg)
{
    printf("status sosiri\n");
    struct threadInfo thread;
    thread = *((struct threadInfo *)arg);
    char msg[300];
    strcpy(msg, "Comanda 1");
    if (write(thread.client, msg, 300) <= 0)
    {
        printf("[Thread %d] ", thread.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
}
void StatusPlecari(void *arg)
{
    struct threadInfo thread;
    thread = *((struct threadInfo *)arg);
    char msg[300];
    strcpy(msg, "Comanda 2");
    if (write(thread.client, msg, 300) <= 0)
    {
        printf("[Thread %d] ", thread.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
}
static void *treat(void *arg)
{
    struct threadInfo thread;
    int nr;
    while (1)
    {

        thread = *((struct threadInfo *)arg);
        if (read(thread.client, &nr, sizeof(int)) <= 0)
        {
            printf("[Thread %d]\n", thread.idThread);
            perror("Eroare la read() de la client.\n");
        }
        printf("thread citire\n");
        if (nr == 1)
        {
            StatusSosiri((struct threadInfo *)arg);
        }
        else if (nr == 2)
        {
            StatusPlecari((struct threadInfo *)arg);
        }
        else if (nr == 3)
        {
            ProgramTrenuri((struct threadInfo *)arg);
        }
        else if (nr == 4)
        {
            shutdown(thread.client, SHUT_RDWR);
            close((intptr_t)arg);
            return (NULL);
        }
    }
    close((intptr_t)arg);
    return (NULL);
};

int main()
{
    int PORT;
    printf("\nIntroduceti PORT-ul dorit pentru a porni serverul: ");
    scanf("%d", &PORT);
    struct sockaddr_in server;
    struct sockaddr_in from;
    int sd;
    pthread_t th[100];

    // pugi::xml_document doc;
    // pugi::xml_parse_result result = doc.load_file("initial.xml");
    // doc.save_file("program.xml");

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
    int i = 0;
    while (1)
    {
        int client;

        threadInfo *thread;
        socklen_t length = sizeof(from);
        printf("Port->%d\n", PORT);
        fflush(stdout);

        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }
        thread = (struct threadInfo *)malloc(sizeof(struct threadInfo));
        thread->client = client;
        thread->idThread = i++;
        printf("client acceptat\n");
        pthread_create(&th[i], NULL, &treat, thread);
    }
}
