/*
 * preload_main.cpp
 *
 *  Created on: 17 Jun 2015
 *      Author: lester
 */

#include <iostream>

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
  if (initfork)
  {
    //do fork
    int pid = fork();
    if (pid == 0)   //child
    {
      p.preload();
      _exit(0);
    }
    if (pid == -1)
    {
      // failed fork
      p.preload();
    }
    char * arg[] = { const_cast<char*>(init_app),  nullptr };
    execv(init_app, arg);
  }
  p.preload();
}



