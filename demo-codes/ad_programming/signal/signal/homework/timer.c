#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "timer.h"

#define JOBSNUM 256

static JOB_T jobs[JOBSNUM];
static int g_index = 0;

#define DEBUG printf("file:%s\nfunc:%s\nLine:%d\n",\
			__FILE__, __func__, __LINE__)

static int sort_job_by_sec(const void * a, const void * b)
{
	JOB_T * x = (JOB_T *)a;
	JOB_T * y = (JOB_T *)b;

	if (x->nsec > y->nsec)
		return -1;
	else if (x->nsec < y->nsec)
		return 1;
	else
		return 0;
}


static void sort_jobs()
{
	if (g_index == 1)
		return;
	qsort(jobs, g_index, sizeof(JOB_T), sort_job_by_sec);
	return;
}

static void reset_jobs(void)
{
	int i, dec = alarm(0);
	int index = g_index - 1;

//	if (!dec) {
//		return;
//	}	

	dec = jobs[index].nsec - dec;
	
	for (i = 0; i < g_index; i++) {
		jobs[i].nsec -= dec;
	}
	return;
}

static int append_jobs(int taskid, jobFunc job, int expire)
{
	if (g_index == JOBSNUM - 1)
		return -1;
	JOB_T tjob = {
		expire, job, taskid, WAITTING
	};	

	memcpy(jobs+g_index, &tjob, sizeof(tjob));
	g_index++;

	return 0;

}

static void begin_jobs(void)
{
	int min = g_index-1;
	int second;
	if (second = jobs[min].nsec)
		alarm(second);
	return;		
}

int add_job(int taskid, jobFunc job, int expire)
{
	int ret;

	reset_jobs(); //stop current timer
	ret = append_jobs(taskid, job, expire);
	if (ret == -1) {
		DEBUG;
		return -1;
	}
	sort_jobs();

	begin_jobs(); //restart timer
}

static void actual_do_job()
{
	int max = g_index-1;
	int i;
	pid_t pid;
	for (i = max; i >= 0; i--) {
		if (jobs[i].nsec == 0) {
			pid = fork();
			if (pid == 0) {
				(jobs[i].job)(jobs[i].taskId);
				exit(0);
			}
			jobs[i].flag = RUNNING;
			g_index--;
		}	
	}
	return;
}

static void do_jobs(void)
{
	reset_jobs();
	actual_do_job();
	begin_jobs();
}

void alarm_handler(int s)
{
	do_jobs();
}

void test_job(int s)
{
	printf("task %d completed\n", s);
}

//test code
void wait_do_job(void)
{
	int ret, sec;
	char buf[512];
	int i = 0;

	signal(SIGALRM, alarm_handler);
	
	while (1) {
		write(1, "Please add job:", 15);
		ret = read(0, buf, 512);
		buf[ret-1] = '\0';
		sec = atoi(buf);
		ret = add_job(i++, test_job, sec);
		if (ret == -1)
			break;
	}

	exit(1);
}


int main(void)
{
	wait_do_job();
	exit(1);
}
