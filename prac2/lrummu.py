from mmu import MMU

class LruMMU(MMU):
    def __init__(self, frames):
        self.access_time = 0
        self.page_table = {}    # {page_num: {'frame': frame_id, 'dirty': is_write}}
        self.last_used = {}     # {page_num: last_access_time}
        self.num_frames = frames # the total number of frames
        self.debug = False       # for debug mode
        self.free_frames = list(range(frames)) # the list of free frames
        self.disk_reads = 0     # the total number of disk reads
        self.disk_writes = 0   # the total number of disk writes
        self.page_faults = 0  # the total number of page faults

    def set_debug(self):
        self.debug = True

    def reset_debug(self):
        self.debug = False

    def read_memory(self, page_number):
        self.access_page(page_number, False)

    def write_memory(self, page_number):
        self.access_page(page_number, True)

    def get_total_disk_reads(self):
        return self.disk_reads

    def get_total_disk_writes(self):
        return self.disk_writes

    def get_total_page_faults(self):
        return self.page_faults

    def lru_paging(self):
        evict = min(self.last_used, key = self.last_used.get) # get lru page
        record = self.page_table.pop(evict)  # remove the lru from page table
        self.last_used.pop(evict) # remove the lru page from last used record

        # if the page is dirty, write to disk
        if record['dirty']:      
          self.disk_writes += 1
          if self.debug:
            print(f"page = {evict}, evict from frame {record['frame']}, dirty, write to disk")
        
        self.free_frames.append(record['frame']) # add the frame id, so this frame is free

    def access_page(self, page_num, is_write):
        self.access_time += 1

        # Hit
        if page_num in self.page_table:
            self.last_used[page_num] = self.access_time

            if is_write:
                self.page_table[page_num]['dirty'] = True
            if self.debug:
                if is_write:
                    print(f"page = {page_num}, hit, write")
                else:
                    print(f"page = {page_num}, hit, read")
            return 
        
        # Miss
        self.page_faults += 1
        if self.debug:
            if is_write:
                print(f"page = {page_num}, miss, write")
            else:
                print(f"page = {page_num}, miss, read")        
        
        # no free frame, implement LRU paging
        if not self.free_frames:
            self.lru_paging()
        
        # allocate frame for the page
        frame_id = self.free_frames.pop()
        self.page_table[page_num] = {'frame': frame_id, 'dirty': is_write}
        self.last_used[page_num] = self.access_time
        self.disk_reads += 1
        if self.debug:
            print(f"page = {page_num}, load into the free frame {frame_id}")
        
        
 

