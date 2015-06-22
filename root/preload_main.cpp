/*
 * preload_main.cpp
 *
 *  Created on: 17 Jun 2015
 *      Author: lester
 */

#include <iostream>
#include <thread>
#include "sys_linux.h"
#include "preload.h"

/*
 * usage
 *  -- no arg --    // preload default file
 * sort <filename>  // load the file, sort it and write to console
 * load <file>      // load the file
 * file <file>      // use this file
 *
 */

int main(int ac, char** av)
{
  constexpr const char * const init_app = "/sbin/init";
  bool bootchartd = false;
  bool cinit = false;
  bool single = false;
  bool initfork = (getpid() == 1);
  char tstr[255];
  SysLinux::mount_procfs(nullptr);
  SysLinux::mount_sysfs(nullptr);
  int fd;
  CHECK_NOT((fd = open("/proc/cmdline", O_RDONLY)), -1, "/proc/cmdline");
  if (fd > 0)
  {
    std::vector<char*> cmdline;
    cmdline.reserve(15);
    *tstr = 0;
    int r = read(fd, tstr, sizeof(tstr) - 1);
    if (r > 0)
      tstr[r - 1] = 0;     // remove ending \n
    close(fd);
    SysLinux::split(tstr, cmdline);
    for (auto p : cmdline)
    {
      if (strcmp(p, "cinit") == 0)
      {
        cinit = true;
      } else if (strcmp(p, "bootchart") == 0)
      {
        bootchartd = true;
      } else if (strcmp(p, "single") == 0)
      {
        single = false;
        break;
      }
    }
  }
  // single user mode with fork
  if (single && initfork)
  {
    char * arg[] = { const_cast<char*>(init_app), nullptr };
    execv(init_app, arg);
  }
  if (bootchartd)
    SysLinux::execute_c("/lib/bootchart/bootchart-collector 50");
  if (cinit)
    setenv("CINIT","1",true);   // avoid run level S from starting

//SysLinux::set_disk_scheduler("sda","noop");

  const char *fname = "/var/lib/e4rat/startup.log";

  bool sort = false;
  int it = 0;
  ++it;
  while (it < ac)
  {
    if (strcmp(av[it], "load") == 0)
    {
      ++it;
      if (it < ac)
        fname = av[it];
    } else if (strcmp(av[it], "file") == 0)
    {
      ++it;
      if (it < ac)
        fname = av[it];
    } else if (strcmp(av[it], "sort") == 0)
    {
      sort = true;
      ++it;
      if (it < ac)
        fname = av[it];
    }
    ++it;
  }
  preload_parser p;
  p.loadFile(fname);
  // reduce priority
  //SysLinux::ioprio_set(IOPRIO_WHO_PROCESS, getpid(), IOPRIO_IDLE_LOWEST);
  if (sort)
  {
    p.Merge();
    p.UpdateBlock();
    p.WriteOut();
  }
  int pid = -1;       // simulate not child
  p.preload(100);
  if (initfork)
  {
    pid = fork();
  }
// Call deferred and preload if (child process or parent process without child)
  if (pid == 0 || pid == -1)    //child or fail
  {
    p.preload();
    //p.preload();
    //SysLinux::set_disk_scheduler("sda", "cfq");
  }
  if (pid == 0)
    _exit(0);     // end child

// we definitly call init
  if (initfork)
  {
    char * arg[] = { const_cast<char*>(init_app), nullptr };
    execv(init_app, arg);
  }
  return 0;
}

