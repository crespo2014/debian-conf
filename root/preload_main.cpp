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
  // Start bootchartd after checking cmd line for bootchart argument
  SysLinux::execute_c("/lib/bootchart/bootchart-collector 50");
  SysLinux::mount_procfs(nullptr);
  SysLinux::mount_sysfs(nullptr);
  SysLinux::set_disk_scheduler("sda","noop");

  const char *fname = "/var/lib/e4rat/startup.log";
  const char *init_app = "/sbin/init";
  bool initfork = (getpid() == 1);
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
  SysLinux::ioprio_set(IOPRIO_WHO_PROCESS, getpid(),IOPRIO_IDLE_LOWEST);

  if (sort)
  {
    p.Merge();
    p.UpdateBlock();
    p.WriteOut();
  }
  p.preload(100);
  int pid = -1;       // simulate not child
  if (initfork)
  {
    pid = fork();
  }
  // Call deferred and preload if (child process or parent process without child)
  if (pid == 0 || pid == -1)  //child or fail
  {
    std::thread thr(SysLinux::deferred_modules,nullptr);
    p.preload();
    thr.join();
    SysLinux::set_disk_scheduler("sda","cfq");
  }
  if (pid == 0)
    _exit(0);     // end child

  // we definitly call init
  if (initfork)
  {
    char * arg[] = { const_cast<char*>(init_app),  nullptr };
    execv(init_app, arg);
  }
  return 0;
}



