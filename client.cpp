#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// extern int errno;

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
    perror("[client]Eroare la creare socket\n");
    return errno;
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(argv[1]);
  server.sin_port = htons(port);
  //////////////////////conectare
  if (connect(sock_descr, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la conectarea la server\n");
    return errno;
  }
  bool admin;
  int ad;
  printf("Alege tipul de utilizator:\n1.Calator\n2.Admin\nComanda: ");
  fflush(stdout);
  scanf("%d", &ad);
  if (ad == 2)
    admin = true;
  else
    admin = false;
  if (write(sock_descr, &admin, sizeof(admin)) <= 0)
  {
    perror("[client]Eroare la scrierea catre server\n");
    return errno;
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
        perror("[client]Eroare la scrierea catre server\n");
        return errno;
      }
      if (read(sock_descr, &ok, sizeof(ok)) < 0)
      {
        perror("[client]Eroare la citirea de la server\n");
        return errno;
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
          perror("[client]Eroare la scrierea catre server\n");
          return errno;
        }
        if (read(sock_descr, &ok, sizeof(ok)) < 0)
        {
          perror("[client]Eroare la citirea de la server\n");
          return errno;
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

  bool run = true;
  /////////////////bucla pentru introducere comenzi
  while (run)
  {
    // meniul
    printf("1 => Status Sosiri\n2 => Status Plecari\n3 => Program\n4 => Quit\n");
    if (admin)
    {
      printf("5 => Intarziere\n");
    }
    printf("\nComanda dorita: ");
    fflush(stdout);
    scanf("%d", &command);
    //////////////////trimitere comanda la server
    if (write(sock_descr, &command, sizeof(int)) <= 0)
    {
      perror("[client]Eroare la scrierea catre server\n");
      return errno;
    }
    if (command == 4)
    {
      run = false;
    }
    else if (command == 1)
    {
      // sosiri
      printf("introduceti statia: ");
      fflush(stdout);
      char statie[30];
      bzero(&statie, sizeof(statie));
      scanf("%s", statie);
      statie[strlen(statie)] = '\0';
      //////////////////trimitere comanda la server
      if (write(sock_descr, &statie, sizeof(statie)) <= 0)
      {
        perror("[client]Eroare la scrierea catre server\n");
        return errno;
      }
      char raspuns[10000];
      bzero(raspuns, sizeof(raspuns));
      ///////////////primire raspuns
      if (read(sock_descr, raspuns, 10000) < 0)
      {
        perror("[client]Eroare la citirea de la server\n");
        return errno;
      }
      printf("\n%s\n", raspuns);
    }
    else if (command == 2)
    {
      // plecari
      printf("introduceti statia: ");
      fflush(stdout);
      char statie[30];
      bzero(&statie, sizeof(statie));
      scanf("%s", statie);
      statie[strlen(statie)] = '\0';
      //////////////////trimitere comanda la server
      if (write(sock_descr, &statie, sizeof(statie)) <= 0)
      {
        perror("[client]Eroare la scrierea catre server\n");
        return errno;
      }
      char raspuns[10000];
      bzero(raspuns, sizeof(raspuns));
      ///////////////primire raspuns
      if (read(sock_descr, raspuns, 10000) < 0)
      {
        perror("[client]Eroare la citirea de la server\n");
        return errno;
      }
      printf("\n%s\n", raspuns);
    }
    else if (command == 3)
    {
      char raspuns[10000];
      bzero(raspuns, sizeof(raspuns));
      ///////////////primire raspuns
      if (read(sock_descr, raspuns, 10000) < 0)
      {
        perror("[client]Eroare la citirea de la server\n");
        return errno;
      }
      printf("\n%s\n", raspuns);
    }
    else if (command == 5 && admin==true)
    {
      char n[5];
      printf("Numar tren: ");
      fflush(stdout);
      scanf("%s", n);
      n[strlen(n)] = '\0';
      if (write(sock_descr, n, sizeof(n)) <= 0)
      {
        perror("[client]Eroare la scrierea catre server\n");
        return errno;
      }
      printf("Sosire cu intarziere/mai devreme in statia: ");
      fflush(stdout);
      char statie[30];
      scanf("%s", statie);
      statie[strlen(statie)] = '\0';
      if (write(sock_descr, statie, sizeof(statie)) <= 0)
      {
        perror("[client]Eroare la scrierea catre server\n");
        return errno;
      }
      printf("Intarziere/sosire mai devreme (in minute): ");
      fflush(stdout);
      char est[10];
      scanf("%s", est);
      est[strlen(est)] = '\0';
      if (write(sock_descr, est, sizeof(est)) <= 0)
      {
        perror("[client]Eroare la scrierea catre server\n");
        return errno;
      }
    }
    else
    {
      printf("\n! Comanda gresita !\n");
    }
  }
  ////////comanda quit a fost apelata, inchidem conexiunea
  close(sock_descr);
  exit(0);
}

// #include <netdb.h>
// #include <sys/types.h>