#include "utility.h"

long get_curr_time(){
    struct timeval my_time;
    gettimeofday(&my_time, 0);
    return my_time.tv_sec * 1000;
}
