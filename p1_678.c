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

#define PERM_FLAG         0644    // May also need to be 0666

// definition of message -------------------------------------------
struct message{
   long mtype;
   int mtext[BUFFER_SIZE];
};

struct shared_mem {
   uint go_flag;
   uint done_flag[NUM_CHILD];
   int individual_sum[NUM_CHILD];
   int type_flag;
};

/* function "millisleep" ------------------------------------------ */
void millisleep(unsigned ms)
{ usleep(ms * 1000); }

// uniform_rand ///////////////////////////////////////////////////////////////
unsigned int uniform_rand(void)
/* generate a random number 0 ~ 999 */
{
	unsigned int my_rand;

	my_rand = rand() % 1000;

	return (my_rand);
}

/* Process C1 ============================================================= */
void consumer_process(struct shared_mem * p_shm, int msqid, int process_number)
{
   int          i;            // the loop counter
   int          status;       // result status code           
   unsigned int my_rand;      // a randon number
   unsigned int checksum;     // the local checksum

   struct message buf;
   char* consumer_string = "first";
   int messages_received = 0;

   if (process_number == 3) {
      consumer_string = "second";
   }

   // REQUIRED output #1 -------------------------------------------
   // NOTE: C1 can not make any output before this output
   printf("    Child Process #%d is created ....\n", process_number);
   printf("    I am the %s consumer ....\n\n", consumer_string);

   // REQUIRED: shuffle the seed for random generator --------------
   srand(time(0)); 

   while (p_shm->go_flag == 0){
      // printf("Waiting to start Consumer\n");
      millisleep(1000);
   }

   while (messages_received < NUM_REPEATS)
   {
      // printf("Recieving");
      // don't wait for a message to appear
      status = msgrcv(msqid, (struct message*)&buf, sizeof(buf.mtext), 0, 0); 

      if (status == -1) {
         perror("Error occured when recieving message in consumer process");
         continue;
      }

      my_rand = buf.mtext[0];

      p_shm->individual_sum[process_number] += my_rand;

      messages_received++;
   }

   checksum = p_shm->individual_sum[process_number];

   // REQUIRED 3 second wait ---------------------------------------
   millisleep (THREE_SECONDS);

   // REQUIRED output #2 -------------------------------------------
   // NOTE: after the following output, C1 can not make any output
   printf("    Child Process #%d is terminating (checksum: %d) ....\n\n", process_number, checksum);

   // raise my "Done_Flag" -----------------------------------------
   p_shm->done_flag[process_number] = checksum;  // I m done!
}

void producer_process(struct shared_mem * p_shm, int msqid, int process_number)
{
   int          i;            // the loop counter
   int          status;       // result status code           
   unsigned int my_rand;      // a randon number
   unsigned int checksum;     // the local checksum

   struct message buf;

   char* consumer_string = "first";

   if (process_number == 1) {
      consumer_string = "second";
   }

   printf("    Child Process #%d is created ....\n", process_number);
   printf("    I am the %s producer ....\n\n", consumer_string);

   while (p_shm->go_flag == 0){
      // printf("Waiting to start Producer\n");
      millisleep(1000);
   }

   for (i = 0; i < NUM_REPEATS; i++)
   {
      my_rand = uniform_rand();
      buf.mtype = 1;
      buf.mtext[0] = my_rand;

      status = msgsnd(msqid, (struct message*)&buf, sizeof(buf.mtext), 0);
      // printf("Producing random number: %d\n", my_rand);

      if (status == -1) {
         perror("Error occured when recieving message in consumer process");
      }

      p_shm->individual_sum[process_number] += my_rand;
   }


   checksum = p_shm->individual_sum[process_number];

   // REQUIRED 3 second wait ---------------------------------------
   millisleep (THREE_SECONDS);

   // REQUIRED output #2 -------------------------------------------
   // NOTE: after the following output, C1 can not make any output
   printf("    Child Process #%d is terminating (checksum: %d) ....\n\n", process_number, checksum);

   // raise my "Done_Flag" -----------------------------------------
   p_shm->done_flag[process_number] = checksum;  // I m done!
}

bool processes_done(struct shared_mem* p_shm) {
   for (int i = 0; i < NUM_CHILD; i++){
      if (p_shm->done_flag[i] == 0){
         return false;
      }
   }

   return true;
}

int main(void) {
   int message_queue_id;
   int shared_mem_id;
   uint child_processes = 1;

   struct shared_mem shmem;
   struct shared_mem* shared_mem_pointer;
   
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

   shared_mem_pointer = shmat(shared_mem_id, NULL, 0);
   shared_mem_pointer->type_flag = 0;

   // determines which type of process the child process is
   // 0 - 1 is producer, 2 - 3 is consumer
   int process_type_flag = 0;

   int process_id;
   for (int i = 0; i < NUM_CHILD; i++){
      process_id = fork();

      process_type_flag = shared_mem_pointer->type_flag;

      if (process_id != 0){
         break;
      }

      shared_mem_pointer->type_flag += 1;
   }

   printf("Fork ID: %d. Type: %d\n", process_id, process_type_flag);

   if (process_id != 0) {
      if (process_type_flag == 0 || process_type_flag == 1){
         producer_process(shared_mem_pointer, message_queue_id, process_type_flag);
      } else {
         consumer_process(shared_mem_pointer, message_queue_id, process_type_flag);
      }
   } else if (process_id == 0){
      shared_mem_pointer->go_flag = 1;

      while(!processes_done(shared_mem_pointer)){
         // printf("Waiting for children to finish \n");
      }

      int send_checksum = shared_mem_pointer->individual_sum[0] + shared_mem_pointer->individual_sum[1];
      int receive_checksum = shared_mem_pointer->individual_sum[2] + shared_mem_pointer->individual_sum[3];

      printf("SEND CHECKSUM: %d\n", send_checksum);
      printf("RCV CHECKSUM: %d\n", receive_checksum);

      if (send_checksum == receive_checksum){
         printf("PROGRAM SUCCESS. CHECKSUMS VALID\n");
      } else {
         printf("PROGRAM FAILED: CHECKSUMS INVALID\n");
      }   


      // Delete the message queue
      msgctl(message_queue_id, IPC_RMID, NULL);
      printf("Deleted Message Queue, ID: %d\n", message_queue_id);

      // Delete shared memory
      shmctl(shared_mem_id, IPC_RMID, NULL);
      printf("Deleted Shared Memory, ID: %d\n", shared_mem_id);
   }
}