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


//void print_cur_dir(){
//  char tmp[256];
//  getcwd(tmp, 256);
//  std::cout << "Get: " << tmp << std::endl;
//}

void make_directory (char *name)
{
  if (access (name, F_OK) == -1)  // TODO check mode is relevant here
    {
      if (mkdir (name, MKDIR_MODE) != 0)
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
  chdir ("sys");
  make_directory ((char *) "fs");
  chdir ("fs");
  make_directory ((char *) "cgroup");
  chdir ("cgroup");
  make_directory ((char *) "pids");
  chdir ("pids");
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

  char **temp = (char**) args;
  std::cout << "running child function" << std::endl;
//  for (int  i = 0; i < 5; i++){
//      std::cout << "args[" << i << "] = " << *(temp + i) << std::endl;
//    }

  // change root folder
  char *path = *(temp + 2);
  std::cout << "got path: " << path << std::endl;
  chroot (path);
  chdir (path);

  // cgroups initialization of directories
  create_cgroup_directories_pids ();

  // change hostname
  char *name = *(temp + 1);
  std::cout << "got hostname: " << name << std::endl;
  sethostname (name, strlen (name));

  write_int_to_file ((char *) "/sys/fs/cgroup/pids/cgroup.procs",
                     getpid ());
  write_int_to_file ((char *) "/sys/fs/cgroup/pids/pids.max",
                     atoi(*(temp + 3)));

  // mount new procfs
  chdir (path);
//  std::cout << "changed dir to: " << path << std::endl;

  mount ("proc", "/proc", "proc", 0, 0);

  // run terminal/new program
  char *_args[] = {*(temp + 4), *(temp + 5), (char *) 0};
  int ret = execvp (*(temp + 4), _args);


  // notify on release:
  write_int_to_file ((char *) "/sys/fs/cgroup/pids/notify_on_release",
                     1);

  return 0;
}

void remove_directory (char path[])
{
  if (rmdir (path) != 0)
    {
      std::cerr
          << "system error: failed removing directory: " <<
          path << " " << errno << std::endl;
      exit (1);
    }
}

void signal_handler (char *path)
{
  // unmount
  umount (path);
  std::cout << "unmounted officially!" << std::endl;

  // remove files
  chdir ((char *) "/sys/fs/cgroup/pids");
  int ret = remove ((char *) "cgroup.procs");
  std::cout << "file: " << "cgroup.procs" << " removed. ret val is: " << ret << std::endl;

  ret = remove ((char *) "pids.max");
  std::cout << "file: " << "pids.max" << " removed. ret val is: " << ret << std::endl;
  ret = remove ((char *) "notify_on_release");
  std::cout << "file: " << "notify_on_release" << " removed. ret val is: " << ret << std::endl;
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

