/*
 * test.cpp
 *
 *  Created on: 16 Jun 2015
 *      Author: lester
 */

#include "sys_linux.h"
#include "preload.h"


int main()
{
  SysLinux::execute_arg({"/bin/cat","/proc/deferred_initcalls",";"});
  SysLinux::execute_arg({"/bin/bash","(ls / ; ls /home)"});
  int pid = fork();
  if (pid == 0)
  {
    SysLinux::execute_arg({"/bin/ls","/"},false,false);
  }
}

