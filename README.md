# MemoryManagement
User Level Memory Management - Design and Implement a Virtual Memory System and TLB

The goal of the project is to implement “a_malloc()” (Amazing malloc), which will return a virtual address that maps to a physical page. 
For simplicity, use a 32-bit address space that can support 4GB address space. 

Keep track of which physical pages are already allocated and which pages are free; use a virtual and physical page bitmap that 
represents a page.

In this part, you will implement a direct-mapped TLB. Remember that a TLB caches virtual page number to physical address. This part 
cannot be completed unless Part 1 is correctly implemented.

Initialize a direct-mapped TLB when initializing a page table. For any new page that gets allocated, no translation would exist in the 
TLB. So, after you add a new page table translation entry, also add a translation to the TLB by implementing add_TLB(). Before performing
a translation (in translate()), lookup the TLB to check if virtual to physical page translation exists. If the translation exists, you 
do not have to walk through the page table for performing translation (as done in Part 1). You must implement check_TLB() function to 
check the presence of a translation in the TLB.

If a translation does not exist in the TLB, check whether the page table has a translation (using your part 1). If a translation 
exists in the page table, then you could simply add a new virtual to physical page translation in the TLB using the function add_TLB()
