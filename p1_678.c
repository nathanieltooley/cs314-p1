/* ********************************************************* *
 * p1_solution.cpp:                                          *
 *                                                           *
 * os.cs.siue.edu                                            *
 *                                                           *
 * compile: "cc p1_solution.cpp"                             *
 *                                                           *
 * Created by Nathaniel Tooley                               *
 * 2:57 p.m., February 3, 2023                               *
 * ********************************************************* */
#include <stdio.h>                // for printf
#include <unistd.h>               // for usleep

#include <stdlib.h>               // for exit 
#include <stdbool.h>              // for "false" and "true"   
#include <string.h>               // for strcpy   

#include <time.h>                 // for time()

#include <sys/ipc.h>              // for IPC   
#include <sys/shm.h>              // for shared memory system calls
#include <sys/sem.h>              // for semaphore system calls
#include <sys/msg.h>              // for message queue

#define SHM_KEY           8810    // the shared memory key 
#define MSG_KEY           8310    // (unique) message queue key

#define NUM_REPEATS        200    // number of loops for high-priority processes
#define NUM_CHILD            4    // number of the child processes

#define BUFFER_SIZE       1024    // max. message queue size

#define ONE_SECOND     1000000    //    1 second
#define THREE_SECONDS  3000000    //    3 seconds

#define PERM_FLAG         0644

// definition of message -------------------------------------------
struct message{
         long mtype;
         int mtext[BUFFER_SIZE];
};

struct shared_mem {
   uint go_flag;
   uint done_flag[NUM_CHILD];
   int individual_sum[NUM_CHILD];
};

/* function "millisleep" ------------------------------------------ */
void millisleep(unsigned micro_seconds)
{ usleep(micro_seconds); }

// uniform_rand ///////////////////////////////////////////////////////////////
unsigned int uniform_rand(void)
/* generate a random number 0 ~ 999 */
{
	unsigned int my_rand;

	my_rand = rand() % 1000;

	return (my_rand);
}

/* Process C1 ============================================================= */
void process_C1(struct shared_mem * p_shm, int msqid)
{
   int          i;            // the loop counter
   int          status;       // result status code           
   unsigned int my_rand;      // a randon number
   unsigned int checksum;     // the local checksum

   // REQUIRED output #1 -------------------------------------------
   // NOTE: C1 can not make any output before this output
   printf("    Child Process #1 is created ....\n");
   printf("    I am the first consumer ....\n\n");

   // REQUIRED: shuffle the seed for random generator --------------
   srand(time(0)); 

 








   // REQUIRED 3 second wait ---------------------------------------
   millisleep (THREE_SECONDS);

   // REQUIRED output #2 -------------------------------------------
   // NOTE: after the following output, C1 can not make any output
   printf("    Child Process #1 is terminating (checksum: %d) ....\n\n", checksum);

   // raise my "Done_Flag" -----------------------------------------
   p_shm->done_flag[0] = checksum;  // I m done!
}

int main(void) {
   int message_queue_id;
   int shared_mem_id;

   struct shared_mem shmem;
   
   size_t shm_size = sizeof shmem;

   printf("Size of shared memory: %d\n", shm_size);

   // Create message queue
   message_queue_id = msgget(MSG_KEY, PERM_FLAG | IPC_CREAT);
   shared_mem_id = shmget(SHM_KEY, shm_size, PERM_FLAG | IPC_CREAT);

   if (message_queue_id == -1) {
      perror("Error creating message queue. Returned with id -1\n");
      exit(1);
   }

   if (shared_mem_id == -1) {
      perror("Error creating shared memory. Returned id -1\n");
   }

   printf("Created Message Queue with id, %d\n", message_queue_id);
   printf("Created shared memory with id, %d\n", shared_mem_id);

   // Delete the message queue
   msgctl(message_queue_id, IPC_RMID, NULL);

   // Delete shared memory
   shmctl(shared_mem_id, IPC_RMID, NULL);
}