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
#include <mutex>

typedef struct threadInfo
{
    int idThread;
    int client;
    bool admin;
} threadInfo;
typedef struct Clients
{
    int client;
    pid_t pid;
    pthread_t thread;
} Clients;
typedef struct info
{
    char statie[30];
    char tren[5];
    char intarziere[5];
} info;
Clients clients[100];
pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;
std::mutex xmlMutex;

void ProgramTrenuri(char *msg)
{
    // printf("program\n");
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("program.xml");
    // printf("xml1\n");
    strcpy(msg, "\n\n//////////////////////////////////////\n\nProgram Trenuri:\n");
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

                if (strcmp(statie.attribute("id").value(), "first"))
                {
                    strcat(msg, statie.attribute("ora_sosire").value());
                }
                else
                    strcat(msg, "-");

                strcat(msg, ", Plecare: ");

                if (strcmp(statie.attribute("id").value(), "last"))
                {
                    strcat(msg, statie.attribute("ora_plecare").value());
                }
                else
                    strcat(msg, "-");

                strcat(msg, ", Intarziere: ");
                strcat(msg, statie.attribute("intarziere").value());
                strcat(msg, " min");
                strcat(msg, "\n");
            }

            strcat(msg, "\n");
        }

        strcat(msg, "\n\n//////////////////////////////////////////////////\n\n");
    }
    else
    {
        printf("XML [ ] parsed with errors, attr value: %s\n", doc.child("node").attribute("attr").value());
        printf("Error description: %s", result.description());
    }
}
void StatusSosiri(info infos, char *msg)
{
    std::lock_guard<std::mutex> lock(xmlMutex);
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("program.xml");
    bool statie_gasita = false, tren_sosire = false;

    if (result)
    {
        // actualizam ora trecuta in fisier cu ora locala
        time_t current_time;
        struct tm *time_info;
        char data[30], ora[10];

        time(&current_time);
        time_info = localtime(&current_time); // schimbam tipul de date
        strcpy(data, asctime(time_info));     // schimbam formatul

        int index = 0, i;
        for (i = 11; i <= 15; i++)
            ora[index++] = data[i];
        ora[index] = '\0';
        pugi::xml_node node = doc.child("Program");
        pugi::xml_attribute attr = node.attribute("time");
        attr = ora;

        doc.save_file("program.xml");

        strcpy(msg, "\n\n//////////////////////////////////////////////\n\nSosiri in statia: ");
        strcat(msg, infos.statie);
        strcat(msg, "\n");
        for (pugi::xml_node tren = doc.child("Program").child("Tren"); tren; tren = tren.next_sibling("Tren"))
        {
            for (pugi::xml_node station = tren.child("Statii").child("Statie"); station; station = station.next_sibling("Statie"))
            {
                bool ok;
                int ora1, ora2, min1, min2;
                char ora_tren[10], intarziere[10];
                strcpy(ora_tren, station.attribute("ora_sosire").value());
                strcpy(intarziere, station.attribute("intarziere").value());
                ora1 = (ora[0] - '0') * 10 + (ora[1] - '0');
                min1 = (ora[3] - '0') * 10 + (ora[4] - '0');
                ora2 = (ora_tren[0] - '0') * 10 + (ora_tren[1] - '0');
                min2 = (ora_tren[3] - '0') * 10 + (ora_tren[4] - '0');
                int intarz = atoi(intarziere);
                if (intarz >= 0)
                {
                    ora2 = ora2 + (min2 + intarz) / 60;
                    min2 = (min2 + intarz) % 60;
                }
                else
                {
                    if (min2 >= -intarz)
                        min2 = min2 + intarz;
                    else
                    {
                        ora2 = ora2 - 1 + (min2 + intarz) / 60;
                        min2 = 60 + (min2 + intarz) % 60;
                    }
                }
                ora2 = ora2 % 24;
                ora1 = ora1 % 24;
                if (ora1 == 23 && ora2 == 0)
                    ora2 = 24;
                if (ora1 == ora2 - 1 && (60 - min1 + min2) <= 60)
                    ok = true;
                else if (ora1 == ora2 && (min2 - min1) >= 0 && (min2 - min1) <= 60)
                    ok = true;
                else
                    ok = false;
                // printf("\n ora min program %d %d  ora min fisier %d %d\n", ora1, min1, ora2, min2);
                // fflush(stdout);
                if (strcmp(station.attribute("name").value(), infos.statie) == 0)
                {
                    statie_gasita = true;
                    if (ok == true)
                    {
                        strcat(msg, "Tren ");
                        tren_sosire = true;
                        strcat(msg, tren.attribute("name").value());
                        if (strcmp(station.attribute("id").value(), "first"))
                        {
                            strcat(msg, "  Sosire ");
                            strcat(msg, station.attribute("ora_sosire").value());
                        }

                        strcat(msg, " Plecare ");
                        strcat(msg, station.attribute("ora_plecare").value());

                        if (strcmp(station.attribute("intarziere").value(), "0"))
                        {
                            strcat(msg, " Intarziere ");
                            strcat(msg, station.attribute("intarziere").value());
                        }
                        strcat(msg, "\n");
                    }
                }
            }
        }
    }
    else
    {
        printf("XML [ ] parsed with errors, attr value: %s\n", doc.child("node").attribute("attr").value());
        printf("Error description: %s", result.description());
    }
    if (statie_gasita == false)
        strcat(msg, "Statia nu a fost gasita\n");
    else if (tren_sosire == false)
        strcat(msg, "Niciun tren cu sosire in statia dorita in urmatoarea ora\n");
    strcat(msg, "\n\n////////////////////////////////////////////////////\n\n");
    // printf("Am executat sosiri\n");
    // fflush(stdout);
}
void StatusPlecari(info infos, char *msg)
{
    // printf("status plecari\n");
    std::lock_guard<std::mutex> lock(xmlMutex);
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("program.xml");
    bool statie_gasita = false, tren_sosire = false;

    if (result)
    {
        // actualizam ora trecuta in fisier cu ora locala
        time_t current_time;
        struct tm *time_info;
        char data[30], ora[10];

        time(&current_time);
        time_info = localtime(&current_time); // schimbam tipul de date
        strcpy(data, asctime(time_info));     // schimbam formatul

        int index = 0, i;
        for (i = 11; i <= 15; i++)
            ora[index++] = data[i];
        ora[index] = '\0';
        pugi::xml_node node = doc.child("Program");
        pugi::xml_attribute attr = node.attribute("time");
        attr = ora;
        doc.save_file("program.xml");

        strcpy(msg, "\n\n///////////////////////////////////////////////////\n\nPlecari din statia: ");
        strcat(msg, infos.statie);
        strcat(msg, "\n");
        for (pugi::xml_node tren = doc.child("Program").child("Tren"); tren; tren = tren.next_sibling("Tren"))
        {
            for (pugi::xml_node station = tren.child("Statii").child("Statie"); station; station = station.next_sibling("Statie"))
            {
                bool ok;
                int ora1, ora2, min1, min2;
                char ora_tren[10], intarziere[10];
                strcpy(ora_tren, station.attribute("ora_sosire").value());
                strcpy(intarziere, station.attribute("intarziere").value());
                ora1 = (ora[0] - '0') * 10 + (ora[1] - '0');
                min1 = (ora[3] - '0') * 10 + (ora[4] - '0');
                ora2 = (ora_tren[0] - '0') * 10 + (ora_tren[1] - '0');
                min2 = (ora_tren[3] - '0') * 10 + (ora_tren[4] - '0');
                int intarz = atoi(intarziere);
                if (intarz >= 0)
                {
                    ora2 = ora2 + (min2 + intarz) / 60;
                    min2 = (min2 + intarz) % 60;
                }
                else
                {
                    if (min2 >= -intarz)
                        min2 = min2 + intarz;
                    else
                    {
                        ora2 = ora2 - 1 + (min2 + intarz) / 60;
                        min2 = 60 + (min2 + intarz) % 60;
                    }
                }
                ora2 = ora2 % 24;
                ora1 = ora1 % 24;
                if (ora1 == 23 && ora2 == 0)
                    ora2 = 24;
                if (ora1 == ora2 - 1 && (60 - min1 + min2) <= 60)
                    ok = true;
                else if (ora1 == ora2 && (min2 - min1) >= 0 && (min2 - min1) <= 60)
                    ok = true;
                else
                    ok = false;
                // printf("\ndif min %d",min2-min1);
                // printf("\n ora min program %d %d  ora min fisier %d %d\n", ora1, min1, ora2, min2);
                // fflush(stdout);
                if (strcmp(station.attribute("name").value(), infos.statie) == 0)
                {
                    statie_gasita = true;
                    if (ok == true)
                    {
                        strcat(msg, "Tren ");
                        tren_sosire = true;
                        strcat(msg, tren.attribute("name").value());
                        strcat(msg, "  Sosire ");
                        strcat(msg, station.attribute("ora_sosire").value());
                        if (strcmp(station.attribute("id").value(), "last"))
                        {
                            strcat(msg, " Plecare ");
                            strcat(msg, station.attribute("ora_plecare").value());
                        }
                        if (strcmp(station.attribute("intarziere").value(), "0"))
                        {
                            strcat(msg, " Intarziere ");
                            strcat(msg, station.attribute("intarziere").value());
                        }
                        strcat(msg, "\n");
                    }
                }
            }
        }
    }
    else
    {
        printf("XML [ ] parsed with errors, attr value: %s\n", doc.child("node").attribute("attr").value());
        printf("Error description: %s", result.description());
    }
    if (statie_gasita == false)
        strcat(msg, "Statia nu a fost gasita\n");
    else if (tren_sosire == false)
        strcat(msg, "Niciun tren plecand din statia dorita in urmatoarea ora\n");
    strcat(msg, "\n\n////////////////////////////////////////////////////////////\n\n");
}
void send(const char msg[10000], Clients *clients)
{
    for (int i = 0; i < 100; i++)
    {
        if (clients[i].client)
        {
            if (kill(clients[i].pid, SIGUSR1) < 0)
            {
                perror("eroare trimitere semnal\n");
            }
            pthread_mutex_lock(&socket_mutex);
            if (write(clients[i].client, msg, 10000) <= 0)
            {
                perror("Eroare la write() catre clienti.");
            }
            pthread_mutex_unlock(&socket_mutex);
        }
    }
}
void Intarziere(info infos, char *rasp)
{
    std::lock_guard<std::mutex> lock(xmlMutex);
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("program.xml");
    if (result)
    {
        strcpy(rasp, "\n\n////////////////////////////////\n\nintarziere inregistrata\n\n////////////////////////////////////\n\n");
        char msg[10000];
        bzero(msg, sizeof(msg));
        strcpy(msg, "\n\n////////////////////////////////////////////////\n\nTrenul ");
        strcat(msg, infos.tren);
        strcat(msg, "  Statia ");
        strcat(msg, infos.statie);
        if (strcmp(infos.intarziere, "0") == 0)
            strcat(msg, " Ajuns conform graficului initial\n\n////////////////////////////////////////////////////////\n\n");
        else if (strstr(infos.intarziere, "-"))
        {
            strcat(msg, " Ajunge mai devreme cu ");
            strcat(msg, infos.intarziere + 1);
            strcat(msg, " min\n\n///////////////////////////////////////////////////////////////////\n\n");
        }
        else
        {
            strcat(msg, " Intarziere ");
            strcat(msg, infos.intarziere);
            strcat(msg, " min\n\n////////////////////////////////////////////////////////////////////////////\n\n");
        }
        for (pugi::xml_node tren = doc.child("Program").child("Tren"); tren; tren = tren.next_sibling("Tren"))
        {
            if (strcmp(infos.tren, tren.attribute("name").value()) == 0)
            {
                for (pugi::xml_node station = tren.child("Statii").child("Statie"); station; station = station.next_sibling("Statie"))
                {
                    if (strcmp(infos.statie, station.attribute("name").value()) == 0)
                    {
                        for (pugi::xml_node nextst = station; nextst; nextst = nextst.next_sibling("Statie"))
                        {
                            pugi::xml_attribute attr = nextst.attribute("intarziere");

                            attr.set_value(infos.intarziere);
                        }
                        doc.save_file("program.xml");
                        break;
                    }
                }
                break;
            }
        }
        send(msg, clients);
    }
    else
    {
        printf("XML [ ] parsed with errors, attr value: %s\n", doc.child("node").attribute("attr").value());
        printf("Error description: %s", result.description());
    }
}
void connection(struct threadInfo &arg)
{
    struct threadInfo thread;
    bool ad;
    if (read(arg.client, &ad, sizeof(ad)) < 0)
    {
        perror("Eroare la citirea de la client\n");
        return;
    }
    if (ad == 1)
    {
        // printf("check 1\n");
        // fflush(stdout);
        bool ok = false;
        while (ok == false)
        {
            char id[10], pd[10];
            if (read(arg.client, id, sizeof(id)) < 0)
            {
                perror("Eroare la citirea de la server\n");
                return;
            }
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_file("users.xml");
            // printf("check 2\n");
            // fflush(stdout);

            if (result)
            {
                for (pugi::xml_node user = doc.child("Admins").child("user"); user; user = user.next_sibling("user"))
                {
                    if (strcmp(id, user.attribute("id").value()) == 0)
                        ok = true;
                }
                if (write(arg.client, &ok, sizeof(ok)) < 0)
                {
                    perror("Eroare la scrierea de la server\n");
                    return;
                }
                // printf("check 3\n");
                // fflush(stdout);

                if (ok)
                {
                    ok = false;
                    if (read(arg.client, pd, sizeof(pd)) < 0)
                    {
                        perror("Eroare la citirea de la server\n");
                        return;
                    }
                    // printf("check 4\n");
                    // fflush(stdout);

                    for (pugi::xml_node user = doc.child("Admins").child("user"); user; user = user.next_sibling("user"))
                    {
                        if (strcmp(pd, user.attribute("pswd").value()) == 0)
                            ok = true;
                    }
                    if (write(arg.client, &ok, sizeof(ok)) < 0)
                    {
                        perror("Eroare la scrierea de la server\n");
                        return;
                    }
                    // printf("check 5\n");
                    // fflush(stdout);
                }
            }
            else
            {
                printf("XML [ ] parsed with errors, attr value: %s\n", doc.child("node").attribute("attr").value());
                printf("Error description: %s", result.description());
            }
        }
        arg.admin = true;
    }
    else
        arg.admin = false;
}
static void *treat(void *arg)
{
    struct threadInfo thread;
    int nr;
    thread = *((struct threadInfo *)arg);
    connection(thread);
    struct info infos;
    char raspuns[10000];

    while (1)
    {
        bzero(&infos, sizeof(infos));
        bzero(&raspuns, sizeof(raspuns));
        // pthread_mutex_lock(&socket_mutex);
        if (read(thread.client, &nr, sizeof(int)) <= 0)
        {
            printf("[Thread %d]\n", thread.idThread);
            perror("Eroare la read() de la client.\n");
        }
        // printf("%d\n", nr);
        // fflush(stdout);
        // pthread_mutex_unlock(&socket_mutex);
        // usleep(10);
        // pthread_mutex_lock(&socket_mutex);
        if (read(thread.client, &infos, sizeof(infos)) <= 0)
        {
            printf("[Thread %d]\n", thread.idThread);
            perror("Eroare la read() de la client.\n");
        }
        // pthread_mutex_unlock(&socket_mutex);
        printf("%s\n", infos.statie);
        printf("thread citire\n");
        fflush(stdout);
        if (nr == 1)
        {
            StatusSosiri(infos, raspuns);
            // printf("%s\n", raspuns);
            // printf("aici\n");
            // fflush(stdout);
        }
        else if (nr == 2)
        {
            StatusPlecari(infos, raspuns);
        }
        else if (nr == 3)
        {
            ProgramTrenuri(raspuns);
        }
        else if (nr == 4)
        {
            for (int i = 0; i < 100; i++)
                if (clients[i].client == thread.client)
                {
                    clients[i].client = 0;
                }
            shutdown(thread.client, SHUT_RDWR);
            close((intptr_t)arg);
            return (NULL);
        }
        else if (nr == 5 && thread.admin == true)
        {
            // printf("comanda 5\n");
            // fflush(stdout);
            Intarziere(infos, raspuns);
        }

        printf("before write\n");
        fflush(stdout);
        // pthread_mutex_lock(&socket_mutex);
        if (write(thread.client, raspuns, sizeof(raspuns)) <= 0)
        {
            perror("eroare write raspuns catre client\n");
        }
        // pthread_mutex_lock(&socket_mutex);

        printf("final comanda\n");
        fflush(stdout);
    }
    return (NULL);
};

int main()
{
    int port;
    printf("\nIntroduceti portul dorit: ");
    scanf("%d", &port);
    struct sockaddr_in server;
    struct sockaddr_in from;
    int sd;
    pthread_t th[100];

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("initial.xml");
    if (result)
    {
        doc.save_file("program.xml");

        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("Eroare la socket()\n");
            exit(errno);
        }
        int on = 1;
        setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        bzero(&server, sizeof(server));
        bzero(&from, sizeof(from));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        server.sin_port = htons(port);
        if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
        {
            perror("Eroare la bind().\n");
            exit(errno);
        }
        if (listen(sd, 2) == -1)
        {
            perror("Eroare la listen().\n");
            exit(errno);
        }
        int i = 0;
        int k = 0;
        while (1)
        {
            int client;
            pid_t pid;
            threadInfo *thread;
            socklen_t length = sizeof(from);
            printf("port: %d\n", port);
            fflush(stdout);
            if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
            {
                perror("Eroare la accept().\n");
                continue;
            }

            if (read(client, &pid, sizeof(pid)) <= 0)
            {
                perror("eroare read pid client\n");
                exit(errno);
            }
            thread = (struct threadInfo *)malloc(sizeof(struct threadInfo));
            thread->client = client;
            thread->idThread = i++;
            clients[k].client = client;
            clients[k].pid = pid;
            printf("client acceptat\n");
            pthread_create(&th[i], NULL, &treat, thread);
            clients[k++].thread = th[i];
        }
    }
    else
    {
        printf("XML [ ] parsed with errors, attr value: %s\n", doc.child("node").attribute("attr").value());
        printf("Error description: %s", result.description());
    }
}
