#include "VirtualMemory.h"
#include "PhysicalMemory.h"

#define FAIL 0
#define SUCCESS 1

/*
 * Reset all cells of the given frame to 0
 */
void resetFrame(uint64_t frameRoot) {
  for(uint64_t i = 0; i < PAGE_SIZE; i++) {
    PMwrite(frameRoot * PAGE_SIZE + i , 0);
  }
}

/*
 * Initialize the virtual memory. Only clears frame 0.
 */
void VMinitialize() {
  resetFrame(0);
}

/*
 * Travel through the tree and change given arguments to get optimal frame
 */
void findOptimalFrame(bool *isEmpty, word_t *emptyVal, uint64_t treeLevel, word_t page, word_t currFrame,
                      word_t *currParent, word_t *maxFrame, uint64_t pagedSwappedIn, uint64_t *optimalVal,
                      word_t *optimalCyclic, word_t *optimalParent, word_t *evict, word_t frame) {

  if(*isEmpty || treeLevel == TABLES_DEPTH) {
    return;
  }
  bool isValueExist = false;
  word_t val;
  page = page << OFFSET_WIDTH;

  for(int i = 0; i < PAGE_SIZE; i++) {
    PMread(currFrame * PAGE_SIZE + i, &val);
    if (val != 0) {
      isValueExist = true; // A subtree exists
      if(val > *maxFrame) {
        *maxFrame = val;
      }
      if(treeLevel + 1 == TABLES_DEPTH) {
        // option1, option2 refers to the options in the min function in the ex.pdf
        uint64_t p = (uint64_t) page + i;
        uint64_t option2;
        uint64_t min;

        // abs:
        if(pagedSwappedIn > p) {
            option2 = pagedSwappedIn - p;
        }
        else {
            option2 = p - pagedSwappedIn;
        }

        uint64_t option1 = NUM_PAGES - option2;

        // find min:
        if (option1 < option2) {
            min = option1;
        }
        else {
            min = option2;
        }

        if(min > *optimalVal) {
          *optimalVal = min;
          *optimalParent = currFrame;
          *optimalCyclic = val;
          *evict = page + i;
        }
      }
      *currParent = currFrame;

      findOptimalFrame(isEmpty, emptyVal, treeLevel + 1, page + i, val, currParent, maxFrame, pagedSwappedIn,
                       optimalVal, optimalCyclic, optimalParent, evict, frame);

      if (*isEmpty) {
        return;
      }
    }
  }

  if (!isValueExist and currFrame != frame) {
    *isEmpty = true;
    *emptyVal = currFrame;
    *optimalParent = *currParent;
    return;
  }

}

/*
 * Find an unused frame or evict a page from some frame.
 * Suppose this frame number is f1. Write 0 in all of its contents
 * (only necessary if next layer is a table)
 */
word_t findUnusedFrame(uint64_t pagedSwappedIn, word_t frame) {
  bool isEmpty = false;
  word_t emptyVal = 0;
  word_t maxFrame = 0;
  word_t currParent = 0;
  word_t optimalParent = 0;
  word_t evict = 0;
  word_t optimalCyclic = 0;
  uint64_t optimalVal = 0;

  findOptimalFrame(&isEmpty, &emptyVal, 0, 0, 0, &currParent, &maxFrame, pagedSwappedIn,
                   &optimalVal, &optimalCyclic, &optimalParent, &evict, frame);

  if(isEmpty) {
    word_t value;
    for (uint64_t i = 0; i < PAGE_SIZE; i++) {
      uint64_t physicalAddress = optimalParent * PAGE_SIZE + i;
      PMread(physicalAddress, &value);
      if(value == emptyVal) {
        PMwrite(physicalAddress, 0);
        break;
      }
    }
    return emptyVal;
  }

  if (maxFrame + 1 < NUM_FRAMES) {
    resetFrame(maxFrame + 1);
    return maxFrame + 1;
  }

  word_t value;
  for (uint64_t i = 0; i < PAGE_SIZE; i++) {
      uint64_t physicalAddress = optimalParent * PAGE_SIZE + i;
      PMread(physicalAddress, &value);
      if(value == optimalCyclic) {
          PMwrite(physicalAddress, 0);
          break;
      }
  }

  PMevict(optimalCyclic, evict);
  resetFrame(optimalCyclic);
  return optimalCyclic;
}

/*
 * Converts a virtual address to a physical address
 */
uint64_t virtualToPhysical(uint64_t virtualAddress) {
  word_t root = 0;
  word_t addr1 = 0;
  word_t frame = 0;
  uint64_t swapped = virtualAddress >> OFFSET_WIDTH;

  for(uint64_t i = 0; i < TABLES_DEPTH; i++) {
    uint64_t shifts = (TABLES_DEPTH - i) * OFFSET_WIDTH;
    uint64_t p = virtualAddress >> shifts;
    p = p % PAGE_SIZE;
    PMread(root * PAGE_SIZE + p, &addr1);

    if(addr1 == 0) {
      frame = findUnusedFrame(swapped, frame);
      PMwrite(root * PAGE_SIZE + p, frame);
      root = frame;
    }
    else {
      root = addr1;
    }
    frame = root;
//    root = root * PAGE_SIZE;
  }
  PMrestore(frame, swapped);
  return frame * PAGE_SIZE + (virtualAddress % PAGE_SIZE);
}

/* Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value){
  if(virtualAddress >= VIRTUAL_MEMORY_SIZE || virtualAddress < 0) {
    return FAIL;
  }
  uint64_t physicalAddress = virtualToPhysical (virtualAddress);
  PMread(physicalAddress, value);
  return SUCCESS;
}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value){
  if(virtualAddress >= VIRTUAL_MEMORY_SIZE || virtualAddress < 0) {
      return FAIL;
    }
  uint64_t physicalAddress = virtualToPhysical (virtualAddress);
  PMwrite(physicalAddress, value);
  return SUCCESS;
}