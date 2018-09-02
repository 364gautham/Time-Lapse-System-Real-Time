/*
 *
 *  Example by Sam Siewert 
 *
 *  Updated 10/29/16 for OpenCV 3.1
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/time.h>
#include <errno.h>
#include <string>
#include <cmath>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

using namespace cv;
using namespace std;
#define TRUE (1)
#define FALSE (0)
#define USEC_PER_MSEC (1000)
#define NANOSEC_PER_SEC (1000000000)
#define NUM_THREADS 3

#define HRES 640
#define VRES 480
char timg_window_name[] = "Image Window";
pthread_t threads[NUM_THREADS];
pthread_mutex_t lock;
int abortTest=FALSE;
int abortS1=FALSE,abortS2=FALSE,abortS3=FALSE;
sem_t semS1,semS2,semS3,semS33,semS22;
struct timeval start_time_val;

//#define COMPRESSION


Mat src[20];

struct timespec end_time, prev_time;



void *frame_capture(void* a)
{
    struct timeval current_time_val,current_time_val1;
    double current_time;
    unsigned long long S1Cnt=0;
    double ex_time,jitter,deadline=100;;
    volatile double jitter_avg=0;
    gettimeofday(&current_time_val, (struct timezone *)0);
    //syslog(LOG_CRIT, "Frame Sampler thread @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
    //printf("Frame Sampler thread @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
    
    VideoCapture videoCapture(0);
	// check if we succeeded
    if (!videoCapture.isOpened()) 
        cerr << "ERROR! Unable to open camera\n";

    videoCapture.set(CAP_PROP_FRAME_WIDTH,HRES);
    videoCapture.set(CAP_PROP_FRAME_HEIGHT,VRES);
    volatile int i=0;
    prev_time.tv_sec=0;prev_time.tv_nsec=0;
    long int j=0;

    while(!abortS1)
    {
	sem_wait(&semS1);
        S1Cnt++;
        gettimeofday(&current_time_val, (struct timezone *)0);
        //syslog(LOG_CRIT, "capture release %llu @ sec=%d, msec=%d\n", S1Cnt, (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
      
        //get a frame
        videoCapture >> src[i];
	if (!videoCapture.read(src[i++])) {
           cerr << "ERROR! blank frame grabbed\n";
          break;
        }
	
	syslog(LOG_CRIT, "ivalue %d",(int)i);
	gettimeofday(&current_time_val1, (struct timezone *)0);
	//syslog(LOG_CRIT, "capture end %llu @ sec=%d, msec=%d\n", S1Cnt, (int)(current_time_val1.tv_sec-start_time_val.tv_sec),(int)current_time_val1.tv_usec/USEC_PER_MSEC);

	if(i==7 || i==17){
		j++;
		if(j>2)sem_post(&semS22);
	}
	if(i==20) i=0;

	clock_gettime( CLOCK_REALTIME, &end_time);
        prev_time.tv_sec = end_time.tv_sec - prev_time.tv_sec;
        prev_time.tv_nsec = end_time.tv_nsec - prev_time.tv_nsec;
        ex_time = prev_time.tv_sec *1000 + (prev_time.tv_nsec/1000000);
        prev_time.tv_nsec=end_time.tv_nsec;prev_time.tv_sec=end_time.tv_sec;
	if(S1Cnt>2){
		if(ex_time<deadline)
			jitter = deadline - ex_time;
		else
			jitter = ex_time -deadline;	
	   jitter_avg=jitter+(jitter_avg*(S1Cnt-3));
	   jitter_avg = jitter_avg/(S1Cnt-2);
	   //syslog(LOG_CRIT,"Jitter:%d **Jitter_Average:%d",(int)jitter,(int)jitter_avg);	
	}


    }
	pthread_exit((void *)0);
}



void *frame_save_1hz(void* a){
        int cnt=0;
	int sel=0;
	struct timeval current_time_val,current_time_val1;
	unsigned long long S1Cnt=0;
	vector<int> compression_params;
    	compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    	compression_params.push_back(30);
        string filename ="/home/pi/Documents/project/nature/";
	string filename1 ="/home/pi/Documents/project/nature/";
	long int tstamp;
	while(!abortS2)
        {
		sem_wait(&semS2);
		
		
        	string stri = to_string(cnt++);
		filename = filename + stri +".ppm";
		filename1 =  filename1 + stri +".jpg";
	 gettimeofday(&current_time_val, (struct timezone *)0);
		tstamp=current_time_val.tv_sec;
		stri = to_string(tstamp);
		stri = stri + "-Linux-rpi 4.14 SMP armv71 GNU/Linux";
		
		S1Cnt++;
	
		sem_wait(&semS22);
	gettimeofday(&current_time_val, (struct timezone *)0);
        syslog(LOG_CRIT, "writer release %llu @ sec=%d, msec=%d\n", S1Cnt, (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
      	
		sel++;
		if(sel==1){
		  #ifdef COMPRESSION	
		  putText(src[3], stri, cvPoint(50,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,0), 1, CV_AA);
		  imwrite(filename1,src[3],compression_params); 
		 #else
		  putText(src[3], stri, cvPoint(50,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,0), 1, CV_AA);
		  imwrite(filename,src[3]); 
		 #endif
		
	gettimeofday(&current_time_val1, (struct timezone *)0);
	syslog(LOG_CRIT, "writer end %llu @ sec=%d, msec=%d\n", S1Cnt, (int)(current_time_val1.tv_sec-start_time_val.tv_sec),(int)current_time_val1.tv_usec/USEC_PER_MSEC);
		}
		else if(sel==2){
		  #ifdef COMPRESSION
		  putText(src[13], stri, cvPoint(50,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,0), 1, CV_AA);
		  imwrite(filename1,src[13],compression_params);
	          #else
		  putText(src[13], stri, cvPoint(50,450),FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0,0,0), 1, CV_AA);
		  imwrite(filename,src[13]);
		  #endif
		   sel=0;
		}
		
		filename="/home/pi/Documents/project/nature/";
		filename1="/home/pi/Documents/project/nature/";

		
	}
	pthread_exit((void *)0);
}


void *thread1(void* a)
{  
    struct timeval current_time_val,current_time_val1;
    struct timespec delay_time = {0,50000000}; // delay for 33.33 msec, 30 Hz
    struct timespec remaining_time;
    double current_time;
    double residual;
    int rc, delay_cnt=0;
    unsigned long long seqCnt=0; 
    long int value,cnt=0,value1=0,value2=0;
    long int val1=0,val2=0,val3=0;
    
	
    gettimeofday(&current_time_val, (struct timezone *)0);
    //syslog(LOG_CRIT, "Sequencer thread @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
    //printf("Sequencer thread @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);

   unsigned long long last;

	last=40040;

	sem_post(&semS1);

    do
    {
        delay_cnt=0; residual=0.0;

        gettimeofday(&current_time_val, (struct timezone *)0);
        //syslog(LOG_CRIT, "Sequencer thread prior to delay @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
        do
        {
            rc=nanosleep(&delay_time, &remaining_time);

            if(rc == EINTR)
            { 
                residual = remaining_time.tv_sec + ((double)remaining_time.tv_nsec / (double)NANOSEC_PER_SEC);

                if(residual > 0.0) printf("residual=%lf, sec=%d, nsec=%d\n", residual, (int)remaining_time.tv_sec, (int)remaining_time.tv_nsec);
 
                delay_cnt++;
            }
            else if(rc < 0)
            {
                perror("Sequencer nanosleep");
                exit(-1);
            }
           
        } while((residual > 0.0) && (delay_cnt < 100));

	seqCnt++;delay_time = {0,50000000};
        gettimeofday(&current_time_val1, (struct timezone *)0);
        //syslog(LOG_CRIT, "Sequencer cycle %llu @ sec=%d, msec=%d\n", seqCnt, (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);
	//syslog(LOG_CRIT, "Sequencer cycle %llu @ sec=%d, msec=%d\n", seqCnt, (int)(current_time_val1.tv_sec-current_time_val.tv_sec),(int)current_time_val1.tv_usec/USEC_PER_MSEC);

	if(((seqCnt % 20)-19) == 0){
		if(cnt>0){
			val1=value;
			val2=(int)current_time_val1.tv_usec/USEC_PER_MSEC;
			val3=(int)abs(val2-val1);
			//syslog(LOG_CRIT,"value = %d",val3);
			if(val3<50 && val3>42){
			val3 =val3*1000000;
			delay_time = {0,val3};
			}
		}
	}

	if((seqCnt % 20) == 0){
		cnt++;
		if(cnt>0){
			value1 = value;	
			value = (int)current_time_val1.tv_usec/USEC_PER_MSEC;
			value2 = (int)abs(value1-value);
			if(value2>0){
			value2 =50-value2;
			//syslog(LOG_CRIT,"value = %d",value2);
			if((value2 <40) || (value2>55)) value2=50;
			value2 =value2*1000000;delay_time = {0,value2};
					
		   }
		}
		else{value = (int)current_time_val1.tv_usec/USEC_PER_MSEC;
		}
	}

	//syslog(LOG_CRIT,"value = %d",value);

        if(delay_cnt > 1) printf("Sequencer looping delay %d\n", delay_cnt);


        // Release each service at a sub-rate of the generic sequencer rate
	
        
        // Service_1	@ 10 Hz
        if((seqCnt % 2) == 0) sem_post(&semS1);


        // Service_2   @ 1 Hz
        if(((seqCnt % 20)-14) == 0) sem_post(&semS2);
      

        //gettimeofday(&current_time_val, (struct timezone *)0);
        //syslog(LOG_CRIT, "Sequencer release all sub-services @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/USEC_PER_MSEC);

    } while(!abortTest && (seqCnt < last));

    sem_post(&semS1);sem_post(&semS2);
    abortS1=TRUE; abortS2=TRUE;

    pthread_exit((void *)0);
     	
}


int main( int argc, char** argv )
{
    //semaphore initilization	
    if (sem_init (&semS1, 0, 0)) { printf ("Failed to initialize S1 semaphore\n"); exit (-1); } 
    if (sem_init (&semS2, 0, 0)) { printf ("Failed to initialize S2 semaphore\n"); exit (-1); } 
    if (sem_init (&semS22, 0, 0)) { printf ("Failed to initialize S22 semaphore\n"); exit (-1); } 


   if (pthread_mutex_init(&lock, NULL) != 0)
    {
        return 1;
    }
    // Setting priority and creating pthread attributes  
    pthread_attr_t rt_sched_attr;
    struct sched_param rt_param[NUM_THREADS];
    int rt_max_prio = sched_get_priority_max(SCHED_FIFO);
    pthread_attr_init(&rt_sched_attr);
    pthread_attr_setinheritsched(&rt_sched_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&rt_sched_attr, SCHED_FIFO);

    cpu_set_t threadcpu;
    CPU_ZERO(&threadcpu);
    CPU_SET(2, &threadcpu);
    pthread_attr_setaffinity_np(&rt_sched_attr,sizeof(cpu_set_t), &threadcpu);

    // starting demo
    gettimeofday(&start_time_val, (struct timezone *)0);

    //high priority sequencer -30hz
    rt_param[0].sched_priority=rt_max_prio;
    //frame capture - 10hz
    rt_param[1].sched_priority=rt_max_prio-1;
	//framesave - 1hz
    rt_param[2].sched_priority=rt_max_prio-3;


    pthread_attr_setschedparam(&rt_sched_attr, &rt_param[0]);
    int i=0;
    pthread_create(&threads[i], &rt_sched_attr,thread1,NULL);     
    
    i++;
    // running services on core 2
    CPU_ZERO(&threadcpu);
    CPU_SET(3, &threadcpu);
    pthread_attr_setaffinity_np(&rt_sched_attr,sizeof(cpu_set_t), &threadcpu);
    pthread_attr_setschedparam(&rt_sched_attr, &rt_param[1]);
    pthread_create(&threads[i], &rt_sched_attr,frame_capture,NULL);
       
	
    i++;
    pthread_attr_setaffinity_np(&rt_sched_attr,sizeof(cpu_set_t), &threadcpu);
    pthread_attr_setschedparam(&rt_sched_attr, &rt_param[2]);
    pthread_create(&threads[i], &rt_sched_attr,frame_save_1hz,NULL);    

    for(i=0; i<NUM_THREADS; i++)
         pthread_join(threads[i], NULL);
    printf("TEST COMPLETE\n");
    return 0;   
};