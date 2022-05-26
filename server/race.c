#include<stdlib.h>
#include<unistd.h>
#include"race.h"
struct racetrack_ht tr_racetrack_htable;
size_t racetrack_hashstr(uint32_t num)
{
    return num % tr_racetrack_htable.bcnt;
}
int racetrack_init(void)
{
    int succ = 0;
    return succ;
}
int racetrack_insert(uint32_t num)
{
    int succ = 0;
    return succ;
}
int racetrack_join(uint32_t num, int cfd, const char *name)
{
    int succ = 0;
    return succ;
}
void racetrack_remove(uint32_t num)
{
    size_t h = racetrack_hashstr(num);
    if(tr_racetrack_htable.buckets[h].cap != 0)
    {
        if(tr_racetrack_htable.buckets[h].num == num)
        {
            rm:
            for(const struct racer *rp = tr_racetrack_htable.buckets[h].racers; rp != tr_racetrack_htable.buckets[h].racers + tr_racetrack_htable.buckets[h].cnt; ++rp)
                close(rp->cli);
            free(tr_racetrack_htable.buckets[h].racers);
            tr_racetrack_htable.buckets[h].cap = 0;
        }
        else
        {
            size_t og = h++;
            for(; h != og; ++h, h = racetrack_hashstr(h))
            {
                if(tr_racetrack_htable.buckets[h].num == num)
                    goto rm;
                else if(tr_racetrack_htable.buckets[h].cap == 0)
                    h = og - 1;
            }
        }
    }
}
