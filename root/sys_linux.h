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
#include <sys/ptrace.h>
#include <asm/unistd.h>
#include <linux/sched.h>
//#include <linux/ioprio.h>
//#include <ext2fs/ext2fs.h>
//#include <blkid/blkid.h>

// Execute system function and print error information if return code is not ecode
#define CHECK_EQUAL(fnc,ecode,msg)  if (fnc != ecode) perror(msg);
#define CHECK_ZERO(fnc,msg)         CHECK_EQUAL(fnc,0,msg)
#define CHECK_NOT(fnc,ecode,msg)    if (fnc == ecode) perror(msg);

#define IOPRIO_WHO_PROCESS 1
#define IOPRIO_CLASS_IDLE 3
#define IOPRIO_CLASS_SHIFT 13
#define IOPRIO_IDLE_LOWEST (7 | (IOPRIO_CLASS_IDLE << IOPRIO_CLASS_SHIFT))

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
    int status = 0;
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
  static void mount_root(void*)
  {
    CHECK_ZERO(mount("/dev/sda5", "/", "ext4", MS_NOATIME | MS_NODIRATIME | MS_REMOUNT | MS_SILENT, ""),"remount /");
  }
  // mount all filesystem in fstab
  static void mount_all(void*)
  {
    CHECK_ZERO(execute_c("/bin/mount -a",true), "mount all");
  }
  static void mount_procfs(void*)
  {
    CHECK_ZERO(mount("", "/proc", "proc", MS_NOATIME | MS_NODIRATIME | MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""), "mount /proc");
  }
  static void mount_sysfs(void*)
  {
    CHECK_ZERO(mount("sys", "/sys", "sysfs", MS_NOATIME | MS_NODIRATIME | MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""), "mount sys");
  }
  static void mount_devfs(void*)
  {
    CHECK_ZERO(mount("dev", "/dev", "devtmpfs", MS_SILENT, ""), "mount dev");
    CHECK_ZERO(mkdir("/dev/pts", 0755), "mkdir /dev/pts");
    CHECK_ZERO(mount("pts", "/dev/pts", "devpts", MS_SILENT | MS_NOSUID | MS_NOEXEC, "gid=5,mode=620"), "mount pts");
  }
  /*
   * Prepare all links and directories
   */
  static void mount_run(void*)
  {
    CHECK_ZERO(mount("run", "/run", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""), "mount /run ");
    CHECK_ZERO(mkdir("/run/lock", 01777), "mkdir /run/lock");
    CHECK_ZERO(mkdir("/run/shm", 01777), "mkdir /run/shm");
    CHECK_ZERO(chmod("/run/shm", 01777), "chmod /run/shm");
    CHECK_ZERO(chmod("/run/lock", 01777), "chmod /run/lock");
    CHECK_ZERO(symlink("/run", "/var/run"), "symlink /run /var/run");
    CHECK_ZERO(symlink("/run/lock", "/var/lock"), "symlink /run/lock /var/lock");
    CHECK_ZERO(symlink("/run/shm", "/dev/shm"), "symlink /run/shm /dev/shm");    // depends on dev_fs
  }
  static void mount_tmp(void*)
  {
    CHECK_ZERO(mount("tmp", "/tmp", "tmpfs", MS_NODEV | MS_NOEXEC | MS_SILENT | MS_NOSUID, ""), "mount /tmp");
    mkdir("/tmp/.X11-unix", 01777);
    chmod("/tmp/.X11-unix", 01777);
    mkdir("/tmp/.ICE-unix", 01777);
    chmod("/tmp/.ICE-unix", 01777);
  }
  static void deferred_modules(void*)
  {
    FILE * pFile;
    pFile = fopen("/proc/deferred_initcalls", "r");
    if (pFile == NULL)
      perror("/proc/deferred_initcalls");
    else
    {
      fclose(pFile);
    }
  }
  static void start_udev(void*)
  {
    execute_c("/etc/init.d/udev start", true);
    /*
     *  symlink("/dev/MAKEDEV", "/bin/true");
     *   SysLinux::execute_c("/sbin/udevd --daemon");  // move to the end be carefull with network cards
     */
  }
  static void set_disk_scheduler(const char* disk, const char* scheduler)
  {
    char tstr[255];
    sprintf(tstr, "/sys/block/%s/queue/scheduler", disk);
    auto fd = open(tstr, O_WRONLY);
    if (fd > 0)
    {
      sprintf(tstr, "%s\n", scheduler);
      write(fd, tstr, strlen(tstr));
    }
    perror(tstr);
  }
  static void set_cpu_governor(const char* )
  {

  }
  static inline void ioprio_set(int , int , int )
  {
    //return syscall(SYS_ioprio_set, which, who, ioprio);
  }

};

#endif /* SYS_LINUX_H_ */
