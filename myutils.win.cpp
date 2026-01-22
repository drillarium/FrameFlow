#include "myutils.h"
#include <Windows.h>
#include <psapi.h>

double cpuUsage()
{
  static ULARGE_INTEGER prevIdle, prevKernel, prevUser;

  FILETIME idle, kernel, user;
  GetSystemTimes(&idle, &kernel, &user);

  ULARGE_INTEGER i, k, u;
  i.LowPart = idle.dwLowDateTime;
  i.HighPart = idle.dwHighDateTime;
  k.LowPart = kernel.dwLowDateTime;
  k.HighPart = kernel.dwHighDateTime;
  u.LowPart = user.dwLowDateTime;
  u.HighPart = user.dwHighDateTime;

  double idleDiff = (double) (i.QuadPart - prevIdle.QuadPart);
  double totalDiff = (double) ((k.QuadPart - prevKernel.QuadPart) + (u.QuadPart - prevUser.QuadPart));

  prevIdle = i;
  prevKernel = k;
  prevUser = u;

  return (1.0 - idleDiff / totalDiff) * 100.0;
}

MemoryInfo memoryUsage()
{
  MEMORYSTATUSEX mem{};
  mem.dwLength = sizeof(mem);
  GlobalMemoryStatusEx(&mem);

  double totalGB = mem.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);
  double usedGB = (mem.ullTotalPhys - mem.ullAvailPhys) / (1024.0 * 1024.0 * 1024.0);

  MemoryInfo info;
  info.totalGB = totalGB;
  info.usedGB = usedGB;
  info.usedPercent = usedGB / totalGB * 100.0;

  return info;
}

