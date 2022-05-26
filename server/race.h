#ifndef Included_race_h
#define Included_race_h
#include<stddef.h>
#include<stdint.h>
struct racer
{
    char name[30];
    int cli;
};

struct racetrack
{
    struct racer *racers;
    size_t cnt, cap;
    uint32_t num;
};

struct racetrack_ht
{
    struct racetrack *buckets;
    size_t cnt, bcnt;
};

int racetrack_init(void);
int racetrack_insert(uint32_t num);
int racetrack_join(uint32_t num, int cfd, const char *name);
void racetrack_remove(uint32_t num);
#endif
