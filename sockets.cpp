//
// Created by Evyatar on 12/06/2022.
//

//    GENERAL STRUCTURE:
//    ./sockets client <port> <terminal_command_to_run>
//    ./sockets server <port>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
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
  char *name = (char *) "localhost";
  int server_socket_id = call_socket (name, port);
  if (server_socket_id == -1)
    {
      std::cerr << "system error: "
                << "failed creating connection to socket: from client"
                << std::endl;
      exit (-1);
    }
  char *buffer = (char *) (malloc (BUF_SIZE));
  strcpy (buffer, terminal_command_to_run);

  // write command to server
  write (server_socket_id, buffer, strlen (buffer));
  return 0;
}

int open_server_socket (int port)
{
  const char *myname = "localhost";
  int s;
  struct sockaddr_in sa;
  struct hostent *hp;
  //hostnet initialization
  sethostname (myname, strlen (myname));
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
  if ((t = accept (s, nullptr, nullptr)) < 0)
    {
      return -1;
    }
  return t;
}

int create_server (int port)
{
  int server_socket_id = open_server_socket (port);

  while (true)
    {
      int new_connection_socket = get_connection (server_socket_id);
      if (new_connection_socket == -1)
        {
          std::cerr << "system error: "
                    << "failed creating connection to socket:  from server"
                    << std::endl;
        }
      char *buffer = (char *) (malloc (BUF_SIZE));
      read_data (new_connection_socket, buffer, BUF_SIZE);
      int ret_val = system (buffer);
      if (ret_val != 0)
        {
          std::cerr << "system error: "
                    << "failed running command on server."
                    << std::endl;
          exit (1);
        }
    }
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