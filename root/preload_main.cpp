/*
 * preload_main.cpp
 *
 *  Created on: 17 Jun 2015
 *      Author: lester
 */

#include <iostream>
#include <thread>
#include "sys_linux.h"
#include "tasks.h"
#include "preload.h"
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>

#define TASK_ID(x)  \
  x(none)\
  x(root_fs) \
  x(sys_fs) \
  x(dev_fs)  \
  x(run_fs) \
  x(tmp_fs) \
  x(all_fs) \
  x(acpi)\
  x(hostname) \
  x(deferred) \
  x(udev)\
  x(X)\
  x(dev_subfs) \
  x(udev_trigger)\
  x(dbus)\
  x(procps)\
  x(xfce4)\
  x(slim) \
  x(init_d)\
  x(grp_none)  /* no group */ \
  x(grp_krn_fs) /* proc sys dev tmp run + setup directories */ \
  x(grp_fs)     /* all fs ready home, data and ... */ \
  x(max)\

#define TO_STRING(id)                 #id
#define TO_NAME(id)               TO_STRING(id),
#define TO_ID(id)                 id ## _id,

typedef enum
{
  TASK_ID(TO_ID)
} task_id;

static const char* getTaskName(task_id id)
{
  static const char* const names[] = { TASK_ID(TO_NAME)"" };
  if (id >= sizeof(names) / sizeof(*names))
    return "";
  return names[id];
}

/*
 * usage
 *  -- no arg --    // preload default file
 * sort <filename>  // load the file, sort it and write to console
 * load <file>      // load the file
 * file <file>      // use this file
 * deferred         // do deferred calls
 *
 */
int main(int ac, char** av)
{
  constexpr const char * const init_app = "/sbin/init";
  const char *fname = "/var/lib/e4rat/startup.log";
  bool bootchartd = false;
  bool single = false;
  bool sort = false;
  bool initfork = (getpid() == 1);
  char tstr[255];
  if (initfork)
  {
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
        if (strcmp(p, "bootchart") == 0)
        {
          bootchartd = true;
        } else if (strcmp(p, "single") == 0)
        {
          single = false;
          break;
        }
      }
    }
    // single user mode, start init
    if (single)
    {
      SysLinux::execute_arg({init_app}, false, false);
    }
    if (bootchartd)
      SysLinux::execute_arg({"/lib/bootchart/bootchart-collector","50"},false);

    SysLinux::execute_arg({av[0],"init"},false);    // by default preload load all files

    setenv("CINIT", "1", true);    // avoid run level S from starting
    // Start system scripts
    static const Tasks<task_id>::task_info_t tasks[] = {    ///
        { &SysLinux::mount_root, root_fs_id, grp_krn_fs_id, none_id, none_id },    //
            { &SysLinux::mount_sysfs, sys_fs_id, grp_krn_fs_id, none_id, none_id },    //
            { &SysLinux::mount_devfs, dev_fs_id, grp_krn_fs_id, run_fs_id, none_id },    //
            { &SysLinux::mount_tmp, tmp_fs_id, grp_krn_fs_id, none_id, none_id },    //
            { &SysLinux::mount_run, run_fs_id, grp_krn_fs_id, none_id, none_id },    //
            { &SysLinux::mount_all, all_fs_id, grp_krn_fs_id, dev_fs_id, none_id },    //
            { &SysLinux::hostname_s, hostname_id, grp_krn_fs_id, none_id, none_id },    //
            { &SysLinux::udev, udev_id, grp_none_id, dev_fs_id, none_id },    //
            { &SysLinux::acpi_daemon, acpi_id, grp_none_id, grp_krn_fs_id, none_id },    //
            { &SysLinux::dbus, dbus_id, grp_none_id, grp_krn_fs_id, none_id },    //
            { &SysLinux::slim, slim_id, grp_none_id, grp_krn_fs_id, none_id },    //
            { &SysLinux::procps, procps_id, grp_none_id, udev_id, none_id },    //
        };
    Tasks<task_id> scheduler(tasks, tasks + sizeof(tasks) / sizeof(*tasks), &getTaskName);
    scheduler.start(4);
    // start init application
    SysLinux::execute_arg({init_app}, false, false);
    _exit(0);
  }
  // running outside init. we sort or load the file
  bool init = false;
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
    } else if (strcmp(av[it], "init") == 0)
    {
      init = true;
    }
    ++it;
  }
  if (init) // called by himself at init
  {
    setpriority(PRIO_PROCESS, getpid(), 10);
    preload_parser p;
    std::thread t([&]{p.readahead(fname);});
    SysLinux::execute_arg({"/bin/cat","/proc/deferred_initcalls"});
    SysLinux::execute_arg({"/bin/udevadm","trigger"});
    t.join();
  }
  if (sort)
  {
    preload_parser p;
    p.loadFile(fname);
    p.Merge();
    p.UpdateBlock();
    p.WriteOut();
  }
  return 0;
}

