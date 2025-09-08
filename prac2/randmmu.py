from mmu import MMU
import random

class RandMMU(MMU):
    def __init__(self, frames):
        self.frames = frames
        self.debug = False
        self.page_table = {}                                                # Track dirty status
        self.memory = []                                                    # Store current pages in memory
        self.disk_reads = 0
        self.disk_writes = 0
        self.page_faults = 0
        self.debug = True

    def reset_debug(self):
        self.debug = False

    def read_memory(self, page_number):
        if page_number in self.memory:                                      # Hit
            if self.debug:
                print(f"Read HIT: page {page_number} hit")
        else:                                                               # Miss :Page fault
            self.page_faults += 1
            self.disk_reads += 1
            if self.debug:
                print(f"Read MISS: page {page_number}  Fault")

            if len(self.memory) < self.frames:
                self.memory.append(page_number)
            else:
                victim = random.choice(self.memory)
                if self.page_table.get(victim, (False,))[0]:
                    self.disk_writes += 1
                    if self.debug:
                        print(f"DIRTY page {victim}")
                self.memory.remove(victim)
                self.memory.append(page_number)

            self.page_table[page_number] = (False, None)

    def write_memory(self, page_number):
        if page_number in self.memory:                                         # Hit
            self.page_table[page_number] = (True, None)
            if self.debug:
                print(f"Write HIT: page {page_number} DIRTY")
        else:  
            self.page_faults += 1
            self.disk_reads += 1
            if self.debug:
                print(f"Write MISS: page {page_number}   Fault")

            if len(self.memory) < self.frames:
                self.memory.append(page_number)
            else:
                victim = random.choice(self.memory)
                if self.page_table.get(victim, (False,))[0]:
                    self.disk_writes += 1
                    if self.debug:
                        print(f"DIRTY page {victim}")
                self.memory.remove(victim)
                self.memory.append(page_number)

            self.page_table[page_number] = (True, None)

    def get_total_disk_reads(self):
        return self.disk_reads

    def get_total_disk_writes(self):
        return self.disk_writes

    def get_total_page_faults(self):
        return self.page_faults
