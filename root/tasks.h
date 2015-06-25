/*
 * tasks.h
 *  Implementing task execution.
 *  Task id is a template (none_id and grp_none_id,max_id must be defined )
 *  task name (a function need to be provieded )
 *  Created on: 22 Jun 2015
 *      Author: lester.crespo
 */

#ifndef ROOT_TASKS_H_
#define ROOT_TASKS_H_

#include <condition_variable>
#include <mutex>
#include <iostream>
#include <list>
#include <signal.h>
#include <thread>




template<class ID>
class Tasks
{
public:
  typedef void (*task_fnc_t)(void*);                      ///< task function prototype
  typedef const char* (*get_task_name_fnc_t)(ID);      ///< get task name function prototype
  // task static data
   struct task_info_t
   {
     task_fnc_t fnc;
     ID id;
     ID grp_id;
     // id of dependencies
     ID parent_id;         // = none_id;
     ID parent_id2;        // = none_id;
   };
  //ctor
  Tasks(const task_info_t* begin, const task_info_t* end, get_task_name_fnc_t get_name_fnc) :
      begin(begin), end(end), getTaskName(get_name_fnc)
  {
    memset(status, 0, sizeof(status));      //clear all status information
    // update child and group reference counter
    for (const task_info_t* it = begin; it != end; ++it)
    {
      status[it->id].status = waiting;
      if (it->parent_id != ID::none_id)
      {
        ++status[it->parent_id].child_count;
        if (it->parent_id2 != ID::none_id)
          ++status[it->parent_id2].child_count;
      }
      if (it->grp_id != ID::grp_none_id)
      {
        status[it->grp_id].grp_ref++;     // also use as group counter
        status[it->grp_id].status = waiting;
      }
    }
    status[ID::none_id].status = done;
    status[ID::grp_none_id].status = done;

    // to support X server notifycation
    // block SIGUSR1 on main thread
    sigset_t sig_mask;
    sigset_t oldmask;

    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sig_mask, &oldmask);
  }
  // Start execution of task using thread_max threads
  void start(unsigned thread_max,void* thread_data)
  {
    user_param = thread_data;
    std::list < std::thread > threads;
    for (; thread_max > 0; --thread_max)
    {
      threads.emplace_back(sthread, this);
    }
    for (auto &it : threads)
    {
      it.join();
    }
  }

private:
  // Diferent task status
  enum task_status
  {
    disable = 0,    //
    waiting,    //
    running,    //
    done,
  };
  /*
   * This is the dynamic data of the task
   */
  struct task_status_t
  {
    unsigned grp_ref;
    unsigned child_count;
    enum task_status status;
    struct timespec started;
    struct timespec ended;    // task spend time
  };

  // disable ctor
  Tasks(const Tasks&) = delete;
  Tasks(const Tasks&&) = delete;
  Tasks& operator =(const Tasks&) = delete;
  Tasks& operator =(const Tasks&&) = delete;

  // Peek a new task from list , mark the incoming task as done
  const task_info_t* peekTask(const task_info_t* it)
  {
    bool towait;    // if true means wait for completion, false return current task or null
    unsigned child_count = 0; // how many child to wakeup
    std::unique_lock < std::mutex > lock(mtx);
    if (it != nullptr)
    {
      status[it->id].status = done;
      child_count += status[it->id].child_count;
      if (it->grp_id != ID::grp_none_id)
      {
        --status[it->grp_id].grp_ref;
        if (status[it->grp_id].grp_ref == 0)
        {
          child_count += status[it->grp_id].child_count;
          status[it->grp_id].status = done;
        }
      }
      // put back all done task
      if (it == begin)
      {
        while (begin != end && status[begin->id].status == done)
          ++begin;
      }
    }
    for (;;)
    {
      towait = false;    // no task to waiting for
      for (it = begin; it != end; ++it)
      {
        // find any ready task
        if (status[it->id].status == waiting)
        {
          if (status[it->parent_id].status == done && status[it->parent_id2].status == done)
          {
            status[it->id].status = running;
            break;
          }
          towait = true;
        }
      }
      if (it != end)
        break;     // get out for ;;
      // we got nothing
      if (towait)    // wait and try again
      {
        std::cout << 'W' << std::endl;
        cond_var.wait(lock);
      }
      else
      {
        it = nullptr;    // we done here
        child_count = 3;  // wake_up all task because we done
        break;    // get out for ;;
      }
    }    // for ;; loop
    /*
     * Actions to take when tasks are release
     * 1 - nothing we already pick this task
     * 2 - One thread need to wake up to pick the second task
     * 3..n - All thread need to wake up to pick 2 task
     */
    if (child_count != 0)
    {
      if (child_count > 2)
        cond_var.notify_all();
      else if (child_count == 2)
        cond_var.notify_one();
    }
    return it;
  }

  void print_statics(void* p)
  {
	  Tasks* lnx = reinterpret_cast<Tasks*>(p);
    for (auto *t = lnx->begin; t != lnx->end; ++t)
    {
      auto &st = lnx->status[t->id];
      std::cout << getTaskName(t->id) << " " << (st.ended.tv_nsec / 1000000 + st.ended.tv_sec * 1000) - (st.started.tv_nsec / 1000000 + st.started.tv_sec * 1000) << " ms" << std::endl;
    }

  }
  static void sthread(Tasks* lnx)
  {
    const task_info_t* t = nullptr;
    for (t = lnx->peekTask(t); t != nullptr; t = lnx->peekTask(t))
    {
      std::cout << " S " << lnx->getTaskName(t->id) << std::endl;
      clock_gettime(CLOCK_MONOTONIC, &lnx->status[t->id].started);
      t->fnc(lnx->user_param);
      clock_gettime(CLOCK_MONOTONIC, &lnx->status[t->id].ended);
      std::cout << " E " << lnx->getTaskName(t->id) << " " << (lnx->status[t->id].ended.tv_nsec / 1000000 + lnx->status[t->id].ended.tv_sec * 1000) - (lnx->status[t->id].started.tv_nsec / 1000000 + lnx->status[t->id].started.tv_sec * 1000) << " ms"
          << std::endl;
    }
  }

  std::mutex mtx;
  std::condition_variable cond_var;
  const task_info_t* begin = nullptr, * const end = nullptr;
  struct task_status_t status[ID::max_id];
  get_task_name_fnc_t& getTaskName;
  void *user_param = nullptr;
};

#endif /* ROOT_TASKS_H_ */
