#include "my_vm.h"
#include "string.h"

void* physMemory; 
void* physBitmap; 
void* virtBitmap;
pde_t* pgdir;

unsigned int offsetBits, level1Bits, level2Bits, numPages1, numPages2;
unsigned int offset_bitmask, level1_bitmask, level2_bitmask;
unsigned int is_init = 0;
unsigned int freeing = 0;

/*
Function returns offset for virtual address
*/
unsigned int getOffset(void* va){
	return ((unsigned int)va) & offset_bitmask;
}
/*
Function returns index of page directory for virtual address
*/
unsigned int getLevel1Index(void* va){
	return (((unsigned int)va) & level1_bitmask) >> (offsetBits+level1Bits);
}
/*
Function returns index of page table for virtual address
*/
unsigned int getLevel2Index(void* va){
	return (((unsigned int)va) & level2_bitmask) >> offsetBits;
}
/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {
	if(pthread_mutex_init(&lock, NULL) != 0)
		puts("Failed to initialize mutex");
	
	pthread_mutex_lock(&lock);
    is_init = 1;
    offsetBits = getLog(PGSIZE);
    int pageBits = 32 - offsetBits;

    if (pageBits % 2 != 0) {
        level1Bits = (pageBits / 2) + 1;
        level2Bits = (pageBits / 2);
    } else {
        level1Bits = (pageBits / 2);
        level2Bits = (pageBits / 2);
    }

    numPages2 = pow(2, pageBits);
	numPages1 = pow(2, level1Bits);
	unsigned int numPhysPages = MEMSIZE / PGSIZE;
    //TODO: allocate numpages2
    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating
    physMemory = malloc(MEMSIZE);

	pgdir = (pde_t*)calloc(numPages1, sizeof(pde_t));
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them
	physBitmap = (void*)malloc(ceil((double)numPhysPages/8));
	memset(physBitmap, 0, ceil((double)numPhysPages/8));
	virtBitmap = (void*)malloc(ceil((double)numPages2/8));
	memset(virtBitmap, 0, ceil((double)numPages2/8));
	
	tlb_store.bitmap = (void*)malloc(ceil((double)TLB_ENTRIES/8));
	memset(tlb_store.bitmap, 0, ceil((double)TLB_ENTRIES/8));
	tlb_store.miss = 0;
	tlb_store.hit = 0;
	tlb_store.curr = 0;

	// Set bitmasks for helper functions to separate virtual addresses
	offset_bitmask = (unsigned int)pow(2, offsetBits)-1;
	level1_bitmask = ((unsigned int)pow(2, level1Bits)-1) << (offsetBits+level2Bits);
	level2_bitmask = ((unsigned int)pow(2, level2Bits)-1) << offsetBits;

	pthread_mutex_unlock(&lock);
}
int getLog(int val) {
    int ans = 0;
    while(val >>= 1) ++ans;

    return ans;
}
/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */
int
add_TLB(void *va, void *pa)
{

	for(int i = 0; i < TLB_ENTRIES; i++) {
		char *c = tlb_store.bitmap + (i/8);
		if ((*c & (int)pow(2, i % 8)) == 1 && tlb_store.va[i] == ((unsigned int) va >> offsetBits)) {
			puts("This virtual address is already in the TLB");
			return -1;
		}
	}
	tlb_store.miss++;
	//See if there are any empty entries in the TLB
	for(int i = 0; i < TLB_ENTRIES; i++) {
		char *c = tlb_store.bitmap + (i/8);
		if((*c & (int)pow(2, i % 8)) == 0) {
			tlb_store.va[i] = ((unsigned int) va >> offsetBits);
			tlb_store.pa[i] =  (pte_t) pa;

			
    		char bit = 1 << (i % 8);
    		*c |= bit;
			return 0;
		}
	}
	tlb_store.va[tlb_store.curr] = ((unsigned int) va >> offsetBits);
	tlb_store.pa[tlb_store.curr] =  (pte_t) pa;
	
	tlb_store.curr = (tlb_store.curr + 1) % TLB_ENTRIES;
	

    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */
	
    return 0;
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_TLB(void *va) {
	for(int i = 0; i < TLB_ENTRIES; i++) {
		char *c = tlb_store.bitmap + (i/8);
		if((*c & (int)pow(2, i % 8)) == 0) 
			continue;
		if (tlb_store.va[i] == ((unsigned int) va >> offsetBits)) {
			tlb_store.hit++;
			return (pte_t *)tlb_store.pa[i]; 
		}
	}
	// puts("not found in TLB");
	return NULL; //Not in TLB
}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    double miss_rate = 0;	
	double hit_rate = 0;

    /*Part 2 Code here to calculate and print the TLB miss rate*/
	double misses = tlb_store.miss;
	double hits = tlb_store.hit;

	miss_rate = misses/(misses + hits);
	hit_rate = hits/(misses + hits);

    fprintf(stderr, "TLB miss rate %lf \nTLB hit rate %lf \nTotal hits: %lf\nTotal misses: %lf\n", miss_rate, hit_rate, hits, misses);
}



/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t *translate(pde_t *pgdir, void *va) {
    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    *
    * Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */
	pte_t* phys_addr = check_TLB(va);
	if(phys_addr != NULL)
		return (pte_t*)((unsigned int)phys_addr + getOffset(va));
	unsigned int offset = getOffset(va);
	unsigned int level1Index = getLevel1Index(va);
	unsigned int level2Index = getLevel2Index(va);
	
	unsigned int bitmapIndex = (unsigned int)va >> offsetBits;
	
	if(*((char*)virtBitmap + (bitmapIndex / 8)) & (1 << bitmapIndex % 8) == 0){ // memory has not been set to this address
		puts("1 \n");
		return NULL;
	}


	pde_t *page_dir = pgdir + level1Index;
	if(*page_dir == 0){ // page directory entry does not exist
		puts("2 \n");
		return NULL;
	}
	pte_t *page_table = ((pte_t*) *page_dir)+level2Index;
	if(*page_table == 0){ // page table entry does not exist
		puts("3 \n");
		return NULL;
	}
	
	pte_t *phys_page_addr = (pte_t*)(*page_table+offset);
	if(freeing) { // TLB check missed if at this point. add_TLB() adds 1 to misses, so must do this here
		tlb_store.miss++;
	} else {
		add_TLB(va, (pte_t*)*page_table);
	}
	return phys_page_addr;
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int
page_map(pde_t *pgdir, void *va, void *pa)
{
    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */
	unsigned int offset = getOffset(va);
	unsigned int level1Index = getLevel1Index(va);
	unsigned int level2Index = getLevel2Index(va);
	
	unsigned int bitmapIndex = (unsigned int)va >> offsetBits;
	if((*((char*)virtBitmap + (bitmapIndex/8)) & (1 << bitmapIndex%8)) == 1){ // memory has already been set to this address
		return -1;
	}

	pde_t *page_dir = pgdir + level1Index;
	if(*page_dir == 0){ // page directory entry does not exist
		*page_dir = (pde_t) calloc(pow(2, level2Bits), sizeof(pte_t));
	}
	pte_t *page_table = ((pte_t*) *page_dir)+level2Index;

	*page_table = (pte_t) pa;
	return 0;	
}

/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) {
 	unsigned int i;
	unsigned int j;
	unsigned int currIndex = -1;
	unsigned int currOffset = -1;
	unsigned int count = 0;
	for(i = 0; i< ceil((double)numPages2/8); i++){
		char* c = (char*) virtBitmap + i;
		char curr = *c;
		for(j = 0; j<8; j++){
			if((curr&(int)pow(2, j)) == 0){
				if(currIndex == -1){
					currIndex = i;
					currOffset = j;
				}
				count++;
				if(count == num_pages){
					return (void*)((currIndex*8 + currOffset) << offsetBits);
				}
			} else {
				currIndex = -1;
				currOffset = -1;
				count = 0;
			}
		}
	}
	return NULL;
    //Use virtual address bitmap to find the next free page
}

/* Function that gets array of all needed physical page addresses
*/
void **get_physical_memory(int num_pages) {
	unsigned int i;
	unsigned int j;
	unsigned int count = 0;
	void** arr = (void*) malloc(sizeof(void*) *num_pages);
	for(i = 0; i< ceil((double)numPages2/8); i++){
		char* c = (char*) physBitmap + i;
		char curr = *c;
		for(j = 0; j<8; j++){

			if((curr & (int)pow(2, j)) == 0){
				arr[count] = (void*)(((i*8 + j) << offsetBits)+physMemory);
				count++;

				if(count == num_pages){
					return (void*)(arr);
				}
			}
		}
	}
	return NULL;
}

/* Function responsible for allocating pages
and used by the benchmark
*/
void *a_malloc(unsigned int num_bytes) {

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */

	if(!is_init) set_physical_mem();
	pthread_mutex_lock(&lock);
   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */
	
	int num_pages = (int)ceil((double)num_bytes / (double)PGSIZE);

	void *va = get_next_avail(num_pages);
	
	void* *physArr = get_physical_memory(num_pages);

	if(physArr == NULL) {
		printf("not able to get physical memory\n");
		pthread_mutex_unlock(&lock);
		return NULL;
	}
	for(int i = 0; i<num_pages; i++){
		//printf("%d ", i);
		int temp = page_map(pgdir, va+i*(int)(pow(2, offsetBits)), physArr[i]);
		if(temp == -1) {
			//printf("bad mapping in malloc\n");
		} else {
			unsigned int virtIndex = ((unsigned int)va >> offsetBits)+i;
			char* c = (char*)virtBitmap + (virtIndex/8);
			*c = *c | (1 << (virtIndex%8));
			unsigned int physIndex = ((unsigned int)physArr[i] - (unsigned int)physMemory) >> offsetBits;
			c = (char*)physBitmap + (physIndex/8);
			*c = *c | (1 << (physIndex%8));
		}
	}
	pthread_mutex_unlock(&lock);
    return va;
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void a_free(void *va, int size) {
	 /* Part 1: Free the page table entries starting from this virtual address
     * (va). Also mark the pages free in the bitmap. Perform free only if the 
     * memory from "va" to va+size is valid.
     */

	pthread_mutex_lock(&lock);
	int num_pages = (int)ceil((double) size/(double)PGSIZE);
	int i;
	for(i = 0; i < num_pages; i++) {
		unsigned int virtIndex = ((unsigned int) va >> offsetBits) + i;
		char* c = (char*) virtBitmap + (virtIndex / 8);
		if((*c & (int)pow(2, virtIndex % 8)) == '0') {
			puts("Bad call to free");
			pthread_mutex_unlock(&lock);
			return;
		}
	}

	for(i = 0; i < num_pages; i++) {
		unsigned int virtIndex = ((unsigned int) va >> offsetBits) + i;
		char* c = (char*) virtBitmap + (virtIndex / 8);
		*c &= ~(1 << virtIndex%8);
		unsigned int pa = (unsigned int)translate(pgdir, (void*)(virtIndex << offsetBits));
		unsigned int physIndex = (pa - (unsigned int)physMemory) >> offsetBits;
		c = (char*)physBitmap + (physIndex/8);
		*c &= ~(1 << physIndex%8);
	}

	/*
     * Part 2: Also, remove the translation from the TLB
     */
	for(int j = 0; j < num_pages; j++) {
		unsigned int temp_va = ((unsigned int) va >> offsetBits) + j;
		for(int i = 0; i < TLB_ENTRIES; i++) {
			char *c = tlb_store.bitmap + (i/8);
			if((*c & (int)pow(2, i % 8)) == 0) 
				continue;
			if (tlb_store.va[i] == temp_va) {
				tlb_store.va[i] = 0;
				tlb_store.pa[i] = 0;
				char bit = 1 << (i % 8);
				*c &= ~bit;
			}
		}
	}
    pthread_mutex_unlock(&lock);
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
*/
void put_value(void *va, void *val, int size) {

    /* HINT: Using the virtual address and translate(), find the physical page. Copy
     * the contents of "val" to a physical page. NOTE: The "size" value can be larger 
     * than one page. Therefore, you may have to find multiple pages using translate()
     * function.
     */
	pthread_mutex_lock(&lock);
	int num_pages = (int)ceil((double)size / (double)PGSIZE);
	int size_left = size;
	for(int i = 0; i<num_pages; i++){
		int to_set = size_left < PGSIZE ? size_left : PGSIZE;
		char* pa = (char*)translate(pgdir, va+i*(int)(pow(2, offsetBits)));
		if(pa==NULL){
			printf("failed to translate in put_value\n");
		}

		for(int j = 0; j<to_set; j++){
			*(pa+j) = *((char*)val+j+i*PGSIZE);
		}
		size_left -= to_set;
	}
	pthread_mutex_unlock(&lock);
}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {
    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */
   	pthread_mutex_lock(&lock);
	int num_pages = (int)ceil((double)size / (double)PGSIZE);
	int size_left = size;
	for(int i = 0; i<num_pages; i++){
		int to_set = size_left < PGSIZE ? size_left : PGSIZE;
		char* pa = (char*)translate(pgdir, va+i*(int)(pow(2, offsetBits)));
		if(pa==NULL){
			printf("failed to translate in get_value\n");
		}
		for(int j = 0; j<to_set; j++){
			*((char*)val+j+i*PGSIZE) = *(pa+j);
		}
		size_left -= to_set;
	}
	pthread_mutex_unlock(&lock);
}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void mat_mult(void *mat1, void *mat2, int size, void *answer) {
    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
     * matrix accessed. Similar to the code in test.c, you will use get_value() to
     * load each element and perform multiplication. Take a look at test.c! In addition to 
     * getting the values from two matrices, you will perform multiplication and 
     * store the result to the "answer array"
     */
	int valM1 = 0;
	int valM2 = 0;
	int sum;
	int i, j, k;
	for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
			sum = 0;
			for(k = 0; k < size; k++) {
				int address_a = (unsigned int)mat1 + ((i * size * sizeof(int)) + (k * sizeof(int)));
				int address_b = (unsigned int)mat2 + ((k * size * sizeof(int)) + (j * sizeof(int)));
				get_value((void *)address_a, &valM1, sizeof(int));
				get_value((void *)address_b, &valM2, sizeof(int));
				sum += (valM1 * valM2);
			}
			put_value((int*) answer + ((i*size)+j), &sum, sizeof(int));
        }
    }   
}



