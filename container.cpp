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

#define STACK 8192
#define MKDIR_MODE 0755
#define MAX_PROC_NUM 256

int child (void *args)
{
  std::cout << "running child function" << std::endl;
  // 3
  // change hostname and root folder
  char *name = (char *) args;
  std::cout << "got hostname: " << name << std::endl;
  sethostname (name, strlen (name));

  char * path = (char *)args + 1;
  std::cout << "got path: " << path << std::endl;
  chroot(path);
  chdir(path);


  // 4
  // mount new procfs

  // TODO mount procfs
//  mount("proc", "/proc", "proc", 0, 0);


  // 5
  // run terminal/new program

  //  char* _args[] ={"/bin/echo", (char*)argv + 1, (char *)0};
  //  int ret = execvp("/bin/echo", _args);

  return 0;
}


void make_directory(char * name){
  if (access(name, MKDIR_MODE) == -1)
    {
      if (mkdir (name, MKDIR_MODE))
        {
          std::cerr << "system error: " << "failed creating new directory: "
                    << name << std::endl;
          exit(1);
        }
    }
}

void create_pid_directory(){
  make_directory((char *)"sys");
  chdir("/sys");
  make_directory((char *)"fs");
  chdir("/sys/fs");
  make_directory((char *)"cgroup");
  chdir("/sys/fs/cgroup");
  make_directory((char *)"pids");
}

int main (int argc, char *argv[])
{
  // cgroups initialization of directories
  create_pid_directory();
  chdir((char *)"/");

  // create new process
  void* stack = malloc(STACK);
  int child_pid = clone(child, stack + STACK,
                        CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, argv + 1);
  std::cout << "cloned new child. child pid: " << child_pid << std::endl;

  // 2
  // limit the number of processes that can run within the container



  wait(nullptr);
  // 6
  // unmount
  // delete cgroups mkdir
  ret_val = rmdir();

  return 0;
}


//int main(int argc, char* argv[]) {
//  char* _args[] ={"/bin/echo", (char*)argv + 1, (char *)0};
//  int ret = execvp("/bin/echo", _args);
//
//
//  void* stack = malloc(STACK);
//  char* name = "container";
//  int child_pid = clone(child, stack + STACK, SIGCHLD, name);
//  wait(NULL);
//}
