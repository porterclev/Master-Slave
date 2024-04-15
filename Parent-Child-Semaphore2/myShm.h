struct CLASS
{
    int count; /*shared variable, need mutual exclusion*/

    /*
        Assignment 5
    */
    sem_t mutex_sem; /* enforce mutual exclusion for accessing count */
    int index;
    int response[10];
};