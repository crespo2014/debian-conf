/*
 * sys_linux.h
 *  Class containing usefully functions to be use in linux
 *
 *  Created on: 19 Jun 2015
 *      Author: lester
 */

#ifndef SYS_LINUX_H_
#define SYS_LINUX_H_

#include <vector>
#include <cstring>

#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <unistd.h>
#include <stdarg.h>

class SysLinux
{
public:
  /*
   * Split element delimiter by spaces
   */
  static void split(char* list, std::vector<char*>& v)
  {
    char *cptr = list;
    do
    {
      // find letter
      while (*cptr == ' ' && *cptr != 0)
        ++cptr;
      if (*cptr == '\'')    // argument with delimiters
      {
        ++cptr;
        v.push_back(cptr);
        while (*cptr != '\'' && *cptr != 0)
          ++cptr;
        if (*cptr == '\'')
        {
          *cptr = 0;
          ++cptr;
        }
      } else if (*cptr != 0)
      {
        v.push_back(cptr);
        //next space
        while (*cptr != ' ' && *cptr != 0)
          ++cptr;
        if (*cptr == ' ')
        {
          *cptr = 0;
          ++cptr;
        }
      }

    } while (*cptr != 0);
  }
  /*
   * Execute a system command
   * wait if true wait for command to finish
   * no fork the current process
   */
  static int execute(char* cmd, bool wait = true, bool nofork = false)
  {
    std::vector<char*> arg;
    arg.reserve(10);
    split(cmd, arg);
    arg.push_back(nullptr);
    return launch(wait, arg.data(), nofork);
  }
  /*
   * Execute command from const char*
   */
  static int execute_c(const char* ccmd, bool wait = false, bool fork = true)
  {
    char cmd[512];
    strcpy(cmd, ccmd);
    return SysLinux::execute(cmd, wait, !fork);
  }

  // do not forget (char*) nullptr as last argument
  static int launch(bool wait, char * const * argv, bool nofork = false)
  {
    if (nofork)
    {
      execv(argv[0], (char* const *) argv);
      _exit (EXIT_FAILURE);
    }
    int status;
    pid_t pid = fork();
    if (pid == -1)
    {
      perror(argv[0]);
      status = -1;
    } else if (pid > 0)
    {
      if (wait)
        waitpid(pid, &status, 0);
    }
    if (pid == 0)
    {
      // child
      execv(argv[0], (char* const *) argv);
      _exit (EXIT_FAILURE);
    }
    return status;
  }
};

#endif /* SYS_LINUX_H_ */
