#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
// #include <SDL2/SDL.h>
// #include <GL/gl.h>
#include <stdbool.h>
int port;
int main(int argc, char *argv[])
{
  int sd;
  struct sockaddr_in server;

  int comanda1 = 0;
 // char buf[10];
  bool isRunning = true;

  if (argc != 3)
  {
    printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  port = atoi(argv[2]);

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
    return errno;
  }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
      perror("[client]Eroare la connect().\n");
      return errno;
    }
    while (isRunning)
    {
      // MENIU
      printf("\n1.Status Sosiri");
      printf("\n2.Status Plecari");
      printf("\n3.Program");
      printf("\n4.Quit\n");
      printf("Introduceti o comanda: ");
      fflush(stdout);

      // Citim ce doreste utilizatorul si trimitem catre server
      scanf("%d", &comanda1);
      if (write(sd, &comanda1, sizeof(int)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
        return errno;
      }
      if (comanda1 == 4)
      {
        isRunning = false;
      }
      else if (comanda1 == 1 || comanda1 == 2 || comanda1==3)
      {
        char primit[300];
        bzero(primit, sizeof(primit));
        if (read(sd, primit, 300) < 0)
        {
          perror("[client]Eroare la read() de la server.\n");
          return errno;
        }
        printf("%s", primit);
      }
      else
      {
        printf("Comanda gresita");
      }
    
    }
    close(sd);
    exit(0);
  
}