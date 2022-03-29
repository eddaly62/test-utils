// pattern3q.c
// proof of concept multi-threaded regular expression search
//
// This program creates several threads to search a table of regular expressions
// to find a callback function to execute for the matching regular expression.
//
// The input string is typed in via the keyboard and the number of threads created
// to search the table of regular expressions is an input arguement to the program.
// The range is 2 to 7.
//
// The results are written to the display along with the measured search time.
// If a match is found it will run the callback function.
//
// This implemenation has a function that creates the threads for the search and once the 
// search completes the threads are released. 
//
// The function uses one barriers to provide synchronization between the threads.
// The data exchange between the threads is accomplished with arrays, every thread gets
// its own storage location in the arrays, avoiding the use of mutexes which
// would slow the processing down. Once the search is completed the threads end and 
// the results are collected with no conflicts.
//
// This version changes the return and input ares of the dap_pattern_find function and
// introduces a queue functionality

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <sys/time.h>
#include <regex.h>
#include <stdint.h>
#include <stdbool.h>


#define MAX_PATTERN_BUF_SIZE 100
#define MATCH 0
#define EXIT_STRING "q"     // string to type to exit program
#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

// input queue size
#define MAX_IN_Q_SIZE 6

// app supplied callback prototypes
void callback(char *s);

// definitions

enum NUMTHR {
    MAXTHRIDX = 6,
    RESULTIDX = 7,
    MAXNUMTHR = 8,
};

enum ELTIME {                        // for elapsed time
    START = 0,
    END,
};   

// dap pattern/callback definitions (goes in header file)
typedef void(*cb_func_c)(char*);
typedef void(*cb_func_i)(int);

// Pattern/Callback Look Up Table
struct DAP_PATTERN_CB{
    char pattern[MAX_PATTERN_BUF_SIZE];
    cb_func_c cb;
};

struct DAP_REGEX_RESULTS {
    char out[MAX_PATTERN_BUF_SIZE]; // string matched
    cb_func_c cb;                   // callback function pulled from lut
    regmatch_t  pmatch[1];          // start and end offset info were in string match was found
    int indexlut;                   // index in pattern lut match was found
    long idx;                       // thread index
    pthread_t tid;                  // thread id
    regoff_t len;                   // length of matched string
};


// vars

// input queue declarations
struct DAP_REGEX_RESULTS in_q[MAX_IN_Q_SIZE];
int in_q_front = 0;
int in_q_rear = -1;
int in_q_count = 0;

struct timeval start, end;          // elapsed time

int nthr;                           // number of threads 
pthread_t tid;                      // thread id
pthread_barrier_t b1;               // barrier used to synchronize search threads
char in[MAX_PATTERN_BUF_SIZE];      // input buffer
struct DAP_REGEX_RESULTS reresults[MAXNUMTHR];
struct DAP_PATTERN_CB *re_cb_lut_ptr;
int relutsize;

// app supplied pattern/callback look up table 
const struct DAP_PATTERN_CB relut[] = {
    {"033A", &callback},
    {"033B", &callback},
    {"033C", &callback},
    {"033D", &callback},
    {"033E", &callback},    
    {"033F", &callback},    
    {"033G", &callback},    
    {"033H", &callback},    
    {"033I", &callback},    
    {"033J", &callback},    
    {"033K", &callback},    
    {"033L", &callback},    
    {"033M", &callback},    
    {"033N", &callback},    
    {"033O", &callback},    
    {"033P", &callback},    
    {"033Q", &callback},    
    {"033R", &callback},    
    {"033S", &callback},    
    {"033T", &callback},    
    {"033U", &callback},    
    {"033V", &callback},    
    {"033W", &callback},    
    {"033X", &callback},    
    {"033Y", &callback},    
    {"033Z", &callback},    
};

// sample callback function supplied by the app
void callback(char *s){
    fprintf(stdout, "callback function called, pattern = %s\n", s);
    return;
}

// queue functions
void peek_in_q(struct DAP_REGEX_RESULTS *data) {
    memcpy(data, &in_q[in_q_front], sizeof(in_q[in_q_front]));
}

bool is_empty_in_q() {
   return in_q_count == 0;
}

bool is_full_in_q() {
   return in_q_count == MAX_IN_Q_SIZE;
}

int size_in_q() {
   return in_q_count;
}  

void insert_data_in_q(struct DAP_REGEX_RESULTS *data) {

   if(!is_full_in_q()) {
	
      if(in_q_rear == MAX_IN_Q_SIZE-1) {
         in_q_rear = -1;            
      }       

      in_q_rear++;
      memcpy(&in_q[in_q_rear], data, sizeof(in_q[in_q_rear]));
      in_q_count++;
   }
}

void remove_data_in_q(struct DAP_REGEX_RESULTS *data) {
    
    memcpy(data, &in_q[in_q_front], sizeof(in_q[in_q_front]));
    in_q_front++;

    if(in_q_front == MAX_IN_Q_SIZE) {
        in_q_front = 0;
    }
	
    in_q_count--;
}

// elapse time function
long long elapsed_time(enum ELTIME sts, struct timeval *start, struct timeval *end)
{
	long long startusec = 0, endusec = 0;
	long long elapsed = 0;

	if (sts == START)
	{
		gettimeofday(start, NULL);
	}
	else
	{
		gettimeofday(end, NULL);
		startusec = start->tv_sec * 1000000 + start->tv_usec;
		endusec = end->tv_sec * 1000000 + end->tv_usec;
		elapsed = endusec - startusec;				// usec
	}
	return elapsed;
}

// clear results array
static void clearresults(void) {
    int i;
  
    for (i = 0; i < MAXNUMTHR; i++) {
        memset (&reresults[i], 0, sizeof(reresults[i]));
    }
}

// get results
static void getresults(void) {
    long i;
    
    // check results
    for (i = 0; i < nthr; i++) {

        // check and see if any of the threads found a match
        if (reresults[i].cb != NULL) {
            reresults[RESULTIDX].cb = reresults[i].cb;
            reresults[RESULTIDX].indexlut = reresults[i].indexlut;
            reresults[RESULTIDX].pmatch[0].rm_so = reresults[i].pmatch[0].rm_so;
            reresults[RESULTIDX].pmatch[0].rm_eo = reresults[i].pmatch[0].rm_eo;
            reresults[RESULTIDX].tid = reresults[i].tid;
            reresults[RESULTIDX].idx = reresults[i].idx;
            reresults[RESULTIDX].len = reresults[i].len;
            memcpy(reresults[RESULTIDX].out, reresults[i].out, sizeof(reresults[RESULTIDX].out));
        }
    }
}

// Pattern matching threads.
static void *thr_fn(void *arg)
{
    regex_t     regex;
    regmatch_t  pmatch[1];
    regoff_t    off, len;
    long idx;
    int i;

    idx = (long)arg;

    for (i=0; i < relutsize; i++) {
        
        // only check every nthr entry in LUT
        if (idx == (i+1)%nthr) {

            if (regcomp(&regex, re_cb_lut_ptr[i].pattern, REG_NEWLINE))
                exit(EXIT_FAILURE);

            if (regexec(&regex, in, ARRAY_SIZE(pmatch), pmatch, 0) == MATCH) {
                off = pmatch[0].rm_so;
                len = pmatch[0].rm_eo - pmatch[0].rm_so;
                reresults[idx].indexlut = i;
                reresults[idx].idx = idx;
                reresults[idx].tid = reresults[i].tid;
                reresults[idx].pmatch[0].rm_so = pmatch[0].rm_so;
                reresults[idx].pmatch[0].rm_eo = pmatch[0].rm_eo;
                reresults[idx].cb = re_cb_lut_ptr[i].cb;
                reresults[idx].len = len;
                reresults[idx].tid = pthread_self();
                memcpy(&reresults[idx].out[0], &in[off], len);
            }

            // cleanup lex's
            regfree(&regex);
        }
    }

    // wait until all threads have finished
    pthread_barrier_wait(&b1);

    return((void *)0); // exit
}

#define SUCCESS 0
#define PATTERN_FIND_ERROR (-1)
int dap_pattern_find(char *s, const struct DAP_PATTERN_CB *ptnlut, int len, struct DAP_REGEX_RESULTS *rt) {

    long i;
    int err;
    int result = SUCCESS;

    if ((s[0] == 0) || (ptnlut == NULL) || (len == 0)) {
        return PATTERN_FIND_ERROR;
    }

    // store input args to private vars so threads can access
    re_cb_lut_ptr = (struct DAP_PATTERN_CB*)ptnlut;
    relutsize = len;

    // clear result structures
    clearresults();

    // get data packet to process
    strncpy(in, s, sizeof(in));    

    // create thread barrier
    pthread_barrier_init(&b1, NULL, nthr+1);
    
    // create nthr number of threads to search table
    for (i = 0; i < nthr; i++) {
        err = pthread_create(&tid, NULL, thr_fn, (void *)i);
        if (err != 0) {
            exit(EXIT_FAILURE);
        }
    }

    // wait until all threads have finished
    pthread_barrier_wait(&b1);

    // all threads have finished, get the results
    getresults();

    // copy result
    memcpy(rt, &reresults[RESULTIDX], sizeof(reresults[RESULTIDX]));

    // deinitialize barrier
    pthread_barrier_destroy(&b1);

    return result;
}


int main(int argc, char *argv[]) {

    long i;
    int err, n;
	long long elapsedt=0;
    char s[MAX_PATTERN_BUF_SIZE];
    struct DAP_REGEX_RESULTS rt;

    if (argc != 2) {
		printf("Usage:\n./pattern2 [2-7]\n\nWhere:\n\t2-7 is the number of threads\n");
		exit(EXIT_FAILURE);
	}
 
    n = atoi(argv[1]);
    if (n >= 2 && n <= 7){
        nthr = n;
    }
    else {
        printf("The number of threads can only be 2 to 7\n\n");
   		printf("Usage:\n./pattern2 [2-7]\n\nWhere:\n\t2-7 is the number of threads\n");
        exit(EXIT_FAILURE);
    }

    for (;;){
        fprintf(stdout,"Enter data packet to process: ");
        if (fscanf(stdin, "%s", s) > 0) {

            if (strcmp(EXIT_STRING, s) == 0) {
                // quit
                exit(0);
            }
            else {

                // start timer
                elapsed_time(START, &start, &end);

                dap_pattern_find(s, &relut[0], ARRAY_SIZE(relut), &rt);

                elapsedt = elapsed_time(END, &start, &end);
                fprintf(stdout, "Search time is %lld usecs with %d threads\n", elapsedt, nthr);
    
                fprintf(stdout,"index in lut = %d,\t string = %s,\t found by thread = %ld, tid = %lu\n",
                rt.indexlut, rt.out, rt.idx, (unsigned long)rt.tid);

                // run callback
                if (rt.cb != NULL) {
                    (*(rt.cb))(rt.out);
                }

                // store result in queue
                insert_data_in_q(&rt);

                // show contents of queue
                if (in_q_front == -1) {
                    printf("Empty Queue \n");
                }
                else
                {
                    printf("Queue: \n");
                    for (n = in_q_front; n <= in_q_rear; n++)
                        printf("%s ", in_q[n].out);
                    printf("\n");
                }
            }

        }
    }
    

    


}
