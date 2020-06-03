

// C program for simple memory management using Best Fit technique

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct memblock
{
    int flag;              //indicates whether a block is free or not
    int lower;             //keeps track of lower boundary of a block
    int higher;            //keeps track of higher boundary of a block
    int cap;               //capacity of each block
    int process_name;      //stores a process id or is set to -1 if the block is free
    struct memblock *next; //this will point to the next block
};

void bestFit(int name, int psize, struct memblock **headref);        //For RQ
struct memblock *newBlock(int name, int lower, int psize, int flag); //For creating a new memory space for incoming process
void releaseProcess(int name, struct memblock **headref);            //For RL
void printLinkedList(struct memblock **head);                        //For STAT
void compactMemory(struct memblock **headref);                       //For C

struct memblock *head = NULL;
int total_block_size;
char empty[7] = "EMPTY";

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        printf("Please provide the total size of the block of mem you wish to allocate.\n");
        return 0;
    }
    total_block_size = atoi(argv[1]);
    int *mem_pointer;
    mem_pointer = (int *)malloc(total_block_size); //allocating the total memory using malloc. We will be allocating our process chunks of this memory.
    if (mem_pointer == NULL)                       //if allocation isnt successfull, mem_pointer will be returning null
    {
        printf("Couldnt allocate the memory");
        return 0;
    }

    int exit = 0;
    //this loop contains the main program of memory management. Program will terminate when user type in 'exit'. Otherwise it will keep accepting more requests.

    while (!exit)
    {

        int MAX = 30;
        char buf[MAX];
        char cmd[5];
        char exit_s[5] = "quit";
        char request[3] = "RQ";
        char stat[5] = "STAT";
        char compact[3] = "C";
        char release[3] = "RL";
        int psize = 0;
        int pid = -1;
        printf("\nallocator> ");
        fgets(buf, MAX, stdin);                     //gets the entire line of input
        sscanf(buf, "%s %d %d", cmd, &pid, &psize); //scans first three (if applicable) and assign them to certain variables

        //user enters exit
        if (!strcasecmp(cmd, exit_s))
        {
            exit = 1;
            return 0;
        }

        //user requests for certain bytes of memory in our total block
        if (!strcasecmp(cmd, request))
        {
            if ((pid == -1) || (psize == 0)) //checking if process_id and size to be allocated is passed
            {
                printf("RQ requires PID and Process_Size as 2nd and 3rd parameters. Like: 'RQ XXXX 20000'.\n");
                continue;
            }

            //if everything goes right, we shall procced to assign the block of memory to the arriving process
            bestFit(pid, psize, &head);
        }

        //stat for the memory will be displayed
        else if (!strcasecmp(cmd, stat))
        {
            printLinkedList(&head);
        }

        //compaction will be done so that smaller holes can be removed within the memory and larger single hole can be formed so as new process can use it effectively
        else if (!strcasecmp(cmd, compact))
        {
            compactMemory(&head);
        }

        // releasing process from the memory
        else if (!strcasecmp(cmd, release))
        {
            if (pid == -1) //If process name isnt passed, no prcess can be removed.
            {
                printf("RL requires PID as 2nd parameter as in 'RL XXXX'\n");
                continue;
            }
            releaseProcess(pid, &head);
        }

        //anything else will result in invalid command. Therfore such cmds will result a error message and program will continue taking more requests.
        else
        {
            printf("Invalid Syntax. Available Commands: RQ, STAT, C, RL\n");
        }
    }
}

void bestFit(int pid, int psize, struct memblock **headref)
{
    if ((*headref) == NULL)
    {
        struct memblock *first_block = newBlock(-1, 0, total_block_size, 0); //allocating total space as empty at first
        (*headref) = first_block;
    }
    if ((*headref) != NULL)
    { //now that all our space is allocated we will assign our incoming process blocks
        //we will have to find the position to insert
        struct memblock *temp_block = *headref;
        int min_fitting_size = total_block_size + 1;
        while (temp_block != NULL) //traversing linked list until we find
        {
            if ((temp_block->flag == 0) && (temp_block->cap >= psize))
            { //process fits and the block is empty, we find the block which has the min fitting capacity which is the principle of best fit
                min_fitting_size = temp_block->cap;
            }
            temp_block = temp_block->next;
        }
        if (min_fitting_size == total_block_size + 1)
        { //if min_fitting_size couldnt be updated, it means we dont have available space for the incoming process
            printf("No available space for your process. Please delete items to proceed.");
            return;
        }

        //now we search for the min_fitting_size in the linked list.
        temp_block = *headref;
        while (temp_block != NULL)
        {
            if ((temp_block->cap == min_fitting_size) && (temp_block->flag == 0))
            { //we have found the minimum size block to fit our process
                if (temp_block->cap == psize)
                { //if incoming process has exactly the size of available space in the block

                    temp_block->process_name = pid; //we simply change the name of the block
                    temp_block->flag = 1;           //and also update flag to occupied
                }
                else
                {                                             //we will have to break the node into 2 parts:left filled with incoming process and right  empty available block
                    struct memblock *temp = temp_block->next; //storing refernece to next so that it doesnt get lost
                    int block_size = temp_block->cap;

                    temp_block->process_name = pid;
                    temp_block->cap = psize;
                    temp_block->flag = 1;
                    temp_block->higher = temp_block->lower + psize;

                    //creating a new node for empty leftover block
                    struct memblock *right_empty_block = newBlock(-1, temp_block->higher + 1, block_size - psize, 0);
                    temp_block->next = right_empty_block; //we join the empty block to linked list
                    right_empty_block->next = temp;       //joining with remaining final part linked list
                }
            }
            temp_block = temp_block->next;
        }
    }
}

void releaseProcess(int pid, struct memblock **headref)
{
    //first step to removing a process is finding the block where it is stored; we find it by process name or pid
    struct memblock *temp_mem = *headref;
    if (temp_mem == NULL)
    {
        printf("There is currently no processes in the memory!");
    }

    while (temp_mem != NULL) //searching for the memblock according to pid inputed
    {
        if (temp_mem->process_name == pid)
        { //if input pid matches the one in the memblock, we update the name of the block to empty and flag to zero.
            temp_mem->process_name = -1;
            temp_mem->flag = 0;
            return;
        }
        temp_mem = temp_mem->next;
    }
    printf("No process with such ID could be found.");
    return;
}

struct memblock *newBlock(int pid, int lower, int psize, int flag)
{
    /* allocating node */
    struct memblock *new_block =
        (struct memblock *)malloc(sizeof(struct memblock));

    /* putting in the data  */
    new_block->flag = flag;
    new_block->lower = lower;
    new_block->process_name = pid;
    new_block->higher = (lower + psize);
    new_block->cap = psize;
    return new_block;
}

void compactMemory(struct memblock **headref)
{ //compaction is done by creating a new list which first takes in only occupied blocks (filled with process). Then as a final block, we have empty space that equals to total fragmented chunks that we had before.
    struct memblock *temp_block = *headref;
    struct memblock *new_filled_list = NULL;
    struct memblock *new_empty_list = NULL;
    int total_space_seized = 0;
    while (temp_block != NULL)
    {
        if (temp_block->flag == 1) //if tem_block is filled, we put them together in one list.
        {
            if (new_filled_list == NULL)
            { // new list isnt yet initialized
                new_filled_list = newBlock(temp_block->process_name, 0, temp_block->cap, 1);
            }
            else
            { //let's traverse the new list and get our spot
                struct memblock *temp_new_list = new_filled_list;
                while (temp_new_list->next != NULL)
                {
                    temp_new_list = temp_new_list->next;
                }
                //now temp_new_list holds the last node, that is where we need to insert the next filled memBlock
                temp_new_list->next = newBlock(temp_block->process_name, temp_new_list->higher + 1, temp_block->cap, 1);
                //this process will copy from our original list entire memblocks filled with processes and put them one after another.
            }
        }
        else
        { // we put them in another list; basically it will be a single larrge block which combines all empty chunks of mempry capacity
            total_space_seized += temp_block->cap;
        }
        temp_block = temp_block->next;
    }
    //now we find the position where we can insert the empty block
    struct memblock *tempo_block = new_filled_list;

    while (tempo_block->next != NULL)
    {
        tempo_block = tempo_block->next;
    }
    tempo_block->next = newBlock(-1, tempo_block->higher + 1, total_space_seized, 0);
    *headref = new_filled_list; //point the head to a new list with compaction
}

void printLinkedList(struct memblock **head)
{
    struct memblock *temp_block = *head;
    while (temp_block != NULL)
    {
        if (temp_block->higher > total_block_size)
        {
            if (temp_block->flag == 0)
            {
                printf("\nAddresses [%d : END ] FREE\n", temp_block->lower);
            }
            else
            {
                printf("\nAddresses [%d : END ] Process P%d\n", temp_block->lower, temp_block->process_name);
            }
        }
        else
        {
            if (temp_block->flag == 0)
            {
                printf("\nAddresses [%d : %d] FREE", temp_block->lower, temp_block->higher);
            }
            else
            {
                printf("\nAddresses [%d : %d] Process P%d", temp_block->lower, temp_block->higher, temp_block->process_name);
            }
        }
        temp_block = temp_block->next;
    }
}