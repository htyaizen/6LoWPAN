#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <err.h>
 

#include <errno.h>     
#include <pthread.h>   
#include <string.h>    

#include <sys/ioctl.h>
#include <net/if.h>


#define PORT 3000
#define MESSAGE "hello"



char response[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<doctype !html><html><head><title>Test Sensor</title>"
"<style>body { background-color:white }"
"h1 { font-size:2cm; text-align: center; color: black;"
" text-shadow: 0 0 2mm red}</style></head>"
"<body><h1>Sensors value: 00,00</h1></body></html>\r\n";

 
pthread_mutex_t lock;


void socket_management ( void *ptr );


int main()
{

	// thread socket
	pthread_t thread_socket;  
	pthread_create (&thread_socket, NULL, (void *) &socket_management, NULL);

  if (pthread_mutex_init(&lock, NULL) != 0)
  {
   	printf("\n mutex init failed\n");
    return 1;
 	}


	// web-server
  int one = 1, client_fd;
  struct sockaddr_in svr_addr, cli_addr;
  socklen_t sin_len = sizeof(cli_addr);
 
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    err(1, "can't open socket");
 
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
 
  int port = 8080;
  svr_addr.sin_family = AF_INET;
  svr_addr.sin_addr.s_addr = INADDR_ANY;
  svr_addr.sin_port = htons(port);
 
  if (bind(sock, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) == -1) {
    close(sock);
    err(1, "Can't bind");
  }
 
  listen(sock, 5);

  while (1) {

    client_fd = accept(sock, (struct sockaddr *) &cli_addr, &sin_len);

		pthread_mutex_lock(&lock);
    printf("got connection\n");
 
    if (client_fd == -1) {
      perror("Can't accept");
      continue;
    }
 
    write(client_fd, response, sizeof(response) - 1); /*-1:'\0'*/
    close(client_fd);
		pthread_mutex_unlock(&lock);

  }
}




void socket_management ( void *ptr ){

  int sock;
  socklen_t clilen;
  struct sockaddr_in6 server_addr, client_addr;
  char buffer[40];
  char addrbuf[INET6_ADDRSTRLEN];

  FILE* data_file = NULL;

  struct ifreq ifr;


  /* create a DGRAM (UDP) socket in the INET6 (IPv6) protocol */
  sock = socket(PF_INET6, SOCK_DGRAM, 0);

  if (sock < 0) {
    perror("creating socket");
    exit(1);
  }


  memset(&ifr, 0, sizeof(ifr));
  snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "tun0");
  if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
      perror("Set Interface failed");
      exit(4);  
      }


#ifdef V6ONLY
  // setting this means the socket only accepts connections from v6;
  // unset, it accepts v6 and v4 (mapped address) connections
  { int opt = 1;
    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) < 0) {
      perror("setting option IPV6_V6ONLY");
      exit(1);
    }
  }
#endif

  /* create server address: this will say where we will be willing to
     accept datagrams from */

  /* clear it out */
  memset(&server_addr, 0, sizeof(server_addr));

  /* it is an INET6 address */
  server_addr.sin6_family = AF_INET6;

  /* the client IP address, in network byte order */
  /* in this example we accept datagrams from ANYwhere */
  server_addr.sin6_addr = in6addr_any;

  /* the port we are going to listen on, in network byte order */
  server_addr.sin6_port = htons(PORT);

  /* associate the socket with the address and port */
  if (bind(sock, (struct sockaddr *)&server_addr,
	   sizeof(server_addr)) < 0) {
    perror("bind failed");
    exit(2);
  }

  while (1) {

    /* now wait until we get a datagram */
    //printf("\nwaiting for a datagram...\n");
    clilen = sizeof(client_addr);
    if (recvfrom(sock, buffer, 40, 0,
		 (struct sockaddr *)&client_addr,
		 &clilen) < 0) {
      perror("recvfrom failed");
      exit(4);
    }


    data_file = fopen("data.csv", "a");
    if (data_file != NULL)
    {
      fprintf(data_file, "%d.%d\n",(int) buffer[0],(int) buffer[1] );
      fclose(data_file);
    }


    /* now client_addr contains the address of the client */
    printf("Temperature %d.%d from %s\n",(int) buffer[0],(int) buffer[1],
	   inet_ntop(AF_INET6, &client_addr.sin6_addr, addrbuf,
		     INET6_ADDRSTRLEN));

	
		response[269] = (char)( (((int)buffer[0])/10) + 48);
		response[270] = (char)( (int)buffer[0] - ((((int)buffer[0])/10)*10) )+48;
		response[272] = (char)( (((int)buffer[1])/10) + 48);
		response[273] = (char)( (int)buffer[1] - ((((int)buffer[1])/10)*10) )+48;

    /* printf("sending message back\n"); */

    /* // if (sendto(sock, MESSAGE, sizeof(MESSAGE), 0, */
    /* if (sendto(sock, buffer, sizeof(buffer), 0, */
    /*            (struct sockaddr *)&client_addr, */
    /* 	       sizeof(client_addr)) < 0) { */
    /*   perror("sendto failed"); */
    /*   exit(5); */
    /* } */

  }


}



