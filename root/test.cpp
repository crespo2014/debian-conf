/*
 * test.cpp
 *
 *  Created on: 16 Jun 2015
 *      Author: lester
 */

#include "sys_linux.h"
#include "preload.h"
#include "tasks.h"

#define TASK_ID(x)  \
  x(none)\
  x(root_fs) \
  x(sys_fs) \
  x(dev_fs)  \
  x(run_fs) \
  x(tmp_fs) \
  x(setup_fs) /* create all directory in krn fs */ \
  x(all_fs) \
  x(acpi)\
  x(hostname) \
  x(deferred) \
  x(readahead) \
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
  x(grp_krn_fs) /* proc sys dev tmp run */ \
  x(grp_all_fs) /* all fs has been set up, any applcation can run now */ \
  x(grp_fs)     /* all fs ready home, data and ... */ \
  x(max)\

#define TO_STRING(id)                 #id
#define TO_NAME(id)               TO_STRING(id),
#define TO_ID(id)                 id ## _id,

typedef enum
{
  TASK_ID(TO_ID)
} task_id;

static const char* getTaskName(task_id )
{
  return "";
}

int main()
{
  static const Tasks<task_id>::task_info_t tasks[] = {    //

          { &SysLinux::readahead, readahead_id, grp_none_id, none_id, none_id },    //
          { &SysLinux::mount_root, root_fs_id, grp_krn_fs_id, none_id, none_id },    //
          { &SysLinux::mount_sysfs, sys_fs_id, grp_krn_fs_id, none_id, none_id },    //
          { &SysLinux::mount_devfs, dev_fs_id, grp_krn_fs_id, none_id, none_id },    //
          { &SysLinux::mount_tmp, tmp_fs_id, grp_krn_fs_id, none_id, none_id },    //
          { &SysLinux::mount_run, run_fs_id, grp_krn_fs_id, none_id, none_id },    //

          { &SysLinux::deferred_modules, deferred_id, grp_none_id, slim_id, none_id },    //

          { &SysLinux::hostname_s, hostname_id, grp_all_fs_id, none_id, none_id },    //
          { &SysLinux::mount_all, all_fs_id, grp_all_fs_id, dev_fs_id, none_id },    //
          { &SysLinux::setup_fs, setup_fs_id, grp_all_fs_id, grp_krn_fs_id, none_id },    //
          { &SysLinux::udev, udev_id, grp_all_fs_id, grp_krn_fs_id,none_id },    //

          { &SysLinux::acpi_daemon, acpi_id, grp_none_id, grp_all_fs_id, none_id },    //
          { &SysLinux::dbus, dbus_id, grp_none_id, grp_all_fs_id, none_id },    //
  //        { &SysLinux::startX_s, X_id, grp_none_id, grp_all_fs_id, none_id },    //
  //        { &SysLinux::startXfce_s, xfce4_id, grp_none_id, X_id, none_id },    //
          { &SysLinux::slim, slim_id, grp_none_id, acpi_id, dbus_id },    //

          { &SysLinux::procps, procps_id, grp_none_id, deferred_id, none_id },    // last to do
          { &SysLinux::init_d, init_d_id, grp_none_id, grp_all_fs_id,deferred_id  },

          };
      Tasks<task_id> scheduler(tasks, tasks + sizeof(tasks) / sizeof(*tasks), &getTaskName);
      scheduler.start(5,nullptr);

}

