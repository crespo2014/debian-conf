/*
 * preload_main.cpp
 *
 *  Created on: 17 Jun 2015
 *      Author: lester
 */

#include "preload.h"
#include <iostream>

/*
 * usage
 *  -- no arg --    // preload default file
 * sort <filename>  // load the file, sort it and write to console
 * init <init_app> <file> // load default file and fork to init_app
 * load <file>      // load the file
 *
 */

int main(int ac, char** av)
{
  const char *fname = "/var/lib/e4rat/startup.log";
  const char *init_app = "/sbin/init";
  bool initfork = false;
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
    } else if (strcmp(av[it], "init") == 0)
    {
      initfork = true;
      ++it;
      if (it < ac)
        init_app = av[it];
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
  if (sort)
  {
    p.Merge();
    p.UpdateBlock();
    p.WriteOut();
  }
  else if (initfork)
  {
    p.preload(100);
    //do fork
    int pid = fork();
    if (pid == 0)
    {
      char * arg[] = { (char*)init_app, nullptr };
      execv(init_app,(char* const *) arg);
      _exit(EXIT_FAILURE);
    }
    p.preload();
    // check if fork failed
    if (pid == -1)
    {
      char * arg[] = { (char*)init_app,  nullptr };
      execv(init_app,(char* const *) arg);
      _exit(EXIT_FAILURE);
    }
  } else
  {
    p.preload();
  }
}



