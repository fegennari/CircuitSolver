#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

#include "CircuitSolver.h"
#include "STL_Support.h"

#include <algorithm>
#include <queue>

// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, Schedule.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver task scheduling header
// By Frank Gennari
#define eventlist  vector<event>
#define periodlist vector<unsigned>
#define tasklist   vector<unsigned>
#define rsclist    vector<unsigned>

#define SCHEDULE_OUT_FILE "Schedule.txt"

#define DEF_RSC   1
#define IDLE_TASK (unsigned)-1 // max unsigned to be sure there are no real tasks with this id

#define SCH_TEXT_SIZE_S       9
#define SCH_TEXT_SIZE_L       12
#define MAX_DISPLAY_TASKS     12
#define MAX_L_TEXT_TIMESTEPS  60
#define MAX_DISPLAY_TIMESTEPS 100
#define MAX_DISPLAY_PROCS     100
#define MAX_SCH_DISP_CHAR     256

#define SCH_HILITE_COLOR      LTGREY


enum {TASK_FAIL = 0, TASK_EXEC_INVALID, TASK_EXEC, TASK_END, TASK_START, TASK_EXEC2, TASK_NULL};


struct task {

	unsigned comp_total, period, deadline, comp_remaining, rsc_lock, priority;
	char     is_enabled; // redundant, a quick shortcut to replace some memory reads
};


struct task_p { // used for rate monotonic priority sorting

	unsigned comp_total, period, deadline, id;
};


struct event {

	unsigned  task_id, proc_id;
	char      type; /* start = 0, end = 1 */
	unsigned *priority; // used to reference into dynamically assigned task priorities
};


struct proc_exec {

	int      task;
	unsigned proc, priority;
};


struct task_lt_priority {

	task const * const tasks;

	task_lt_priority(task const * const tasks_):tasks(tasks_){};

	bool operator()(unsigned const t1i, unsigned const t2i) const {

		task const * const t1 = &tasks[t1i];
		task const * const t2 = &tasks[t2i];

		if (t1->priority   < t2->priority)   return true;
		if (t1->priority   > t2->priority)   return false;
		if (t1->period     < t2->period)     return true;
		if (t1->period     > t2->period)     return false;
		if (t1->comp_total < t2->comp_total) return true;
		if (t1->comp_total > t2->comp_total) return false;
		if (t1->deadline   < t2->deadline)   return true;
		if (t1->deadline   > t2->deadline)   return false;
		if (t1->rsc_lock   < t2->rsc_lock)   return true;

		return false;
	}
};


struct task_gt_priority {

  task const * const tasks;

	task_gt_priority(task const * const tasks_):tasks(tasks_){};

	bool operator()(unsigned const t1i, unsigned const t2i) const {

		task const * const t1 = &tasks[t1i];
		task const * const t2 = &tasks[t2i];

		if (t1->priority   > t2->priority)   return true;
		if (t1->priority   < t2->priority)   return false;
		if (t1->period     > t2->period)     return true;
		if (t1->period     < t2->period)     return false;
		if (t1->comp_total > t2->comp_total) return true;
		if (t1->comp_total < t2->comp_total) return false;
		if (t1->deadline   > t2->deadline)   return true;
		if (t1->deadline   < t2->deadline)   return false;
		if (t1->rsc_lock   > t2->rsc_lock)   return true;

		return false;
	}
};


typedef priority_queue<unsigned, vector<unsigned>, task_lt_priority> pq_less;
typedef priority_queue<unsigned, vector<unsigned>, task_gt_priority> pq_greater;



#endif
