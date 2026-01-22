#pragma once

struct MemoryInfo
{
  double usedPercent;
  double usedGB;
  double totalGB;
};

double cpuUsage();
MemoryInfo memoryUsage();
