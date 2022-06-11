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

#define STACK 8192
#define MKDIR_MODE 0755


void make_directory (char *name)
{
  if (access (name, MKDIR_MODE) == -1)  // TODO check mode is relevant here
    {
      if (mkdir (name, MKDIR_MODE))
        {
          std::cerr << "system error: " << "failed creating new directory: "
                    << name << std::endl;
          exit (1);
        }
    }
}

void create_cgroup_directories_pids ()
{
  make_directory ((char *) "sys");
  chdir ("/sys");
  make_directory ((char *) "fs");
  chdir ("/sys/fs");
  make_directory ((char *) "cgroup");
  chdir ("/sys/fs/cgroup");
  make_directory ((char *) "pids");
  chdir ("/sys/fs/cgroup/pids");
}

void write_string_to_file(char *path, char * filename, char * content){
  chdir(path);
  if (access (filename, MKDIR_MODE) == 0)
    {
      std::cerr
          << "system error: failed finding file: " <<
          path << "/" << filename
          << std::endl;
      exit (1);
    }
  std::ofstream File(filename);
  File << content << std::endl;
  File.close();
}

void write_int_to_file(char *path, char * filename, int content){
  chdir(path);
  if (access (filename, MKDIR_MODE) == 0)
    {
      std::cerr
          << "system error: failed finding file: " <<
          path << "/" << filename
          << std::endl;
      exit (1);
    }
  std::ofstream File(filename);
  File << content << std::endl;
  File.close();
}

int child (void *args)
{
  std::cout << "running child function" << std::endl;

  // change root folder
  char *path = (char *) args + 1;
  chroot (path);
  chdir (path);

  // cgroups initialization of directories
  create_cgroup_directories_pids ();

  // change hostname
  char *name = (char *) args;
  std::cout << "got hostname: " << name << std::endl;
  sethostname (name, strlen (name));

  write_int_to_file ((char *) "/sys/fs/cgroup/pids",
                        (char *) "cgroup.procs",
                        getpid ());

  write_int_to_file ((char *) "/sys/fs/cgroup/pids",
                        (char *) "pids.max",
                     *((int*) args+2));

  // mount new procfs
  chdir(path);
  mount("proc", "/proc", "proc", 0, 0);


  // run terminal/new program

  //  char* _args[] ={"/bin/echo", (char*)argv + 1, (char *)0};
  //  int ret = execvp("/bin/echo", _args);


  // notify on release:
  write_int_to_file ((char *) "/sys/fs/cgroup/pids",
                        (char *) "notify_on_release",
                        1);

  return 0;
}

int main (int argc, char *argv[])
{
  // create new process
  void *stack = malloc (STACK);
  int child_pid = clone (child, stack + STACK,
                         CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD,
                         argv + 1);
  std::cout << "cloned new child. child pid: " << child_pid << std::endl;


  wait (nullptr);
  // 6
  // TODO unmount
  // TODO delete cgroups
//  if(rmdir () == 0){
//
//  }

  return 0;
}

