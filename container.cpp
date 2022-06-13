//    GENERAL STRUCTURE:
//    ./container <new_hostname> <new_filesystem_directory> <num_processes>
//    <path_to_program_to_run_within_container> <args_for_program>

#include <cstdio>
#include <sched.h>
#include <csignal>
#include <unistd.h>
#include <cstring>
#include <bitset>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <csignal>
#include <cstdlib>

#define STACK 8192
#define MKDIR_MODE 0755
#define EXTRA_SIZE 100


void check_return_value(int val, char *msg){
  if (val == -1)
    {
      std::cerr << "system error: " << msg << std::endl;
      exit (1);
    }
}

void make_directory (char *name)
{
  if (access (name, F_OK) == -1)
    {
      int ret_val = mkdir (name, MKDIR_MODE);
      check_return_value(ret_val, (char *)"failed creating new directory");
    }
}


void create_cgroup_directories_pids ()
{
  make_directory ((char *) "/sys");
  make_directory ((char *) "/sys/fs");
  make_directory ((char *) "/sys/fs/cgroup");
  make_directory ((char *) "/sys/fs/cgroup/pids");
}

void write_int_to_file (char *path, int content)
{
  std::ofstream file_to_write_to(path);
  std::cout << "file_to_write_to: " << path << " created." << std::endl;
  file_to_write_to << content;
  std::cout << "-- writing in file_to_write_to: " << content << std::endl;
  file_to_write_to.close ();
  std::cout << "-- done writing" << std::endl;
}

int child (void *args)
{
  int ret_val;
  char **temp = (char**) args;
  std::cout << "running child function" << std::endl;
  char *path = *(temp + 2);
  std::cout << "got path: " << path << std::endl;

  // change hostname
  char *name = *(temp + 1);
  std::cout << "got hostname: " << name << std::endl;
  ret_val = sethostname (name, strlen (name));
  check_return_value(ret_val, (char *)"failed setting hostname");

  // change root folder
  ret_val = chroot (path);
  check_return_value(ret_val, (char *)"failed changing root");

  // cgroups initialization of directories
  create_cgroup_directories_pids ();

  std::cout << " ++  path is: " << path << std::endl;
  ret_val = chdir ("/");
  check_return_value(ret_val, (char *)"failed changing directory from after root");

  write_int_to_file ((char *) "/sys/fs/cgroup/pids/cgroup.procs", getpid ());
  write_int_to_file ((char *) "/sys/fs/cgroup/pids/pids.max",atoi(*(temp + 3)));

  // mount new procfs
  make_directory((char *)"/proc");
  ret_val = mount("proc", "/proc", "proc", 0, 0);
  check_return_value(ret_val, (char *)"failed mounting proc");

  // run terminal/new program
  char *_args[] = {*(temp + 4), *(temp + 5), (char *) 0};
  const char *file_to_run = *(temp + 4);

  ret_val = execvp (file_to_run, _args);
  check_return_value(ret_val, (char *)"failed running command.");

  return 0;
}

void handle_dead_child (char *path_to_unmount, char *path_of_director_to_remove, void *stack_ptr)
{
  // unmount
  int ret_val;
  ret_val = umount (strcat(path_to_unmount, "/proc"));
  check_return_value(ret_val, (char *)"failed unmounting.");
  std::cout << "unmounted officially!" << std::endl;

  // delete the child's stack
  free(stack_ptr);

  // remove files
  void *cmd = malloc(strlen(path_of_director_to_remove) + EXTRA_SIZE);
  strcpy((char *)cmd, "rm -r ");   //{'r', 'm', ' ', '-', 'r', ' '};
  strcat((char *)cmd, path_of_director_to_remove);
  std::cout << "THIS IS CMD NOW: " << (char *)cmd <<std::endl;
  ret_val = system((char *)cmd);
  check_return_value(ret_val, (char *)"failed removing files");
  free(cmd);
}

int main (int argc, char *argv[])
{
  void *stack = malloc (STACK);
  int child_pid = clone (child, stack + STACK,
                         CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD,
                         argv);
  std::cout << "cloned new child. child pid: " << child_pid << std::endl;

  wait (nullptr);

  void *temp_path1 = malloc (strlen(argv[2]) + EXTRA_SIZE);
  strcpy((char *)temp_path1, argv[2]);

  void *temp_path2 = malloc (strlen(argv[2]) + EXTRA_SIZE);
  strcpy((char *)temp_path2, argv[2]);

  void *temp_path3 = malloc (strlen(argv[2]) + EXTRA_SIZE);
  strcpy((char *)temp_path3, argv[2]);

  write_int_to_file (strcat((char *)temp_path1, "/sys/fs/cgroup/pids/notify_on_release"),1);
  handle_dead_child ((char *)temp_path2, strcat((char *)temp_path3, "/sys/fs"), stack);
  free(temp_path1);
  free(temp_path2);
  free(temp_path3);
  return 0;
}

