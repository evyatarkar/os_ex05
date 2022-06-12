//
// Created by Evyatar on 12/06/2022.
//

//    GENERAL STRUCTURE:
//    ./sockets client <port> <terminal_command_to_run>
//    ./sockets server <port>

void create_client (int port, char *terminal_command_to_run)
{

}

void create_server (int port)
{

}

int main (int argc, char *argv[])
{
  char *type = argv + 1;
  int port = atoi (*(argv + 2));
  char *terminal_command_to_run;
  if (strcmp (type, "client") == 0)
    {
      terminal_command_to_run = *(argv + 3);
      create_client (port, terminal_command_to_run);
    }
  else
    {
    create_server(port);
    }

  return 0;
}