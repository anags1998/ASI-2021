#define SIZE_SHM 4096		// Size shared memory segment

#define OFF_FIRST_KEY 2		// Offset of first key in memory to find
#define OFF_DATA_SEM 10		// Offset hide data in memory to find with sem
#define OFF_TBL_KEY 16		// Offset of table of keys in memory to write 
#define OFF_REG_KEY 48		// Offset of registry of keys to lookup

#define NSEMS 3			// Number of sems in array

union un_reg_keys {
	int key;		// Key value as int
	char key_str[4];	// Key value as char array   Ej: "234\0"
};

#ifndef __APPLE__
union semun {
	int val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO */
};
#endif

extern int connect_MONITOR(char *);
extern int check_key(int, int );
extern int do_child_work(int fildes[2]);

//#define TST_PROF

#define load_var(var,p) {memcpy(p,&var,sizeof(var));p+=sizeof(var);}
#define save_var(var,p) {memcpy(&var,p,sizeof(var));p+=sizeof(var);}

#define N_CLIENT 1

#define TMP_INTER_EXER 3	// Time between exercises

#define MAX_TRIES_K 4		// Max tries allowed to check key value
#define NKEYS_INITIAL 12	// # keys to check in shm table 

#define PRIME_PIPE 2999		// # prime to check pipe value
#define PRIME_SEM 4999		// # prime to check sem value

#define SECRET_K2 2345		// Number to check response to K2

#define ERROR_DATA 12		// Error in data
#define FAKE_VALUE 2345		// Fake value in table

#define KEY11_VALUE __key11_pair	// Number to set in sem 2

#define CNL_MONITOR 1L
#define CNL_SECRET 2L
