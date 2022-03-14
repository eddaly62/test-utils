// pattern.c
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
// This implemenation creates the threads once and keeps them alive for the duration of
// the program. It uses barriers to provide synchronization between the threads.
// The data exchange between the threads is accomplished with arrays, every thread gets
// its own storage location in the arrays, avoiding the use of mutexes which
// would slow the processing down.
// Once the search is completed the search threads are waited and the main thread
// goes and processes the results with no conflicts.
//
// Plan to try variation of this that may increase processing speed.

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


#define MAXNUMTHR 7  /* max number of threads */
#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define MAX_PATTERN_BUF_SIZE 100
#define MATCH 0
#define EXIT_STRING "q"     // string to type to exit program

// app supplied callback prototypes
void callback(char *s);

// definitions
enum DAP_PATTERN_CONTROL {
    DISABLED = 0,
    ENABLED,
};

enum ELTIME {                        // for elapsed time
    START = 0,
    END,
};   

// dap pattern/callback definitions (goes in header file)
typedef void(*cb_func_c)(char*);
typedef void(*cb_func_i)(int);

// Pattern/Callback Look Up Table
struct DAP_PATTERN_LUT{
    char *pattern;
    cb_func_c cb;
};

struct DAP_REGEX_RESULTS {
    char out[MAX_PATTERN_BUF_SIZE]; // string matched
    cb_func_c cb;                   // callback function pulled from lut
    regmatch_t  pmatch[1];          // start and end offset info were in string match was found
    int indexlut;                   // index in pattern lut match was found
    pthread_t tid;                  // thread id
};


// vars
enum  DAP_PATTERN_CONTROL dap_pattern_control;
int nthr;                           // number of threads 
char in[MAX_PATTERN_BUF_SIZE];      // input buffer

struct DAP_REGEX_RESULTS reresults[MAXNUMTHR];

struct timeval start, end;          // elapsed time

pthread_t tid[MAXNUMTHR];            // thread id
pthread_barrier_t b1;                // barrier used to synchronize search threads
pthread_barrier_t b2;                // barrier used to synchronize search threads



// app supplied pattern/callback look up table 
const struct DAP_PATTERN_LUT relut[] = {
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

const int relutsize = ARRAY_SIZE(relut);

void callback(char *s){
    fprintf(stdout, "callback function called, pattern = %s\n", s);
    return;
}

// elapse time
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

// clear results
static void clearresults(void) {
    int i;
  
    for (i = 0; i < MAXNUMTHR; i++) {
        memset (&reresults[i], 0, sizeof(reresults[i]));
    }
}

// get results
// note: only called from one thread
void getresults(void) {
    long i;
    cb_func_c cb;
    char s[MAX_PATTERN_BUF_SIZE];
    
    // check results
    for (i = 0, cb = NULL; i < nthr; i++) {
        fprintf(stdout,"index in lut = %d,\t string = %s,\t found by thread = %ld, tid = %lu\n",
        reresults[i].indexlut, reresults[i].out, i, (unsigned long)reresults[i].tid);

        if (reresults[i].cb != NULL) {
            cb = reresults[i].cb;
            memcpy(s, reresults[i].out, sizeof(s));
        }
    }

    // run callback
    if (cb != NULL) {
        (*cb)(s);
    }
}

// shut down threads
void killthreads(void) {
    
    int err;
    int i;
    void *tret;

    for (i = 0; i < nthr; i++) {
        err = pthread_join(tid[i], &tret);
        if (err != 0) {
            printf("can’t join with thread %d\n", i);
            exit(EXIT_FAILURE);
        }
        printf("thread %d exit code %ld\n", i, (long)tret);
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
    
    while (dap_pattern_control == ENABLED) {

        pthread_barrier_wait(&b1);

        for (i=0; i < relutsize; i++) {
            
            if (idx == (i+1)%nthr) {
                // only check every nthr entry in LUT
                if (regcomp(&regex, relut[i].pattern, REG_NEWLINE))
                    exit(EXIT_FAILURE);

                if (regexec(&regex, in, ARRAY_SIZE(pmatch), pmatch, 0) == MATCH) {
                    off = pmatch[0].rm_so;
                    len = pmatch[0].rm_eo - pmatch[0].rm_so;
                    reresults[idx].indexlut = i;
                    reresults[idx].pmatch[0].rm_so = pmatch[0].rm_so;
                    reresults[idx].pmatch[0].rm_eo = pmatch[0].rm_eo;
                    memcpy(&reresults[idx].out[0], &in[off], len);
                    reresults[idx].cb = relut[i].cb;
                    reresults[idx].tid = pthread_self();
                    regfree(&regex); //TBD
                    break;
                }

                // cleanup lex's
                regfree(&regex);
            }
        }
        // wait until all threads finish searching lut and main thread processes results
        pthread_barrier_wait(&b2);

    }
    return((void *)0); // exit
}

int main(int argc, char *argv[]) {

    long i;
    int err, n;
	long long elapsedt=0;
    cb_func_c cb;

    if (argc != 2) {
		printf("Usage:\n./pattern [2-7]\n\nWhere:\n\t2-7 is the number of threads\n");
		exit(EXIT_FAILURE);
	}
 
    n = atoi(argv[1]);
    if (n >= 2 && n <= 7){
        nthr = n;
    }
    else {
        printf("The number of threads can only be 2 to 7\n\n");
   		printf("Usage:\n./pattern [2-7]\n\nWhere:\n\t2-7 is the number of threads\n");
        exit(EXIT_FAILURE);
    }

    // enable search
    dap_pattern_control = ENABLED;

    // clear result structures
    clearresults();

    // create thread barrier
    pthread_barrier_init(&b1, NULL, nthr+1); // sync at start
    pthread_barrier_init(&b2, NULL, nthr+1); // sync when all threads complete
    
    // create nthr number of threads to search table
    for (i = 0; i < nthr; i++) {
        err = pthread_create(&tid[i], NULL, thr_fn, (void *)i);
        if (err != 0) {
            exit(EXIT_FAILURE);
        }
    }

    for (;;){
        fprintf(stdout,"Enter data packet to process: ");
        if (fscanf(stdin, "%s", in) > 0) {

            if (strcmp(EXIT_STRING, in) == 0) {
                // quit
                exit(0);
            }
            else {
                // start timer
                elapsed_time(START, &start, &end);
                
                // clear results
                clearresults();

                // wait until all threads synced
                // once last thread reaches the barrier
                // we wiil start the pattern matching
                pthread_barrier_wait(&b1);

                // wait until all threads have finished
                pthread_barrier_wait(&b2);

                elapsedt = elapsed_time(END, &start, &end);
                fprintf(stdout, "Search time is %lld usecs with %d threads\n", elapsedt, nthr);
                
                // all threads have finished.
                // get the results
                getresults();

            }
        }
    }


}
