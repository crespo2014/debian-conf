/*
 * test.cpp
 *
 *  Created on: 16 Jun 2015
 *      Author: lester
 */

#include "sys_linux.h"
#include "preload.h"
#include "tasks.h"

typedef enum
{
  none_id,
  deferred_id,
  readahead_id,
  grp_none_id,
  max_id,
} task_id;

static const char* getTaskName(task_id )
{
  return "";
}

int main()
{
  // Start system scripts
      static const Tasks<task_id>::task_info_t tasks[] = {    //
          { &SysLinux::deferred_modules, deferred_id, grp_none_id, none_id, none_id },    //
         // { &SysLinux::readahead, readahead_id, grp_none_id, none_id, none_id },    //

          };
      Tasks<task_id> scheduler(tasks, tasks + sizeof(tasks) / sizeof(*tasks), &getTaskName);
      scheduler.start(4,nullptr);

}

