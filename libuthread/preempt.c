#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include <string.h>

//get ideas from these two link
//https://man7.org/linux/man-pages/man2/sigprocmask.2.html
//https://stackoverflow.com/questions/41345375/about-sigprocmask-sig-block-and-sig-setmask
/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

struct itimerval tmer; //set for alarm for time
struct sigaction Act;
sigset_t SigForPr; //could be struct or int
static int Convert = 1000000;
//https://www.gnu.org/software/libc/manual/html_node/Setting-an-Alarm.html

void preempt_disable(void)
{
	/* TODO Phase 4 */
	sigprocmask(SIG_BLOCK, &SigForPr, NULL); //unchange to remain the value unchange
	//24.7.3 Process Signal Mask 24.7.6
	//https://www.gnu.org/software/libc/manual/html_mono/libc.html#Blocking-for-Handler
}

void preempt_enable(void)
{
	/* TODO Phase 4 */
	sigprocmask(SIG_UNBLOCK, &SigForPr, NULL);
}

static void AlarmHandle (){
	uthread_yield();
}

void preempt_start(void)
{	
	preempt_stop(); // call stop before start
	
	//setup time period 100HZ 
	//from 21.6 setting an Alarm
	//https://www.gnu.org/software/libc/manual/html_mono/libc.html#Blocking-for-Handler
 	tmer.it_interval.tv_sec = 0;
	tmer.it_value.tv_sec = 0;
	tmer.it_value.tv_sec = Convert / HZ;
	tmer.it_interval.tv_sec = Convert / HZ;

	setitimer(ITIMER_VIRTUAL, &tmer, NULL);

}

void preempt_stop(void)
{	
	//reset the struct for all zero
	//24.3.4 24.7.2
	memset(&Act, 0, sizeof(SigForPr));
	sigemptyset(&SigForPr);
	sigaddset(&SigForPr,SIGVTALRM);

	Act.sa_handler = AlarmHandle;
	if(sigaction(SIGVTALRM, &Act, NULL)) exit(1);

}

