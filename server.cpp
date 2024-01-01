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
    bool admin;
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
                strcat(msg, statie.attribute("ora_sosire").value());
                strcat(msg, ", Plecare: ");
                strcat(msg, statie.attribute("ora_plecare").value());
                strcat(msg, ", Intarziere: ");
                strcat(msg, statie.attribute("intarziere").value());
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
        printf("XML [ ] parsed with errors, attr value: %s\n", doc.child("node").attribute("attr").value());
        printf("Error description: %s", result.description());
    }
}
void StatusSosiri(void *arg)
{
    printf("status sosiri\n");
    struct threadInfo thread;
    thread = *((struct threadInfo *)arg);
    char statie[100];
    if (read(thread.client, statie, sizeof(statie)) < 0)
    {
        printf("[thread %d]", thread.idThread);
        perror("eroare read() de la client");
    }
    char msg[10000];
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

        strcpy(msg, "Sosiri in statia: ");
        strcat(msg, statie);
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
                if (ora2 == 0 && ora1 == 23)
                    ora2 = 24;
                if (ora1 == ora2 - 1 && (60 - min1 + min2) <= 60)
                    ok = true;
                else if (ora1 == ora2 && (min2 - min1) >= 0 && (min2 - min1) <= 60)
                    ok = true;
                else
                    ok = false;
                // printf("\n ora min program %d %d  ora min fisier %d %d\n", ora1, min1, ora2, min2);
                // fflush(stdout);
                if (strcmp(station.attribute("name").value(), statie) == 0)
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
    strcat(msg, "\0");
    if (write(thread.client, msg, sizeof(msg)) <= 0)
    {
        printf("[Thread %d] ", thread.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
}
void StatusPlecari(void *arg)
{
    printf("status plecari\n");
    struct threadInfo thread;
    thread = *((struct threadInfo *)arg);
    char statie[100];
    if (read(thread.client, statie, sizeof(statie)) < 0)
    {
        printf("[thread %d]", thread.idThread);
        perror("eroare read() de la client");
    }
    char msg[10000];
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

        strcpy(msg, "Plecari din statia: ");
        strcat(msg, statie);
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
                if (ora2 == 0 && ora1 == 23)
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
                if (strcmp(station.attribute("name").value(), statie) == 0)
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
    strcat(msg, "\0");
    if (write(thread.client, msg, sizeof(msg)) <= 0)
    {
        printf("[Thread %d] ", thread.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
}
void Intarziere(void *arg)
{
    struct threadInfo thread;
    thread = *((struct threadInfo *)arg);
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("program.xml");
    if (result)
    {
        char n[5];
        if (read(thread.client, n, sizeof(n)) < 0)
        {
            perror("[server]Eroare la citirea de la client\n");
            return;
        }
        // printf("%s\n",n);
        // fflush(stdout);
        char statie[30];
        if (read(thread.client, statie, sizeof(statie)) < 0)
        {
            perror("[server]Eroare la citirea de la client\n");
            return;
        }
        // printf("%s\n",statie);
        //         fflush(stdout);

        char est[10];
        if (read(thread.client, est, sizeof(est)) < 0)
        {
            perror("[server]Eroare la citirea de la client\n");
            return;
        }
        // printf("%s\n",est);
        //         fflush(stdout);

        for (pugi::xml_node tren = doc.child("Program").child("Tren"); tren; tren = tren.next_sibling("Tren"))
        {
            if (strcmp(n, tren.attribute("name").value()) == 0)
            {
                for (pugi::xml_node station = tren.child("Statii").child("Statie"); station; station = station.next_sibling("Statie"))
                {
                    if (strcmp(statie, station.attribute("name").value()) == 0)
                    {
                        for (pugi::xml_node nextst = station; nextst; nextst = nextst.next_sibling("Statie"))
                        {
                            pugi::xml_attribute attr = nextst.attribute("intarziere");
                            attr.set_value(est);
                        }
                        doc.save_file("program.xml");
                        break;
                    }
                }
                break;
            }
        }
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
        perror("[server]Eroare la citirea de la client\n");
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
                perror("[client]Eroare la citirea de la server\n");
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
                    perror("[client]Eroare la scrierea de la server\n");
                    return;
                }
                // printf("check 3\n");
                // fflush(stdout);

                if (ok)
                {
                    ok = false;
                    if (read(arg.client, pd, sizeof(pd)) < 0)
                    {
                        perror("[client]Eroare la citirea de la server\n");
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
                        perror("[client]Eroare la scrierea de la server\n");
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
    printf("%d\n", (int)thread.admin);
    fflush(stdout);
    while (1)
    {
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
        else if (nr == 5 && thread.admin == true)
        {
            // printf("comanda 5\n");
            // fflush(stdout);
            Intarziere((struct threadInfo *)arg);
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

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("initial.xml");
    if (result)
    {
        doc.save_file("program.xml");

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
    else
    {
        printf("XML [ ] parsed with errors, attr value: %s\n", doc.child("node").attribute("attr").value());
        printf("Error description: %s", result.description());
    }
}
