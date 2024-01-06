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

pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;

int signal_received = 0;
void signal_handler(int sig)
{
  if (sig == SIGUSR1)
  {
    printf("Received signal SIGUSR1.\n");
    signal_received = +1;
  }
}
int main(int argc, char *argv[])
{
  int sock_descr;
  int port;
  struct sockaddr_in server;
  int command = 0;
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
  pid_t pidt = getpid();
  if (write(sock_descr, &pidt, sizeof(pidt)) <= 0)
  {
    perror("eroare write pid catre server\n");
    exit(errno);
  }
  bool admin;
  int ad;

  printf("\n\n///////////////////////////////////////////////////////\n\n");
  printf("              MERSUL TRENURILOR\n\n");
  printf("\n\n///////////////////////////////////////////////////////\n\n");

  printf("Alege tipul de utilizator:\n1.Calator\n2.Admin\nComanda: ");
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
  char raspuns[10000];
  struct
  {
    char statie[30];
    char tren[5];
    char intarziere[5];
  } info;
  bool run = true, to_do;

  /////////////////bucla pentru introducere comenzi
  while (run)
  {
    // meniul
    to_do = false;
    if (signal(SIGUSR1, signal_handler) == SIG_ERR)
    {
      perror("Signal handling registration failed");
      exit(errno);
    }
    usleep(10);
    if (signal_received)
    {
      signal_received--;
      bzero(raspuns, sizeof(raspuns));

      if (read(sock_descr, raspuns, sizeof(raspuns)) <= 0)

      {
        perror("eroare la read() program\n");
        exit(errno);
      }

      printf("%s", raspuns);
      fflush(stdout);
    }
    sleep(1);
    printf("\n1 => Status Sosiri\n2 => Status Plecari\n3 => Program\n4 => Quit\n");
    if (admin)
    {
      printf("5 => Intarziere\n");
    }
    printf("\nComanda dorita: ");
    fflush(stdout);
    scanf("%d", &command);

    //  printf("\n%d\n",signal_received);
    if (signal_received)
    {
      signal_received--;
      bzero(raspuns, sizeof(raspuns));

      if (read(sock_descr, raspuns, sizeof(raspuns)) <= 0)

      {
        perror("eroare la read() program\n");
        exit(errno);
      }

      printf("%s", raspuns);
      fflush(stdout);
    }
    //////////////////trimitere comanda la server

    if (write(sock_descr, &command, sizeof(int)) <= 0)
    {
      perror("Eroare la scrierea catre server\n");
      exit(errno);
    }

    if (command == 4)
    {
      run = false;
    }
    else if (command == 1 || command == 2 || command == 3 || command == 5)
    {

      bzero(&info, sizeof(info));
      if (command == 1 || command == 2)
      {
        // sosiri sau plecari
        printf("introduceti statia: ");
        fflush(stdout);
        char statie[30];

        scanf("%s", statie);
        statie[strlen(statie)] = '\0';
        strcpy(info.statie, statie);

        if (signal_received)
        {
          signal_received--;
          bzero(raspuns, sizeof(raspuns));

          if (read(sock_descr, raspuns, sizeof(raspuns)) <= 0)

          {
            perror("eroare la read() program\n");
            exit(errno);
          }

          printf("%s", raspuns);
          fflush(stdout);
        }
      }

      else if (command == 5 && admin == true)
      {
        printf("Numar tren: ");
        fflush(stdout);
        char i[5];

        scanf("%s", i);
        i[strlen(i)] = '\0';
        strcpy(info.tren, i);
        if (signal_received)
        {
          signal_received--;
          bzero(raspuns, sizeof(raspuns));

          if (read(sock_descr, raspuns, sizeof(raspuns)) <= 0)

          {
            perror("eroare la read() program\n");
            exit(errno);
          }

          printf("%s", raspuns);
          fflush(stdout);
        }

        printf("Sosire cu intarziere/mai devreme in statia: ");
        fflush(stdout);
        char statie[30];
        scanf("%s", statie);
        statie[strlen(statie)] = '\0';
        strcpy(info.statie, statie);

        if (signal_received)
        {
          signal_received--;
          bzero(raspuns, sizeof(raspuns));

          if (read(sock_descr, raspuns, sizeof(raspuns)) <= 0)

          {
            perror("eroare la read() program\n");
            exit(errno);
          }

          printf("%s", raspuns);
          fflush(stdout);
        }

        printf("Intarziere/sosire mai devreme ( in minute, mai devreme ex: -5 ): ");
        fflush(stdout);
        char est[10];
        scanf("%s", est);
        est[strlen(est)] = '\0';
        strcpy(info.intarziere, est);
        if (signal_received)
        {
          signal_received--;
          bzero(raspuns, sizeof(raspuns));

          if (read(sock_descr, raspuns, sizeof(raspuns)) <= 0)

          {
            perror("eroare la read() program\n");
            exit(errno);
          }

          printf("%s", raspuns);
          fflush(stdout);
        }
      }

      if (write(sock_descr, &info, sizeof(info)) < 0)
      {
        perror("Eroare la scrierea catre server\n");
        exit(errno);
      }

      bzero(raspuns, sizeof(raspuns));

      if (read(sock_descr, raspuns, sizeof(raspuns)) <= 0)

      {
        perror("eroare la read() program\n");
        exit(errno);
      }

      printf("%s", raspuns);
      fflush(stdout);
    }
    else
    {
      printf("\n! Comanda gresita !\n");
    }
  }

  close(sock_descr);
  exit(0);
}
