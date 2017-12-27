/* p3helper.c
 * Connor Guy
 * Program 3
 * CS570
 * John Carroll
 * SDSU
 * 11/3/17
 *
 * This is the only file you may change. (In fact, the other files should
 * be symbolic links to:
 *   ~cs570/Three/p3main.c
 *   ~cs570/Three/p3.h
 *   ~cs570/Three/Makefile
 *   ~cs570/Three/CHK.h    )
 *
 */
#include "p3.h"

// Prototypes
void fairProlog(int kind);
void writerProlog(int kind);
void fairEpilog(int kind);
void writerEpilog(int kind);

/* You may put any semaphore (or other global) declarations/definitions here: */
    sem_t db;                // Locking for db
    sem_t writing;           // Lock out writers
    sem_t lock;              // Used to lock critical data
    sem_t writePriority;     // When we are using write priority alg
    int readersCount = 0;
    int writersCount = 0;

/* General documentation for the following functions is in p3.h
 * Here you supply the code:
 */
void initstudentstuff(int protocol) 
{
    if(sem_init(&db, 0, 1) != 0)
        perror("sem_init failed on reading.");
    if(sem_init(&lock, 0, 1) != 0)
        perror("sem_init failed on lock.");
    if(sem_init(&writing, 0, 1) != 0)
        perror("sem_init failed on writing.");
    if(sem_init(&writePriority, 0, 1) != 0)
        perror("sem_init failed on writePriority.");
}

void prolog(int kind, int protocol) 
{
    if(kind == READER)
    {
    	if(protocol == WRIT) // If were doing writer priority, need to make sure no writers are waiting.
    	{
    		sem_wait(&writePriority);
    	}

        sem_wait(&lock); // No writers qued and we're adding the reader and tmp locking rcount    	
        if(readersCount == 0)
    	{
    	 	sem_wait(&db); //If new reader is the first reader close the db
    	}
        readersCount++; // Make sure to track how many readers are going in
        sem_post(&lock); // Allow others to modify rcount/db
    	
    	if(protocol == WRIT) // If were doing writer priority
    	{
    		sem_post(&writePriority);
    	}
    }

    if(kind == WRITER)
    {
        if(protocol == WRIT)
        {
        	sem_wait(&lock); // No one else modify things
        	if(writersCount == 0) // Lock out readers till all writers done
        		sem_wait(&writePriority);
        	writersCount++; // A writer is waiting
        	sem_post(&lock); // Ok to modify things
        }
        sem_wait(&writing);
        sem_wait(&db);
    }
}

void epilog(int kind, int protocol) 
{
    if(kind == READER)
    {
        sem_wait(&lock); // Lock out reader from modifying rcount
        readersCount--; // A reader has left
        if(readersCount == 0) sem_post(&db); // If no readers left unlock db
        sem_post(&lock); // Allow other reader to mod rcount/db
    }
    if(kind == WRITER)
    {   
        if(protocol == WRIT)
        {
            sem_wait(&lock); // No one else modify things
            writersCount--; // A writer is leaving
            if(writersCount == 0) // Unlock readers
                sem_post(&writePriority);
            sem_post(&lock); // Ok to modify things
        }
        sem_post(&writing);
        sem_post(&db);
    }
}  

