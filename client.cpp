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

bool run=true;
void * for_reading(void* arg)
  {
    int sock_descr=*(int*)arg;
    char raspuns[10000];
    while (run)
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
    pthread_exit(NULL);
  }

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
  int * sock=&sock_descr;
  pthread_t p;
  pthread_create(&p,NULL,&for_reading,sock);

    printf("\n\n///////////////////////////////////////////////////////\n\n");
    printf("              MERSUL TRENURILOR\n\n");
    printf("\n\n///////////////////////////////////////////////////////\n\n");
    fflush(stdout);
    char command[50];
    
    while (run == true)
    {

        printf("\n1.Status Sosiri: sosiri <statie>\n2.Status Plecari: plecari <statie>\n3.Programul de azi: program\n4.Intarziere: intarziere <tren> <statie> <min> (utilizati notatia negativa daca trenul ajunge mai devreme ex: -5)\n5.Quit: quit\n\nComanda dorita: ");
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
      sleep(2);

      if (strcmp(command, "quit")==0)
      {
           run = false;
      }
    }

    close(sock_descr);
        pthread_join(p, NULL); // Wait for the reading thread to exit properly
    exit(0);
  }






      