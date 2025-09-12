from mmu import MMU


class ClockMMU(MMU):
    def __init__(self, frames):
        self.frames = frames  # Number of page frames
        self.debug = False  # Debug mode flag
        self.disk_reads = 0  # Counter for disk reads
        self.disk_writes = 0  # Counter for disk writes
        self.page_faults = 0  # Counter for page faults
        
        # Data structures for Clock algorithm
        self.page_table = {}  # Maps page numbers to frame info
        self.frames_list = [None] * frames  # List of frames with page info
        self.reference_bits = [0] * frames  # Reference bits for each frame
        self.clock_hand = 0  # Pointer for Clock algorithm
        
        # For each frame, we'll store (page_number, dirty_bit)
        # Initially all frames are empty

    def set_debug(self):
        self.debug = True

    def reset_debug(self):
        self.debug = False

    def read_memory(self, page_number):
        # Check if page is already in memory
        if page_number in self.page_table:
            frame_index = self.page_table[page_number]
            # Set reference bit to 1
            self.reference_bits[frame_index] = 1
            if self.debug:
                print(f"Page {page_number} is already in frame {frame_index}")
        else:
            # Page fault - need to load page
            self.page_faults += 1
            self.disk_reads += 1
            
            # Find a frame to replace using Clock algorithm
            frame_index = self.find_victim_frame()
            
            # If the victim frame has a page that is dirty, write it to disk
            if self.frames_list[frame_index] is not None:
                victim_page, is_dirty = self.frames_list[frame_index]
                if is_dirty:
                    self.disk_writes += 1
                    if self.debug:
                        print(f"Writing page {victim_page} to disk")
                # Remove the victim page from page table
                if victim_page in self.page_table:
                    del self.page_table[victim_page]
            
            # Load the new page
            self.frames_list[frame_index] = (page_number, False)  # Not dirty initially
            self.page_table[page_number] = frame_index
            self.reference_bits[frame_index] = 1  # Set reference bit
            
            if self.debug:
                print(f"Page fault: loaded page {page_number} into frame {frame_index}")

    def write_memory(self, page_number):
        # Check if page is already in memory
        if page_number in self.page_table:
            frame_index = self.page_table[page_number]
            # Set reference bit to 1 and mark as dirty
            self.reference_bits[frame_index] = 1
            # Update the frame to mark it as dirty
            self.frames_list[frame_index] = (page_number, True)
            if self.debug:
                print(f"Write to page {page_number} in frame {frame_index}")
        else:
            # Page fault - need to load page
            self.page_faults += 1
            self.disk_reads += 1
            
            # Find a frame to replace using Clock algorithm
            frame_index = self.find_victim_frame()
            
            # If the victim frame has a page that is dirty, write it to disk
            if self.frames_list[frame_index] is not None:
                victim_page, is_dirty = self.frames_list[frame_index]
                if is_dirty:
                    self.disk_writes += 1
                    if self.debug:
                        print(f"Writing page {victim_page} to disk")
                # Remove the victim page from page table
                if victim_page in self.page_table:
                    del self.page_table[victim_page]
            
            # Load the new page and mark it as dirty
            self.frames_list[frame_index] = (page_number, True)
            self.page_table[page_number] = frame_index
            self.reference_bits[frame_index] = 1  # Set reference bit
            
            if self.debug:
                print(f"Page fault: loaded page {page_number} into frame {frame_index} (dirty)")
                
    def find_victim_frame(self):
        # Implement Clock algorithm to find a victim frame
        while True:
            # Check the current frame
            if self.reference_bits[self.clock_hand] == 0:
                # Found a frame with reference bit 0
                victim_frame = self.clock_hand
                self.clock_hand = (self.clock_hand + 1) % self.frames
                return victim_frame
            else:
                # Set reference bit to 0 and move to next frame
                self.reference_bits[self.clock_hand] = 0
                self.clock_hand = (self.clock_hand + 1) % self.frames                
                

    def get_total_disk_reads(self):
        # TODO: Implement the method to get total disk reads
        return self.disk_reads

    def get_total_disk_writes(self):
        # TODO: Implement the method to get total disk writes
        return self.disk_writes

    def get_total_page_faults(self):
        # TODO: Implement the method to get total page faults
        return self.page_faults
