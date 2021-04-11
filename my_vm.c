#include "my_vm.h"

void* physMemory; 
void* physBitmap; 
void* virtBitmap;
pde_t* pgdir;

unsigned int offsetBits, level1Bits, level2Bits, numPages1, numPages2;
unsigned int offset_bitmask, level1_bitmask, level2_bitmask;

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
	
	// Set bitmasks for helper functions to separate virtual addresses
	offset_bitmask = (unsigned int)pow(2, offsetBits)-1;
	level1_bitmask = ((unsigned int)pow(2, level1Bits)-1) << (offsetBits+level2Bits);
	level2_bitmask = ((unsigned int)pow(2, level2Bits)-1) << offsetBits;


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

    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */

    return -1;
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_TLB(void *va) {

    /* Part 2: TLB lookup code here */

}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    double miss_rate = 0;	

    /*Part 2 Code here to calculate and print the TLB miss rate*/




    fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
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
	
	unsigned int offset = getOffset(va);
	unsigned int level1Index = getLevel1Index(va);
	unsigned int level2Index = getLevel2Index(va);
	
	unsigned int bitmapIndex = (unsigned int)va >> offsetBits;
	
	if(*((char*)virtBitmap + (bitmapIndex/8)) & (1 << bitmapIndex%8) == 0){ // memory has not be set to this address
		return NULL;
	}

	pde_t *page_dir = pgdir + level1Index;
	if(*page_dir == NULL){ // page directory entry does not exist
		return NULL;
	}
	pte_t *page_table = ((pte_t*) *page_dir)+level2Index;
	if(*page_table == NULL){ // page table entry does not exist
		return NULL;
	}
	
	pte_t *phys_page_addr = (pte_t*)(*page_table+offset);
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

    return -1;
}


/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) {
 
    //Use virtual address bitmap to find the next free page
}


/* Function responsible for allocating pages
and used by the benchmark
*/
void *a_malloc(unsigned int num_bytes) {

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */

   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */

    return NULL;
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void a_free(void *va, int size) {

    /* Part 1: Free the page table entries starting from this virtual address
     * (va). Also mark the pages free in the bitmap. Perform free only if the 
     * memory from "va" to va+size is valid.
     *
     * Part 2: Also, remove the translation from the TLB
     */
     
    
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




}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */




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

       
}



