//
// Created by Evyatar on 12/06/2022.
//

//    GENERAL STRUCTURE:
//    ./sockets client <port> <terminal_command_to_run>
//    ./sockets server <port>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#define MAXHOSTNAME 100
#define MAX_CLIENTS 5
#define BUF_SIZE 256

int call_socket (char *hostname, unsigned short portnum)
{

  struct sockaddr_in sa;
  struct hostent *hp;
  int s;

  if ((hp = gethostbyname (hostname)) == NULL)
    {
      return (-1);
    }

  memset (&sa, 0, sizeof (sa));
  memcpy ((char *) &sa.sin_addr, hp->h_addr, hp->h_length);
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons ((u_short) portnum);
  if ((s = socket (hp->h_addrtype, SOCK_STREAM, 0)) < 0)
    {
      return (-1);
    }

  if (connect (s, (struct sockaddr *) &sa, sizeof (sa)) < 0)
    {
      close (s);
      return (-1);
    }

  return (s);
}

int read_data (int s, char *buf, int n)
{
  int bcount;       /* counts bytes read */
  int br;               /* bytes read this pass */
  bcount = 0;
  br = 0;

  while (bcount < n)
    { /* loop until full buffer */
      br = read (s, buf, n - bcount);
      if (br > 0)
        {
          bcount += br;
          buf += br;
        }
      if (br < 1)
        {
          return (-1);
        }
    }
  return (bcount);
}

int create_client (int port, char *terminal_command_to_run)
{
  std::cout << "creating client socket" << std::endl;
  char * name = (char *)"localhost";
  int server_socket_id = call_socket (name, port);
  if (server_socket_id == -1){
      std::cerr << "system error: "
                << "failed creating connection to socket: from client"
                << std::endl;
      exit(-1);
  }
  char *buffer = (char *)(malloc (BUF_SIZE));
//  int size_read = read_data (server_socket_id, buffer,);

}

int open_server_socket (int port)
{
//  char myname[MAXHOSTNAME + 1];
  const char *myname = "localhost";

  int s;
  struct sockaddr_in sa;
  struct hostent *hp;
  //hostnet initialization
//  gethostname (myname, MAXHOSTNAME);
  sethostname(myname, strlen(myname));
  hp = gethostbyname (myname);
  if (hp == NULL)
    {
      return (-1);
    }
  //sockaddrr_in initlization
  memset (&sa, 0, sizeof (struct sockaddr_in));
  sa.sin_family = hp->h_addrtype;
  /* this is our host address */
  memcpy (&sa.sin_addr, hp->h_addr, hp->h_length);
  /* this is our port number */
  sa.sin_port = htons (port);

  /* create socket */
  if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
      return (-1);
    }

  if (bind (s, (struct sockaddr *) &sa, sizeof (struct sockaddr_in)) < 0)
    {
      close (s);
      return (-1);
    }

  listen (s, MAX_CLIENTS); /* max # of queued connects */
  return (s);
}

int get_connection (int s)
{
  int t; /* socket of connection */
  std::cout << "getting connection start" << std::endl;
  if ((t = accept (s, nullptr, nullptr)) < 0)
    {
      std::cout << "getting connection failure" << std::endl;
      return -1;
    }
  std::cout << "getting connection success" << std::endl;
  return t;
}

int create_server (int port)
{
  std::cout << "creating server socket" << std::endl;
  int server_socket_id = open_server_socket (port);

  while(true)  // TODO O
    {
      int new_connection_socket = get_connection (server_socket_id);
      if (new_connection_socket == -1)  // TODO check this if needs to crash or live
        {
          std::cerr << "system error: "
                    << "failed creating connection to socket:  from server"
                    << std::endl;
//          exit (1);
        }
        // allocate fd for new socket
    }
//    return 0;
//    int connected = call_socket(myname, port);

}

int main (int argc, char *argv[])
{
  char *type = argv[1];
  int port = atoi (argv[2]);
  char *terminal_command_to_run;
  if (strcmp (type, "client") == 0)
    {
      terminal_command_to_run = *(argv + 3);
      create_client (port, terminal_command_to_run);
    }
  else
    {
      create_server (port);
    }

  return 0;
}