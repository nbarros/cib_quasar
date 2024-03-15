
extern "C"
{
#include <sys/types.h>
#include <sys/sysinfo.h>
};
#include <cstdint>
#include <cinttypes>
#include <cstdio>
#include <thread>
#include <chrono>

void get_memory_info(uint64_t &total, uint64_t &used, uint64_t &free)
{
  struct sysinfo memInfo;
  sysinfo (&memInfo);
  // unit size in bytes
  total = memInfo.totalram*memInfo.mem_unit/(1024*1024);
  free  = memInfo.freeram*memInfo.mem_unit/(1024*1024);
  used  = total - free;
//  long long physMemUsed = memInfo.totalram - memInfo.freeram;
//  //Multiply in next statement to avoid int overflow on right hand side...
//  physMemUsed *= memInfo.mem_unit;
}


static long long unsigned int lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;

void init_cpu_polling(){
    FILE* file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %llu %llu %llu %llu", &lastTotalUser, &lastTotalUserLow,&lastTotalSys, &lastTotalIdle);
    fclose(file);
}

double getCurrentValue()
{
    double percent;
    FILE* file;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;

    file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow,&totalSys, &totalIdle);
    fclose(file);

    if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow ||
        totalSys < lastTotalSys || totalIdle < lastTotalIdle){
        //Overflow detection. Just skip this value.
        percent = -1.0;
    }
    else{
        total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
            (totalSys - lastTotalSys);
        percent = total;
        total += (totalIdle - lastTotalIdle);
        percent /= total;
        percent *= 100;
    }

    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    return percent;
}
int main(int argc, char**argv)
{
  uint64_t tot, use, free;
  get_memory_info(tot,use,free);
  printf("Mem : tot %" PRIu64 " use %" PRIu64 " free %" PRIu64 "\n",tot,use,free);
  init_cpu_polling();

  for (size_t i = 0; i < 15; ++i)
  {
    printf("CPU load %f\n",getCurrentValue());
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  return 0;
}
