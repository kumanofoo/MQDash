#include "LRUHost.h"

LRUHost::LRUHost(uint8_t max) : maxHosts(max), hostList(max) {}

uint8_t LRUHost::getHostId(String name) {
  int targetId = -1;
  
  // The host exist in the list
  for (int i = 0; i < maxHosts; i++) {
    if (hostList[i].isActive && hostList[i].hostname == name) {
      targetId = i;
      break;
    }
  }

  // search empty slot if the host doesn't exists
  if (targetId == -1) {
    for (int i = 0; i < maxHosts; i++) {
      if (!hostList[i].isActive) {
        targetId = i;
        break;
      }
    }
  }

  // search the oldest host if no empty slot.
  if (targetId == -1) {
    uint32_t oldestTime = 0xFFFFFFFF; // Maximum count
    for (int i = 0; i < maxHosts; i++) {
      if (hostList[i].lastUsed < oldestTime) {
        oldestTime = hostList[i].lastUsed;
        targetId = i;
      }
    }
  }

  globalCounter += 1;
  hostList[targetId].hostname = name;
  hostList[targetId].isActive = true;
  hostList[targetId].lastUsed = globalCounter;

  return targetId;
}

