//
// Created by Evyatar on 06/06/2022.
//
//    GENERAL STRUCTURE:
//    ./container <new_hostname> <new_filesystem_directory> <num_processes>
//    <path_to_program_to_run_within_container> <args_for_program>
#include <cstdio>
#include <sched.h>
#include <csignal>
#include <unistd.h>
#include <cstring>
//#include <cstdlib>
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
      check_return_value(ret_val, (char *)"failed creating new directory -- from make directory");
    }
}


void create_cgroup_directories_pids ()
{
  int ret_val;
  make_directory ((char *) "/sys");
  ret_val = chdir ("/sys");
  check_return_value(ret_val, (char *)"failed changing directory");
  make_directory ((char *) "fs");
  ret_val = chdir ("fs");
  check_return_value(ret_val, (char *)"failed changing directory");
  make_directory ((char *) "cgroup");
  ret_val = chdir ("cgroup");
  check_return_value(ret_val, (char *)"failed changing directory");
  make_directory ((char *) "pids");
  ret_val = chdir ("pids");
  check_return_value(ret_val, (char *)"failed changing directory");
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

  // change root folder
  char *path = *(temp + 2);
  std::cout << "got path: " << path << std::endl;

  // change hostname
  char *name = *(temp + 1);
  std::cout << "got hostname: " << name << std::endl;
  ret_val = sethostname (name, strlen (name));

  check_return_value(ret_val, (char *)"failed setting hostname");
  ret_val = chroot (path);
  check_return_value(ret_val, (char *)"failed changing root");
  ret_val = chdir (path);
  check_return_value(ret_val, (char *)"failed changing directory");

  // cgroups initialization of directories
  create_cgroup_directories_pids ();
  ret_val = chdir (path);
  check_return_value(ret_val, (char *)"failed changing directory");
  write_int_to_file ((char *) "/sys/fs/cgroup/pids/cgroup.procs",
                     getpid ());
  write_int_to_file ((char *) "/sys/fs/cgroup/pids/pids.max",
                     atoi(*(temp + 3)));

  // mount new procfs
  ret_val = chdir (path);
  check_return_value(ret_val, (char *)"failed changing directory");
//  std::cout << "changed dir to: " << path << std::endl;
  ret_val = mount ("proc", "/proc", "proc", 0, 0);
  check_return_value(ret_val, (char *)"failed mounting proc");

  // run terminal/new program
  char *_args[] = {*(temp + 4), *(temp + 5), (char *) 0};
  std::cout << "  --- args for execvp:  " << *(temp + 4) << "  -  " << *(temp + 5) << std::endl;
  int ret = execvp (*(temp + 4), _args);
  if (ret == -1){
      std::cerr
          << "system error: failed running command: " << errno  << std::endl;
      exit (1);
    }

  chdir (path);
  // notify on release:
  write_int_to_file ((char *) "/sys/fs/cgroup/pids/notify_on_release",
                     1);

  return 0;
}

void remove_directory (char path[])
{
  if (rmdir (path) != 0)
    {
      std::cerr << "system error: failed removing directory: " <<
          path << std::endl;
      exit (1);
    }
}

void signal_handler (char *path)
{
  // unmount
  umount (path);
  std::cout << "unmounted officially!" << std::endl;

  // remove files
//  chdir ((char *) "/sys/fs/cgroup/pids");
//  int ret = remove ((char *) "cgroup.procs");
//  std::cout << "file: " << "cgroup.procs" << " removed. ret val is: " << ret << std::endl;
//
//  ret = remove ((char *) "pids.max");
//  std::cout << "file: " << "pids.max" << " removed. ret val is: " << ret << std::endl;
//  ret = remove ((char *) "notify_on_release");
//  std::cout << "file: " << "notify_on_release" << " removed. ret val is: " << ret << std::endl;
////  chdir ("");
//  chdir ("/sys/fs/cgroup");
////  std::cout << "current diredctory is: " <<  << std::endl;
//
//  remove_directory ((char *) "pids");
//  chdir ("..");
//  remove_directory ((char *) "cgroup");
//  chdir ("..");
//  remove_directory ((char *) "fs");
//  chdir ("..");
//  remove_directory ((char *) "sys");

}

int main (int argc, char *argv[])
{
//  for (int  i = 0; i < argc; i++){
//      std::cout << "args[" << i << "] = " << argv[i] << std::endl;
//
//    }
//  signal(SIGCHLD, signal_handler);
  // create new process
//  chroot ((char *) argv + 2);
  void *stack = malloc (STACK);
  int child_pid = clone (child, stack + STACK,
                         CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD,
                         argv);
  std::cout << "cloned new child. child pid: " << child_pid << std::endl;

  wait (nullptr);
  signal_handler ((char *) argv + 2);
  return 0;
}

