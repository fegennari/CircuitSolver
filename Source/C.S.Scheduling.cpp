#include "Scheduling.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Scheduling.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver task scheduling
// by Frank Gennari
// * NOT YET FINISHED *
extern int x_blocks, y_blocks, x_screen, y_screen, x_limit, y_limit, object_size, whiteout_text_bkg; // graphics variables


// core scheduler
void     schedule_tasks();
int      open_file(ifstream &infile);
int      get_tasks(task *tasks, periodlist &periods, unsigned ntasks, int &has_rsc_dep, ifstream &infile, int read_from_file);
int      verify_task_consistency(task *tasks, unsigned ntasks);
unsigned period_lcm(periodlist &periods, unsigned ntasks);
void     build_timeline(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks);
int      create_schedule(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks, unsigned nprocs, unsigned nrsc, double utilization);
int      verify_schedule(eventlist *timeline, task *tasks, unsigned runtime, unsigned nprocs, unsigned nrsc, int is_preemptive);
void     sort_timeline(eventlist *timeline, unsigned runtime);
void     convert_exec2s(eventlist *timeline, unsigned runtime);
unsigned count_and_remap_resources(task *tasks, unsigned ntasks);

// display
void     print_periods(periodlist &periods);
void     print_schedule(eventlist *timeline, task *tasks, unsigned runtime, int dest);
void     display_schedule(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks, unsigned nprocs, unsigned nrsc, double utilization);

// scheduling algorithms
int      schedule_rate_monotonic(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks, unsigned nprocs, unsigned nrsc, int is_preemptive);
int      schedule_edf(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks, unsigned nprocs, unsigned nrsc, int is_preemptive);
int      schedule_audsley(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks, unsigned nprocs, unsigned nrsc, int is_preemptive);
int      schedule_custom(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks, unsigned nprocs, unsigned nrsc, int is_preemptive);

// scheduling subfunctions
void     assign_rm_priorities(task *tasks, unsigned ntasks);
int      priority_schedule(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks, unsigned nprocs, unsigned nrsc, int is_preemptive);
int      delete_all_ps(pq_less &pending_tasks, proc_exec *executing, unsigned *resources, unsigned *blocked_tasks, int ret_val);
unsigned get_next_deadline(eventlist *timeline, task *tasks, unsigned &t_curr, unsigned runtime);
void     force_non_preemptive(task *tasks, unsigned ntasks);

//ordering
bool operator<(event     A, event       B);
bool operator<(task      A, task        B);
bool operator<(proc_exec A, proc_exec   B);
int  comp_rm(const void *A, const void *B);


// external display functions
void draw_grid(int grid_color);
void set_text_size(int size);
void draw_string(char *display);
void draw_text(string text, int box_num, int t_size);
int decision_dialog(int id);



// core scheduler
void schedule_tasks() {

	int         print_type(2), has_rsc_dep(0), read_from_file(0), redraw(1);
	unsigned    i, ntasks, nprocs, runtime, total_comp(0), nrsc(0);
	double      utilization;
	task       *tasks    = NULL;
	eventlist  *timeline = NULL;
	periodlist  periods;
	ifstream    infile;

	cout << "Read task list from file? ";
	if (decision()) {
		if (!open_file(infile))
			return;
		
		if (!(infile >> ntasks >> nprocs)) {
			cerr << "Error reading the number of tasks and processors from the input file." << endl;
			return;
		}
		read_from_file = 1;
	}
	else {
		cout << "Enter the number of tasks to schedule:  ";
		ntasks = inp();
		cout << "Enter the number of processors:  ";
		nprocs = inp();
	}
	if (ntasks == 0 || nprocs == 0) {
		return;
	}
	tasks = memAlloc(tasks, ntasks);

	if (!get_tasks(tasks, periods, ntasks, has_rsc_dep, infile, read_from_file)) {
		cerr << "Error reading task information." << endl;
		if (read_from_file) {
			infile.close();
		}
		delete [] tasks;
		return;
	}
	if (read_from_file) {
		infile.close();
	}
	if (!verify_task_consistency(tasks, ntasks)) {
		cout << "There is no valid schedule." << endl;
		delete [] tasks;
		return;
	}
	runtime  = period_lcm(periods, ntasks);
	timeline = memAlloc(timeline, runtime+1);

	for (i = 0; i < ntasks; ++i) {
		total_comp += runtime*tasks[i].comp_total/tasks[i].period;
	}
	utilization = (double)total_comp/((double)runtime*(double)nprocs);

	cout << "Runtime = " << runtime << "." << endl;
	cout << "Total computation = " << total_comp << "." << endl;
	cout << "Available computation = " << runtime*nprocs << "." << endl;
	cout << "Processor utilization = " << utilization << "." << endl;

	if (utilization > 1.0) {
		cout << "These tasks require more than the available time or processing power. There is no valid schedule." << endl;
	}
	else { 
		build_timeline(timeline, tasks, runtime, ntasks);

		if (has_rsc_dep) {
			nrsc = count_and_remap_resources(tasks, ntasks);
		}
		if (create_schedule(timeline, tasks, runtime, ntasks, nprocs, nrsc, utilization)) {
			cout << "The schedule is valid." << endl;
		}
		else {
			cout << "The schedule is invalid." << endl;
		}
		sort_timeline(timeline, runtime);

		cout << "Print schedule to screen?  ";
		if (decision()) {
			print_type = 1;
		}
		print_schedule(timeline, tasks, runtime, print_type);

		while (redraw) {
			display_schedule(timeline, tasks, runtime, ntasks, nprocs, nrsc, utilization);
			redraw = decision_dialog(5);
		}
	}
	for (i = 0; i <= runtime; ++i) {
		vector_delete(timeline[i]);
	}
	delete [] timeline;
	delete [] tasks;
}




int open_file(ifstream &infile) {

	int  status;
	char filename[MAX_SAVE_CHARACTERS];

	do {
		cout << "Enter the name of the input file:  ";
		cin  >> ws;
		strcpy(filename, xin(MAX_SAVE_CHARACTERS));
		status = 1;

		if (!infile_file(infile, filename)) {
			beep();
			cerr << "\nThe input file " << filename << " could not be opened!" << endl;
			status = 0;
		}
		if (!filestream_check(infile))
			status = 0;
		
		if (status == 0) {
			cout << "Try again?  ";
			if (decision()) {
				infile.clear();
			}
			else {
				return 0;
			}
		}
	} while (status == 0);

	return 1;
}




int get_tasks(task *tasks, periodlist &periods, unsigned ntasks, int &has_rsc_dep, ifstream &infile, int read_from_file) {

	unsigned i;

	for (i = 0; i < ntasks; ++i) {
		if (read_from_file) {
			if (!filestream_check(infile))
				return 0;
			
			if (!(infile >> tasks[i].comp_total >> tasks[i].period >> tasks[i].deadline >> tasks[i].rsc_lock)) {
				cerr << "Error reading task " << i << "." << endl;
				return 0;
			}
			if (tasks[i].comp_total == 0 || tasks[i].period == 0 || tasks[i].deadline == 0) {
				cerr << "Error in task " << i << ": Computation, period, and deadline must be nonzero." << endl;
				return 0;
			}
		}
		else {
			cout << "Enter the computation time, period, deadline, and resource id (0 for none) of task " << (i+1) << ":  ";
			tasks[i].comp_total     = inpg0();
			tasks[i].period         = inpg0();
			tasks[i].deadline       = inpg0();
			tasks[i].rsc_lock       = inp();
		}
		tasks[i].comp_remaining = 0;
		tasks[i].priority       = 0; // unassigned
		periods.push_back(tasks[i].period);
		if (tasks[i].rsc_lock > 0) {
			has_rsc_dep = 1; // assuming not all tasks have unique resources
		}
	}
	return 1;
}




int verify_task_consistency(task *tasks, unsigned ntasks) {

	unsigned i;
	task     T;

	for (i = 0; i < ntasks; ++i) {
		T = tasks[i];
		if (T.comp_total > T.deadline) {
			cout << "Task " << (i+1) << ": Computation time is larger than deadline." << endl;
			return 0;
		}
		if (T.comp_total > T.period) {
			cout << "Task " << (i+1) << ": Computation time is larger than period." << endl;
			return 0;
		}
		if (T.deadline > T.period) {
			cout << "Task " << (i+1) << ": Deadline is larger than period." << endl;
			return 0;
		}
	}
	return 1;
}




unsigned period_lcm(periodlist &periods, unsigned ntasks) {

	int        add_period, found_div;
	unsigned   i, j, lcm(1), last_period(0), limit, psize;
	periodlist periods_f, periods_f2;

	sort(periods.begin(), periods.end());

	for (i = 0; i < periods.size(); ++i) { // filter out repeated periods
		if (periods[i] != last_period) {
			periods_f.push_back(periods[i]);
			last_period = periods[i];
		}
	}
	vector_delete(periods);

	for (i = 0; i < periods_f.size(); ++i) { // filter out periods which are factors of larger periods
		add_period = 1;
		for (j = i + 1; add_period && j < periods_f.size(); ++j) {
			if (periods_f[j]%periods_f[i] == 0) // period i is a factor of period j
				add_period = 0;
		}
		if (add_period) {
			periods_f2.push_back(periods_f[i]);
		}
	}
	//print_periods(periods_f2); // testing
	vector_delete(periods_f);
	psize = periods_f2.size();
	limit = (unsigned)ceil(sqrt((double)periods_f2[psize-1]));

	for (j = 2; j <= limit; ++j) {
		found_div = 0;
		for (i = 0; i < psize; ++i) { // extract gcds
			if (periods_f2[i] >= j && periods_f2[i]%j == 0) { // period i is divisible by j
				if (found_div)
					periods_f2[i] /= j;
				else
					found_div = 1;
			}
		}
	}
	for (i = 0; i < psize; ++i) { // lcm is the product
		lcm *= periods_f2[i];
	}
	vector_delete(periods_f2);

	return lcm;
}




void build_timeline(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks) {

	unsigned i, k, period, deadline;
	event    Es, Ee;

	Es.type    = TASK_START;
	Ee.type    = TASK_END;
	Es.proc_id = 0;
	Ee.proc_id = 0;

	for (i = 0; i < ntasks; ++i) {
		period   = tasks[i].period;
		deadline = tasks[i].deadline;
		for (k = 0; k < runtime/period; ++k) { // loop through each periodic task event in the runtime
			Es.task_id  = Ee.task_id  = i;
			Es.priority = Ee.priority = &(tasks[i].priority);
			timeline[k*period].push_back(Es);            // start event at kPi
			timeline[k*period + deadline].push_back(Ee); // end event   at kPi + Di
		}
	}
}




int create_schedule(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks, unsigned nprocs, unsigned nrsc, double utilization) {

	int      is_preemptive, pi_equals_di(1);
	unsigned i, s_type;
	double   util_lower_bound;

	cout << "Use preemptive schedule?  ";
	is_preemptive = decision();

	do {
		cout << "Choose a schedule: 0 = rate monotonic, 1 = EDF, 2 = Audsley, 3 = custom (user entered).  ";
		s_type = inp();
	} while (s_type > 3);

	switch (s_type) {
	case 0:
		if (is_preemptive && nrsc == 0 && nprocs == 1) {
			util_lower_bound = ((double)ntasks)*(pow(2.0, 1.0/((double)ntasks)) - 1.0);
			if (utilization < util_lower_bound) { // <= ?
				cout << "This set of tasks should be schedulable because the utilization factor is below the lower bound of " << util_lower_bound << "." << endl;
			}
		}
		for (i = 0; i < ntasks; ++i) {
			if (tasks[i].period != tasks[i].deadline) {
				pi_equals_di = 0;
				break;
			}
		}
		if (pi_equals_di) {
			cout << "The rate monotonic schedule will be optimal for this set of tasks since Pi = Di for each i." << endl;
		}
		return schedule_rate_monotonic(timeline, tasks, runtime, ntasks, nprocs, nrsc, is_preemptive);

	case 1:
		if (is_preemptive && nrsc == 0 && nprocs == 1) {
			if (utilization < 1.0) { // <= ?
				cout << "This set of tasks should be schedulable because the utilization factor is less than 1.0." << endl;
			}
		}
		return schedule_edf(timeline, tasks, runtime, ntasks, nprocs, nrsc, is_preemptive);

	case 2:
		return schedule_audsley(timeline, tasks, runtime, ntasks, nprocs, nrsc, is_preemptive);

	case 3:
		return schedule_custom(timeline, tasks, runtime, ntasks, nprocs, nrsc, is_preemptive);
	}
	return 0;
}




int verify_schedule(eventlist *timeline, task *tasks, unsigned runtime, unsigned nprocs, unsigned nrsc, int is_preemptive) { // generic case - used for custom schedules

	unsigned   i, j, task_id, proc_id, rsc_id;
	unsigned  *resources = NULL, *proc_exec = NULL;
	char      *proc_used = NULL;
	eventlist  el;
	event      E, E_fail;

	E_fail.priority = NULL;
	proc_used       = memAlloc(proc_used, nprocs);
	proc_exec       = memAlloc_init(proc_exec, nprocs, (unsigned)0);
	resources       = memAlloc_init(resources, nrsc+1, (unsigned)0); // shouldn't need if nrsc == 0
	
	for (i = 0; i <= runtime; ++i) {
		el = timeline[i];

		if (el.size() > 0) {
			for (j = 0; j < nprocs; ++j) {
				proc_used[j] = 0;
			}
		}
		for (j = 0; j < el.size(); ++j) {
			E       = el[j];
			task_id = E.task_id;

			if (task_id == IDLE_TASK) // should not happen, but just in case ...
				continue;

			switch (E.type) {
			case TASK_START:
				tasks[task_id].comp_remaining = tasks[task_id].comp_total; // reset computation counter
				tasks[task_id].is_enabled     = 1;
				break;

			case TASK_END:
				if (tasks[task_id].comp_remaining > 0) { // computation not finished
					cout << "Task " << (task_id+1) << " missed its deadline at time " << i << " with " << tasks[task_id].comp_remaining << " unit(s) of computation remaining." << endl;
					E_fail.type    = TASK_FAIL;
					E_fail.proc_id = 0;
					E_fail.task_id = task_id;
					timeline[i].push_back(E_fail); // actually, should be push_front
					sort(timeline[i].begin(), timeline[i].end()); // fix the order
					delete [] proc_used;
					delete [] proc_exec;
					delete [] resources;
					return 0;
				}
				tasks[task_id].is_enabled = 0;
				break;

			case TASK_EXEC:
				proc_id = E.proc_id;

				if (tasks[task_id].comp_remaining == 0 || tasks[task_id].is_enabled == 0) { // should be error
					cout << "Task " << (task_id+1) << " has already been completed and cannot be processed on processor " << (proc_id+1) << " at time " << i << "." << endl;
				}
				else if (proc_used[proc_id] != 0) { // two tasks executing on the same processor
					cout << "Task " << (task_id+1) << " cannot be executed on processor " << (proc_id+1) << " at time " << i << " because the processor is busy with another task." << endl;
				}
				else if (!is_preemptive && proc_exec[proc_id] != task_id+1 && proc_exec[proc_id] != 0) { // preemption
					cout << "Task " << (task_id+1) << " has preempted task " << proc_exec[proc_id] << " running on processor " << (proc_id+1) << " at time " << i << " in a non-preemptive schedule." << endl;
				}
				else { // valid task execution
					++proc_used[proc_id];
					tasks[task_id].comp_remaining--;

					if (tasks[task_id].comp_remaining == 0) {
						proc_exec[proc_id] = 0; // processor free
					}
					else {
						proc_exec[proc_id] = task_id+1; // processor occupied by task
					}
					if (nrsc > 0) {
						rsc_id = tasks[task_id].rsc_lock;

						if (resources[rsc_id] != 0 && resources[rsc_id] != task_id+1) { // resource already in use by another task
							cout << "Task " << (task_id+1) << " cannot be executed on processor " << (proc_id+1) << " because of a sharing violation of resource " << rsc_id << " at time " << i << "." << endl;
						}
						else if (tasks[task_id].comp_remaining == 0) { // finished
							resources[rsc_id] = 0; // unlock resource
							break;
						}
						else {
							resources[rsc_id] = task_id+1; // lock resource
							break;
						}
					}
					else {
						break;
					}
				}
				E_fail.type    = TASK_EXEC_INVALID;
				E_fail.proc_id = proc_id;
				E_fail.task_id = task_id;
				timeline[i][j].type = TASK_NULL;
				timeline[i].push_back(E_fail); // actually, should be push_front
				sort(timeline[i].begin(), timeline[i].end()); // fix the order
			case TASK_FAIL: // should have returned already
				delete [] proc_used;
				delete [] proc_exec;
				delete [] resources;
				return 0;

			case TASK_NULL:
				break;

			default:
				cerr << "Illegal event type: " << (int)((unsigned char)E.type) << "." << endl;
				delete [] proc_used;
				delete [] proc_exec;
				delete [] resources;
				return 0;
			}
		}
	}
	delete [] proc_used;
	delete [] proc_exec;
	delete [] resources;

	return 1;
}




void sort_timeline(eventlist *timeline, unsigned runtime) {

	unsigned i;
	
	for (i = 0; i <= runtime; ++i) { // order events for simulating/printing/displaying
		sort(timeline[i].begin(), timeline[i].end());
	}
}




void convert_exec2s(eventlist *timeline, unsigned runtime) {

	unsigned i, j;
	
	for (i = 0; i <= runtime; ++i) {
		for (j = 0; j < timeline[i].size(); ++j) { // replace all TASK_EXEC2's with TASK_EXEC's
			if (timeline[i][j].type == TASK_EXEC2) {
				timeline[i][j].type =  TASK_EXEC;
			}
		}
	}
}




unsigned count_and_remap_resources(task *tasks, unsigned ntasks) {

	unsigned  i, nrsc(0), last_rsc(0), max_rsc(0);
	unsigned *rsc_mapping = NULL;
	rsclist   resources;

	for (i = 0; i < ntasks; ++i) {
		if (tasks[i].rsc_lock > 0) {
			resources.push_back(tasks[i].rsc_lock);
			max_rsc = max(max_rsc, tasks[i].rsc_lock);
		}
	}
	rsc_mapping    = memAlloc(rsc_mapping, max_rsc+1); // could be alot of memory if large resource ids are used
	rsc_mapping[0] = 0;
	sort(resources.begin(), resources.end());

	for (i = 0; i < resources.size(); ++i) { // filter out repeated resources
		if (resources[i] != last_rsc) {
			++nrsc;
			last_rsc              = resources[i];
			rsc_mapping[last_rsc] = nrsc;
		}
	}
	for (i = 0; i < ntasks; ++i) {
		tasks[i].rsc_lock = rsc_mapping[tasks[i].rsc_lock];
	}
	vector_delete(resources);
	delete [] rsc_mapping;

	return nrsc;
}



// display
void print_periods(periodlist &periods) { // for testing purposes

	unsigned i;

	cout << "periods: ";

	for (i = 0; i < periods.size(); ++i) {
		cout << periods[i] << " ";
	}
	cout << endl;
}



// dest: 0 = stdout, 1 = both, 2 = file
void print_schedule(eventlist *timeline, task *tasks, unsigned runtime, int dest) {

	unsigned  i, j, num, rsc_id;
	eventlist el;
	event     E;
	ofstream  outfile;

	if (dest < 0 || dest > 3)
		return;

	if (dest != 0) {
		outfile.open(SCHEDULE_OUT_FILE, ios::out | ios::trunc);
		if (outfile.fail() || !filestream_check(outfile)) {
			cerr << "Error: Could not write output file " << SCHEDULE_OUT_FILE << "." << endl;
			if (dest == 2) {
				return;
			}
			dest = 0;
		}
	}
	for (i = 0; i <= runtime; ++i) {
		el = timeline[i];
		if (dest != 2)
			cout    << "TIME = " << i << ": ";
		if (dest != 0)
			outfile << "TIME = " << i << ": ";

		if (dest != 0 && !filestream_check(outfile)) {
			if (dest == 2) {
				return;
			}
		}
		for (num = 1; num <= runtime; num *= 10) { // add spaces to align columns
			if (i < num && (i != 0 || num > 1)) {
				if (dest != 2)
					cout    << " ";
				if (dest != 0)
					outfile << " ";
			}
		}
		for (j = 0; j < el.size(); ++j) {
			E = el[j];

			switch (E.type) {
			case TASK_START:
				if (dest != 2)
					cout    << "S";
				if (dest != 0)
					outfile << "S";
				break;

			case TASK_END:
				if (dest != 2)
					cout    << "E";
				if (dest != 0)
					outfile << "E";
				break;

			case TASK_EXEC_INVALID:
				if (dest != 2)
					cout    << "*";
				if (dest != 0)
					outfile << "*";
			case TASK_EXEC:
				rsc_id = tasks[E.task_id].rsc_lock;
				if (rsc_id > 0) {
					if (dest != 2)
						cout    << "R" << rsc_id;
					if (dest != 0)
						outfile << "R" << rsc_id;
				}
				if (dest != 2)
					cout    << "P" << (E.proc_id+1) << "T";
				if (dest != 0)
					outfile << "P" << (E.proc_id+1) << "T";
				break;

			case TASK_FAIL:
				if (dest != 2)
					cout    << "X";
				if (dest != 0)
					outfile << "X";
				break;

			case TASK_NULL:
				break;

			default:
				if (dest != 2)
					cout    << "?";
				if (dest != 0)
					outfile << "?";
			}
			if (E.type != TASK_NULL) {
				if (dest != 2)
					cout    << (E.task_id+1) << " ";
				if (dest != 0)
					outfile << (E.task_id+1) << " ";
			}
		}
		if (dest != 2)
			cout    << endl;
		if (dest != 0)
			outfile << endl;
	}
	if (dest != 0) {
		outfile.close();
	}
}




void display_schedule(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks, unsigned nprocs, unsigned nrsc, double utilization) {

	int       temp_osize(object_size), task_id, wtbkg(whiteout_text_bkg), tsize, error(0);
	unsigned  i, j, y_start, y_end, task_counter;
	double    x_grid, y_grid;
	char      str[MAX_SCH_DISP_CHAR], str2[3], *task_chars = NULL, *task_bounds = NULL;
	eventlist el;
	event     E;

#ifdef TEXT_ONLY
	cerr << "Graphical display of schedules is not available when running in text mode." << endl;
	return;
#endif

	if (ntasks > MAX_DISPLAY_TASKS) {
		cout << "There are too many tasks to display on the screen at once. Max is " << MAX_DISPLAY_TASKS << "." << endl;
		return;
	}
	if (runtime > MAX_DISPLAY_TIMESTEPS) {
		cout << "There are too many timesteps to display on the screen at once. Max is " << MAX_DISPLAY_TIMESTEPS << "." << endl;
		return;
	}
	if (nprocs > MAX_DISPLAY_PROCS) {
		cout << "There are too many processors to display on the screen at once. Max is " << MAX_DISPLAY_PROCS << "." << endl;
		return;
	}
	if (runtime <= MAX_L_TEXT_TIMESTEPS) {
		tsize = SCH_TEXT_SIZE_L;
	}
	else {
		tsize = SCH_TEXT_SIZE_S;
	}
	task_chars  = memAlloc_init(task_chars,  ntasks, (char)0);
	task_bounds = memAlloc_init(task_bounds, ntasks, (char)0);
	str2[0]     = ' ';
	str2[2]     = 0;

	x_screen    = x_blocks = max(runtime, 10) + 3; // 2 extra for y axis labels and last timestep label
	y_screen    = y_blocks = 2*(max(ntasks, (unsigned)1) + 2); // extra for x axis labels and IDLE_TASK
	x_grid      = (double)x_limit/(double)x_blocks;
	y_grid      = (double)y_limit/(double)y_blocks;
	y_start     = (y_blocks - ntasks - 1)/2;
	y_end       = y_start + ntasks + 1;
	object_size = x_limit/x_blocks + 1; // ???

	whiteout();
	whiteout_text_bkg = 0;
	set_text_size(0); // init font
	draw_grid(WHITE);
	SetDrawingColorx(BLACK);
	set_text_size(tsize);
	set_DW_title("Schedule");

	sprintf(str, " Processors: %u, Tasks: %u, Resources: %u, Runtime: %u, U = %lf", nprocs, ntasks, nrsc, runtime, utilization);
	draw_text((string)str,  0,                                     tsize); // show stats
	draw_text(" Task",      -int((y_start - 1)*x_blocks + 1),      tsize); // label y axis
	draw_text(" idle",      -int((y_start + ntasks)*x_blocks + 1), tsize); // label IDLE_TASK
	draw_text(" Timestep", ((y_end + 1)*x_blocks + x_blocks/2),    tsize); // label x axis
	
	for (i = 0; i < ntasks; ++i) { // label tasks
		sprintf(str, " %u", (i+1));
		draw_text((string)str, -int((y_start + i)*x_blocks + 1), tsize);
	}
	for (i = 0; i <= runtime; ++i) {
		el           = timeline[i];
		task_counter = 0;
		sprintf(str, " %u", i);
		draw_text((string)str, -int(y_end*x_blocks + i + 1), tsize); // label timesteps
		
		for (j = 0; j < el.size(); ++j) {
			E       = el[j];
			task_id = E.task_id;

			switch (E.type) {
			case TASK_START:
				tasks[task_id].is_enabled = 1;
				task_bounds[task_id]     += 1;
				break;

			case TASK_END:
				tasks[task_id].is_enabled = 0;
				task_bounds[task_id]     += 2;
				break;

			case TASK_EXEC_INVALID:
				if (task_chars[task_id] != 'X') {
					task_chars[task_id] = '*'; // fail has priority
				}
				++task_counter;
				error = 1;
				break;

			case TASK_EXEC:
				if (task_chars[task_id] != 'X' && task_chars[task_id] != '*') {
					task_chars[task_id] = '1' + (char)(E.proc_id); // fail and invalid exec have priority
				}
				++task_counter;
				error = 0;
				break;

			case TASK_FAIL:
				task_chars[task_id] = 'X';
				error = 1;
				break;

			case TASK_NULL:
				break;

			default:
				cerr << "Illegal event type: " << (int)((unsigned char)E.type) << "." << endl;
				task_chars[task_id] = '?';
			}
		}
		for (j = 0; j < ntasks; ++j) {
			if (tasks[j].is_enabled && i != runtime) { // display enabled task range
				SetDrawingColorx(SCH_HILITE_COLOR);
				RectangleDraw(((i + 2)*x_grid), ((j + y_start)*y_grid), ((i + 3)*x_grid), ((j + y_start + 1)*y_grid));
				SetDrawingColorx(BLACK);
			}
			if (task_chars[j] != 0 && (i != runtime || task_chars[j] == 'X' || task_chars[j] == '*')) { // display character in box
				str2[1] = task_chars[j];
				draw_text((string)str2, -int((y_start + j)*x_blocks + i + 2), tsize);
				task_chars[j] = 0;
			}
			if (task_bounds[j] == 1 || task_bounds[j] == 3) {
				LineDraw(((i + 2)*x_grid + 1), ((j + y_start)*y_grid), ((i + 2)*x_grid + 1), ((j + y_start + 1)*y_grid));
			}
			if (task_bounds[j] == 2 || task_bounds[j] == 3) {
				LineDraw(((i + 2)*x_grid - 1), ((j + y_start)*y_grid), ((i + 2)*x_grid - 1), ((j + y_start + 1)*y_grid));
			}
			task_bounds[j] = 0;
		}
		if (!error && task_counter < nprocs && i != runtime) {
			str2[1] = '0' + (char)(nprocs - task_counter);
			draw_text((string)str2, -int((y_start + ntasks)*x_blocks + i + 2), tsize);
		}
	}
	for (i = 0; i <= ntasks+1; ++i) {
		LineDraw((2*x_grid), ((i + y_start)*y_grid), (x_limit-x_grid), ((i + y_start)*y_grid));
	}
	for (i = 0; i <= runtime; ++i) {
		LineDraw(((i + 2)*x_grid), (y_start*y_grid), ((i + 2)*x_grid), (y_end*y_grid));
	}
	LineDraw((x_limit-x_grid), (y_start*y_grid), (x_limit-x_grid), (y_end*y_grid));

	cout << "Click on the screen to continue." << endl;
	GetClick();

	delete [] task_chars;
	delete [] task_bounds;
	set_DW_title(init_window_title);
 	whiteout();
	object_size       = temp_osize;
	whiteout_text_bkg = wtbkg;
}



// scheduling algorithms
int schedule_rate_monotonic(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks, unsigned nprocs, unsigned nrsc, int is_preemptive) {

	assign_rm_priorities(tasks, ntasks);

	if (is_preemptive) {
		sort_timeline(timeline, runtime);
	}
	return priority_schedule(timeline, tasks, runtime, ntasks, nprocs, nrsc, is_preemptive);
}



// *** FINISH ***
int schedule_edf(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks, unsigned nprocs, unsigned nrsc, int is_preemptive) {

	unsigned  i, j, k, task_id, e_task_id, t_curr(0);
	eventlist el;
	event     E, E_fail;

	E_fail.priority = NULL;

	cout << "*** Not yet supported. ***" << endl;
	return 0;

	//get_next_deadline(timeline, tasks, t_curr, runtime);

	for (i = 0; i <= runtime; ++i) {
		el = timeline[i];
		for (j = 0; j < el.size(); ++j) {
			E       = el[j];
			task_id = E.task_id;

			switch (E.type) {
			case TASK_START:
				tasks[task_id].comp_remaining = tasks[task_id].comp_total; // reset computation counter
				tasks[task_id].is_enabled     = 1;
				break;

			case TASK_END:
				if (tasks[task_id].comp_remaining > 0) { // computation not finished
					cout << "Task " << (task_id+1) << " missed its deadline at time " << i << " with " << tasks[task_id].comp_remaining << " unit(s) of computation remaining." << endl;
					E_fail.type    = TASK_FAIL;
					E_fail.proc_id = 0;
					E_fail.task_id = task_id;
					timeline[i].push_back(E_fail); // actually, should be push_front
					return 0;
				}
				tasks[task_id].is_enabled = 0;
				break;

			case TASK_FAIL: // should have returned already
				return 0;

			case TASK_NULL:
				break;

			default:
				cerr << "Illegal event type: " << (int)((unsigned char)E.type) << "." << endl;
				return 0;
			}
		}
		for (k = 0; k < nprocs; ++k) { // *** FINISH ***
			e_task_id = 0; // ???
			if (tasks[e_task_id].is_enabled == 0) {
				cout << "Error in scheduling algorithm: Task " << (e_task_id+1) << " should not be executing on processor " << (k+1) << " at time " << i << "." << endl;
				E_fail.type    = TASK_EXEC_INVALID;
				E_fail.proc_id = E.proc_id;
				E_fail.task_id = task_id;
				timeline[i][j].type = TASK_NULL;
				timeline[i].push_back(E_fail);
				return 0;
			}
			tasks[task_id].comp_remaining--;
		}
	}
	return 1;
}



// *** FINISH ***
int schedule_audsley(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks, unsigned nprocs, unsigned nrsc, int is_preemptive) {

	unsigned  i, j, k, task_id, e_task_id, t_curr(0);
	eventlist el;
	event     E, E_fail;

	E_fail.priority = NULL;

	cout << "*** Not yet supported. ***" << endl;
	return 0;

	for (i = 0; i <= runtime; ++i) {
		el = timeline[i];
		for (j = 0; j < el.size(); ++j) {
			E       = el[j];
			task_id = E.task_id;

			switch (E.type) {
			case TASK_START:
				tasks[task_id].comp_remaining = tasks[task_id].comp_total; // reset computation counter
				tasks[task_id].is_enabled     = 1;
				break;

			case TASK_END:
				if (tasks[task_id].comp_remaining > 0) { // computation not finished
					cout << "Task " << (task_id+1) << " missed its deadline at time " << i << " with " << tasks[task_id].comp_remaining << " unit(s) of computation remaining." << endl;
					E_fail.type    = TASK_FAIL;
					E_fail.proc_id = 0;
					E_fail.task_id = task_id;
					timeline[i].push_back(E_fail); // actually, should be push_front
					return 0;
				}
				tasks[task_id].is_enabled = 0;
				break;

			case TASK_FAIL: // should have returned already
				return 0;

			case TASK_NULL:
				break;

			default:
				cerr << "Illegal event type: " << (int)((unsigned char)E.type) << "." << endl;
				return 0;
			}
		}
		for (k = 0; k < nprocs; ++k) { // *** FINISH ***
			e_task_id = 0; // ???
			if (tasks[e_task_id].is_enabled == 0) {
				cout << "Error in scheduling algorithm: Task " << (e_task_id+1) << " should not be executing on processor " << (k+1) << " at time " << i << "." << endl;
				E_fail.type    = TASK_EXEC_INVALID;
				E_fail.proc_id = E.proc_id;
				E_fail.task_id = task_id;
				timeline[i][j].type = TASK_NULL;
				timeline[i].push_back(E_fail);
				return 0;
			}
			tasks[task_id].comp_remaining--;
		}
	}
	return 1;
}




int schedule_custom(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks, unsigned nprocs, unsigned nrsc, int is_preemptive) {

	int      error(0);
	unsigned i, stime, etime, task_id, proc_id;
	event    E;
	ifstream infile;

	E.type = TASK_EXEC2;

	if (!open_file(infile)) {
		cout << "Could not retrieve scheduling information." << endl;
		return 0;
	}
	// task processor start_time end_time

	while (infile.good() && (infile >> task_id >> proc_id >> stime >> etime)) {
		if (stime > runtime) {
			cerr << "Error: Start time of " << stime << " is greater than the runtime of " << runtime << "." << endl;
			error = 1;
		}
		if (etime > runtime) {
			cerr << "Error: End time of " << etime << " is greater than the runtime of " << runtime << "." << endl;
			error = 1;
		}
		if (task_id > ntasks) {
			cerr << "Error: Task ID " << task_id << " is greater than the number of tasks, " << ntasks << "." << endl;
			error = 1;
		}
		if (proc_id > nprocs) {
			cerr << "Error: Processor ID " << proc_id << " is greater than the number of processors, " << nprocs << "." << endl;
			error = 1;
		}
		if (proc_id == 0) {
			cerr << "Error: There is no processor 0. Processor IDs start at 1." << endl;
			error = 1;
		}
		if (error) {
			break;
		}
		if (task_id == 0) {
			task_id = IDLE_TASK;
		}
		if (stime > etime) {
			swap(stime, etime);
		}
		E.task_id  = task_id - 1;
		E.proc_id  = proc_id - 1;
		E.priority = &(tasks[task_id].priority); // probably can just set to NULL

		for (i = stime; i < etime; ++i) {
			timeline[i].push_back(E);
		}
	}
	infile.close();

	if (error) {
		return 0;
	}
	sort_timeline(timeline, runtime);
	convert_exec2s(timeline, runtime);

	return verify_schedule(timeline, tasks, runtime, nprocs, nrsc, is_preemptive);
}



// scheduling subfunctions
void assign_rm_priorities(task *tasks, unsigned ntasks) {

	unsigned  i, priority;
	task_p   *task_ps = NULL;

	task_ps = memAlloc(task_ps, ntasks);

	for (i = 0; i < ntasks; ++i) { // create new kind of task with index to original task
		task_ps[i].comp_total = tasks[i].comp_total;
		task_ps[i].deadline   = tasks[i].deadline;
		task_ps[i].period     = tasks[i].period;
		task_ps[i].id         = i;
	}
	qsort((void *)task_ps, ntasks, sizeof(task_p), comp_rm); // sort new tasks, l->s periods = s->l priorities

	for (i = 0, priority = 0; i < ntasks; ++i) {
		if (i > 0 && (task_ps[i].period != task_ps[i-1].period || task_ps[i].deadline != task_ps[i-1].deadline || task_ps[i].comp_total != task_ps[i-1].comp_total)) {
			++priority; // new task parameters - advance priority
		}
		tasks[task_ps[i].id].priority = priority; // extract priorities from new task ordering
	}
	delete [] task_ps;
}




int priority_schedule(eventlist *timeline, task *tasks, unsigned runtime, unsigned ntasks, unsigned nprocs, unsigned nrsc, int is_preemptive) {

	int        was_preempted;
	unsigned   i, j, k, task_id, e_task_id, nblocked, nexec(0);
	unsigned  *resources = NULL, *blocked_tasks = NULL;
	proc_exec *executing = NULL;
	eventlist  el;
	event      E, E_fail, E_exec;
	pq_less    pending_tasks = pq_less(task_lt_priority(tasks));
	//pq_greater execing_tasks = pq_greater(task_gt_priority(tasks));

	E_fail.priority = NULL;
	E_exec.type     = TASK_EXEC;
	executing       = memAlloc(executing, nprocs);
	resources       = memAlloc_init(resources, nrsc+1, (unsigned)0);
	blocked_tasks   = memAlloc(blocked_tasks, ntasks);

	for (k = 0; k < nprocs; ++k) {
		executing[k].proc     = k;
		executing[k].task     = IDLE_TASK;
		executing[k].priority = 0;
	}
	for (i = 0; i <= runtime; ++i) {
		el = timeline[i];

		// process TASK_START and TASK_END events
		for (j = 0; j < el.size(); ++j) {
			E       = el[j];
			task_id = E.task_id;
				
			switch (E.type) {
			case TASK_START:
				tasks[task_id].comp_remaining = tasks[task_id].comp_total; // reset computation counter
				tasks[task_id].is_enabled     = 1;
				was_preempted                 = 0;

				// *** MULTIPROCESSOR *** - priority queue or sort
				if (is_preemptive && nexec > 0 && (nrsc == 0 || resources[tasks[task_id].rsc_lock] == 0) && tasks[task_id].priority > 0) { // task can possibly preempt
					for (k = 0; k < nprocs; ++k) {
						e_task_id = executing[k].task;
						// NOTE: if task is completed and gets preempted by another task, then the new task is gauranteed to be the highest priority
						if (e_task_id != IDLE_TASK && (tasks[task_id].priority > tasks[e_task_id].priority)) {
							executing[k].task     = task_id; // task preempted - processor not idle, resource not locked, higher priority
							executing[k].priority = tasks[task_id].priority;

							if (tasks[e_task_id].comp_remaining > 0) {
								pending_tasks.push(e_task_id); // add e_task_id to pending queue if not finished
							}
							was_preempted = 1;
							break;
						}
					}
				}
				if (!was_preempted) {
					pending_tasks.push(task_id); // add task_id to pending task queue
				}
				break;

			case TASK_END:
				if (tasks[task_id].comp_remaining > 0) { // computation not finished
					cout << "Task " << (task_id+1) << " missed its deadline at time " << i << " with " << tasks[task_id].comp_remaining << " unit(s) of computation remaining." << endl;
					E_fail.type    = TASK_FAIL;
					E_fail.proc_id = 0;
					E_fail.task_id = task_id;
					timeline[i].push_back(E_fail);
					return delete_all_ps(pending_tasks, executing, resources, blocked_tasks, 0);
				}
				tasks[task_id].is_enabled = 0;
				break;

			case TASK_FAIL: // should have returned already
				return delete_all_ps(pending_tasks, executing, resources, blocked_tasks, 0);

			case TASK_NULL:
				break;

			default:
				cerr << "Illegal event type: " << (int)((unsigned char)E.type) << "." << endl;
				return delete_all_ps(pending_tasks, executing, resources, blocked_tasks, 0);
			}
		}
		nexec = 0;

		// assign TASK_EXEC events and schedule new tasks
		for (k = 0; k < nprocs; ++k) {
			e_task_id = executing[k].task;

			if (e_task_id != IDLE_TASK) {
				if (tasks[e_task_id].comp_remaining == 0) { // task has finished
					tasks[e_task_id].is_enabled = 0;
					e_task_id                   = IDLE_TASK;
					executing[k].task           = IDLE_TASK;
					if (nrsc > 0 && tasks[e_task_id].rsc_lock > 0) {
						resources[tasks[e_task_id].rsc_lock] = 0; // unlock resource
					}
				}
			}
			if (e_task_id != IDLE_TASK) {
				if (tasks[e_task_id].is_enabled == 0) {
					cout << "Error in scheduling algorithm: Task " << (e_task_id+1) << " should not be executing on processor " << (k+1) << " at time " << i << "." << endl;
					E_fail.type    = TASK_EXEC_INVALID;
					E_fail.proc_id = k;
					E_fail.task_id = task_id;
					timeline[i][j].type = TASK_NULL;
					timeline[i].push_back(E_fail);
					return delete_all_ps(pending_tasks, executing, resources, blocked_tasks, 0);
				}
			}
			if (e_task_id == IDLE_TASK) {
				nblocked = 0;
					
				while (!pending_tasks.empty()) { // remove highest priority pending task without locked resources from queue
					task_id = pending_tasks.top();
					pending_tasks.pop();

					if (nrsc == 0 || resources[tasks[task_id].rsc_lock] == 0 || resources[tasks[task_id].rsc_lock] != task_id+1) { // resources are not locked by another task
						e_task_id             = task_id;
						executing[k].task     = task_id; // new task
						executing[k].priority = tasks[task_id].priority;
						break;
					}
					assert(nblocked < ntasks);
					blocked_tasks[nblocked++] = task_id;
				}
				for (j = 0; j < nblocked; ++j) {
					pending_tasks.push(blocked_tasks[j]); // add blocked tasks back into pending tasks
				}
			}
			if (e_task_id != IDLE_TASK) {
				tasks[e_task_id].comp_remaining--; // execute the task
				E_exec.task_id  = e_task_id;
				E_exec.proc_id  = k;
				E_exec.priority = &(tasks[e_task_id].priority); // probably can just set to NULL
				timeline[i].push_back(E_exec);
				++nexec;
				if (nrsc > 0 && tasks[e_task_id].rsc_lock > 0) {
					resources[tasks[e_task_id].rsc_lock] = e_task_id+1; // lock resource
				}
			}
		}
	}
	return delete_all_ps(pending_tasks, executing, resources, blocked_tasks, 1);
}




int delete_all_ps(pq_less &pending_tasks, proc_exec *executing, unsigned *resources, unsigned *blocked_tasks, int ret_val) {

	while(!pending_tasks.empty()) {
		pending_tasks.pop();
	}
	delete [] executing;
	delete [] resources;
	delete [] blocked_tasks;

	return ret_val;
}



// *** FINISH ***
unsigned get_next_deadline(eventlist *timeline, task *tasks, unsigned &t_curr, unsigned runtime) {

	int       found_deadline(0), t_curr_set(0);
	unsigned  i, j, task_id, next_deadline, deadline1, deadline2;
	eventlist el;
	tasklist  tl;

	// deadline/task end queue

	for (i = t_curr; i <= runtime; ++i) {
		el = timeline[i];

		for (j = 0; j < el.size(); ++i) {
			if (el[j].type == TASK_END) { // task deadline
				task_id = el[j].task_id;

				if (tasks[task_id].is_enabled) { // task enabled (has started)
					if (tasks[task_id].comp_remaining > 0) { // task is still pending
						t_curr = i; // next enabled task
			
						if (found_deadline) {
							tl.push_back(task_id); // other deadlines
						}
						else {
							found_deadline = 1;
							next_deadline  = task_id; // first one
						}
					}
				}
				else if (!t_curr_set) { // future task deadline - this task's enabled time is contained inside another's
					t_curr     = i;
					t_curr_set = 1;
				}
			}
		}
		if (found_deadline) { // found one pending deadline in this timestep
			return next_deadline;
		}
		if (tl.size() > 0) { // found at least one pending deadline in this timestep - need to choose one
			task_id   = tl[0];
			deadline2 = tasks[task_id].deadline;

			for (j = 1; j < tl.size(); ++j) {
				deadline1 = tasks[tl[j]].deadline;
				if (deadline1 > deadline2) { // to minimize latency, choose task that has been running longest
					task_id = tl[j];
				}
				deadline2 = deadline1;
			}
			vector_delete(tl);
			return task_id; // ???
		}
	}
	return 0;
}




void force_non_preemptive(task *tasks, unsigned ntasks) { // not used yet

	unsigned i;

	for (i = 0; i < ntasks; ++i) {
		tasks[i].rsc_lock = DEF_RSC; // force non-preemption by making all tasks share an imaginary common resource
	}
}



// ordering
bool operator<(event A, event B) {

	if (A.type    < B.type)    return true;
	if (A.type    > B.type)    return false;

	if (A.priority != NULL && B.priority != NULL) {
		if (*(A.priority) > *(B.priority)) return true;
		if (*(A.priority) < *(B.priority)) return false;
	}	
	if (A.task_id < B.task_id) return true;
	if (A.task_id > B.task_id) return false;
	if (A.proc_id < B.proc_id) return true;

	return false;
}


bool operator<(task A, task B) {

	if (A.priority   < B.priority)   return true;
	if (A.priority   > B.priority)   return false;
	if (A.period     < B.period)     return true;
	if (A.period     > B.period)     return false;
	if (A.comp_total < B.comp_total) return true;
	if (A.comp_total > B.comp_total) return false;
	if (A.deadline   < B.deadline)   return true;
	if (A.deadline   > B.deadline)   return false;
	if (A.rsc_lock   < B.rsc_lock)   return true;

	return false;
}


bool operator<(proc_exec A, proc_exec B) { // for priority scheduling

	return (A.priority < B.priority);
}


int comp_rm(const void *A, const void *B) { // for rate monotonic scheduling

	task_p *a = (task_p *)A, *b = (task_p *)B;

	if (a->period     > b->period)     return -1;
	if (a->period     < b->period)     return 1;
	if (a->deadline   > b->deadline)   return -1;
	if (a->deadline   < b->deadline)   return 1;
	if (a->comp_total > b->comp_total) return -1;
	if (a->comp_total < b->comp_total) return 1;

	return 0;
}




