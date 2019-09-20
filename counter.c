#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <stdint.h>
#include <sys/mman.h>
#include "hps_0.h"

// The start address and length of the Lightweight bridge
#define HPS_TO_FPGA_LW_BASE 0xFF200000
#define HPS_TO_FPGA_LW_SPAN 0x0020000

int main(int argc, char ** argv)
{
	void * lw_bridge_map = 0;
	short * init = 0;
	short * cycles = 0;
	int devmem_fd = 0;
	int result = 0;
	char current;
	char count;
	int i;
	
 // Open up the /dev/mem device (aka, RAM)
	devmem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(devmem_fd < 0) {
		perror("devmem open");
		exit(EXIT_FAILURE);
	}

// mmap() the entire address space of the Lightweight bridge so we can access our custom module 
	lw_bridge_map = (uint32_t*)mmap(NULL, HPS_TO_FPGA_LW_SPAN, PROT_READ|PROT_WRITE, MAP_SHARED, devmem_fd, HPS_TO_FPGA_LW_BASE); 

	if(lw_bridge_map == MAP_FAILED) {
		perror("devmem mmap");
		close(devmem_fd);
		exit(EXIT_FAILURE);
	}
	
// Set the init to the correct offset within the RAM (CUSTOM_SLAVE_0_BASE is from "hps_0.h")
	init = (short*)(lw_bridge_map + INIT_BASE );
	cycles = (short*)(lw_bridge_map + COUNTER_BASE);
	count = 0;
	for (i = 0; i < 31; i++){
		*init = 32;
		*init = i;
		while((short)*cycles < 64) {
	       usleep(1);
		}
		current = (short)*cycles & 63;
		count = count + current;
	}
	printf("IT TOOK %x CYCLES", count);
	// Unmap everything and close the /dev/mem file descriptor
	result = munmap(lw_bridge_map, HPS_TO_FPGA_LW_SPAN); 
	if(result < 0) {
		perror("devmem munmap");
		close(devmem_fd);
		exit(EXIT_FAILURE);
	}

	close(devmem_fd);
	exit(EXIT_SUCCESS);
}
