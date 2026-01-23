#ifndef LRUHOST_H
#define LRUHOST_H
#include <M5Unified.h>

struct HostInfo {
  String hostname;
  bool isActive = false;
  uint32_t lastUsed = 0;
};

const int MAX_HOSTS = 4;

class LRUHost {
public:
  uint8_t getHostId(String name);
  LRUHost(uint8_t max);
private:
  uint8_t maxHosts;
  std::vector<HostInfo> hostList;
  uint32_t globalCounter = 0;
};

#endif // LRUHOST_H
