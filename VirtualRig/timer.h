#ifndef _IDO_TIMER_H
#define _IDO_TIMER_H

#ifdef WIN32
/*! \brief: Timer Class interface for capture frame rate
*/
#include <windows.h>
#include <iostream>

inline double GetRealTime()
{
	static double SCALE_COUNTER=-1;

	if (SCALE_COUNTER==-1.0) {
		LARGE_INTEGER pc;

		if(!QueryPerformanceFrequency((LARGE_INTEGER *)&pc))
		{
			printf("No frequency !!!\n");
		}

		SCALE_COUNTER=1.0/((double)pc.QuadPart);

	}
	//unsigned int perf_cnt = pc.QuadPart;

	//double scale_time = 1.0 / perf_cnt;
	LARGE_INTEGER ct;
	double count;
	if(!QueryPerformanceCounter((LARGE_INTEGER *) &ct))
	{
		printf("No timer !!!");
		return 1;
	}
	count=(double)ct.QuadPart;	// how many ticks till now?
	count*=SCALE_COUNTER;		// tick * nb of ticks per sec = nb of secs
	return count;
}

inline double GetRealTimeInMS() 
{
	double t=GetRealTime();
	double t2=t*1000.0;
	return t2;
}

#else 

#if defined(__i386__) || defined(__ia64__)

/** Time system from xxx*/
#include <sys/time.h>
#include <inttypes.h>

inline uint64_t TimerGetCycle() {
	uint64_t x;
	__asm__ volatile ( "RDTSC" : "=A" (x) ) ;
	return x;
}

inline uint64_t TimerGetHz()
{
	static struct timeval t1, t2;
	static struct timezone tz;
	uint64_t c1,c2;

	gettimeofday(&t1, &tz);
	c1 = TimerGetCycle();
	for (int i = 0; i < 2500000; i++);
	gettimeofday(&t2, &tz);
	c2 = TimerGetCycle();
	return (1000000 * (c2 - c1)) / ((t2.tv_usec - t1.tv_usec) + 1000000 * (t2.tv_sec - t1.tv_sec));
}


double GetRealTime() 
{
	static double SCALE_COUNTER=-1;
	if (SCALE_COUNTER==-1.0)
		SCALE_COUNTER=1.0/(double)TimerGetHz() ;

	return SCALE_COUNTER*(double)TimerGetCycle() ;
}

double GetRealTimeInMS() 
{
	double t=GetRealTime();
	double t2=t*1000.0;
	//   printf ("time %f - %f\n",t,t2);
	return t2;
}

#else 

#include <SDL.h>

double GetRealTime()
{
	const double f=1.0/1000.0;
	return SDL_GetTicks()*f;
}

double GetRealTimeInMS()
{
	return SDL_GetTicks();
}

#endif	// #if defined(__i386__) || defined(__ia64__)

#endif	// #ifdef WIN32

#endif	// #ifndef _IDO_TIMER_H
