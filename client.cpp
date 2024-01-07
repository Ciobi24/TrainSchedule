#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <csignal>
#include <pthread.h>

int main(int argc, char *argv[])
{
  int sock_descr;
  int port;
  struct sockaddr_in server;
  if (argc != 3)
  {
    printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }
  port = atoi(argv[2]);
  //////////////////////initializare socket
  if ((sock_descr = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la creare socket\n");
    exit(errno);
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(argv[1]);
  server.sin_port = htons(port);
  //////////////////////conectare
  if (connect(sock_descr, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("Eroare la conectarea la server\n");
    exit(errno);
  }
 bool admin;
  int ad;

  printf("\n\nLOGIN\n\nAlege cifra pentru tipul de utilizator:\n1 => Calator\n2 => Admin\nComanda: ");
  fflush(stdout);
  scanf("%d", &ad);
  if (ad == 2)
    admin = true;
  else
    admin = false;
  if (write(sock_descr, &admin, sizeof(admin)) <= 0)
  {
    perror("Eroare la scrierea catre server\n");
    exit(errno);
  }
  if (admin == 1)
  {
    char id[10], pd[10];
    bool ok = false;
    while (ok == false)
    {
      printf("\nintroduceti id: ");
      scanf("%s", id);
      id[strlen(id)] = '\0';
      //       for (size_t i = 0; i < strlen(id); i++)
      //     dprintf(STDOUT_FILENO, "%hu ", (unsigned short)id[i]);
      // dprintf(STDOUT_FILENO, "\n");
      if (write(sock_descr, id, sizeof(id)) <= 0)
      {
        perror("Eroare la scrierea catre server\n");
        exit(errno);
      }
      if (read(sock_descr, &ok, sizeof(ok)) < 0)
      {
        perror("Eroare la citirea de la server\n");
        exit(errno);
      }
      if (ok)
      {
        printf("\nintroduceti parola: ");
        scanf("%s", pd);
        pd[strlen(pd)] = '\0';
        // for (size_t i = 0; i < strlen(pd); i++)
        //     dprintf(STDOUT_FILENO, "%hu ", (unsigned short)pd[i]);
        // dprintf(STDOUT_FILENO, "\n");
        if (write(sock_descr, pd, sizeof(pd)) <= 0)
        {
          perror("Eroare la scrierea catre server\n");
          exit(errno);
        }
        if (read(sock_descr, &ok, sizeof(ok)) < 0)
        {
          perror("Eroare la citirea de la server\n");
          exit(errno);
        }
        if (ok)
        {
          printf("Conectat cu succes\n");
          ok = true;
        }
        else
        {
          ok = false;
          printf("Parola gresita\n");
        }
      }
      else
        printf("Id inexistent\n");
    }
  }  

  int pid = fork();
  if (pid == -1)
  {
    perror("Error in fork.\n");
    return errno;
  }

  if (pid)
  {
    printf("\n\n///////////////////////////////////////////////////////\n\n");
    printf("              MERSUL TRENURILOR\n\n");
    printf("\n\n///////////////////////////////////////////////////////\n\n");
    fflush(stdout);
    char command[50];
    bool run = true,first=true;
    while (run == true)
    {
      if(first==true){
              fgets(command, sizeof(command), stdin);
              first=false;
            continue;
      }
      if(admin==1)
        printf("\n1.Status Sosiri: sosiri <statie>\n2.Status Plecari: plecari <statie>\n3.Programul de azi: program\n4.Intarziere: intarziere <tren> <statie> <min> (utilizati notatia negativa daca trenul ajunge mai devreme ex: -5)\n5.Quit: quit\n\nComanda dorita: ");
        else
                printf("\n1.Status Sosiri: sosiri <statie>\n2.Status Plecari: plecari <statie>\n3.Programul de azi: program\n4.Quit: quit\n\nComanda dorita: ");

        fflush(stdout);


      fgets(command, sizeof(command), stdin);
      command[strlen(command) - 1] = '\0';
      // for (size_t i = 0; i < strlen(command); i++)
      // dprintf(STDOUT_FILENO, "%hu ", (unsigned short)command[i]);
      // dprintf(STDOUT_FILENO, "\n");
      // printf("%s\n", command);
      // fflush(stdout);
      if (write(sock_descr, command, sizeof(command)) <= 0)
      {
        perror("Eroare la scrierea catre server\n");
        exit(errno);
      }
      sleep(1);

      if (strstr(command, "quit"))
      {
        run = false;
        kill(pid, 9);
      }
    }

    close(sock_descr);
    exit(0);
  }
  else
  {
    char raspuns[10000];
    while (1)
    {
      bzero(raspuns, sizeof(raspuns));

      if (read(sock_descr, raspuns, sizeof(raspuns)) < 0)

      {
        perror("eroare la read() program\n");
        exit(errno);
      }

      printf("%s", raspuns);
      fflush(stdout);
    }
  }
}
