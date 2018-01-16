#include "elf.h"

#include "mem_physical.h"
#include "mem_virtual.h"

#include "debug.h"

/*
 * Load elf execution file exe to the virtual address space pmap.
 */
void ELFLoad(void* exePtr, uint32 pid)
{
	elfhdr *eh;
	proghdr *ph, *eph;
    //char* strtab;
	sechdr *sh;
    //sechdr *esh;
	uint32 exe = (uint32)exePtr;

	eh = (elfhdr*)exe;

	KERN_ASSERT(eh->e_magic == ELF_MAGIC);
	KERN_ASSERT(eh->e_shstrndx != ELF_SHN_UNDEF);

	sh = (sechdr*)((uint32) eh + eh->e_shoff);
	//esh = sh + eh->e_shnum;

	//strtab = (char*)(exe + sh[eh->e_shstrndx].sh_offset);
	KERN_ASSERT(sh[eh->e_shstrndx].sh_type == ELF_SHT_STRTAB);

	ph = (proghdr*)((uint32)eh + eh->e_phoff);
	eph = ph + eh->e_phnum;

	for (; ph < eph; ph++)
	{
		uint32 fa;
		uint32 va, zva, eva, perm;

		if (ph->p_type != ELF_PROG_LOAD)
			continue;

		fa = (uint32) eh + RoundDown(ph->p_offset, PAGESIZE);
		va = RoundDown(ph->p_va, PAGESIZE);
		zva = ph->p_va + ph->p_filesz;
		eva = RoundUp(ph->p_va + ph->p_memsz, PAGESIZE);

		perm = PTE_U | PTE_P;
		if (ph->p_flags & ELF_PROG_FLAG_WRITE)
			perm |= PTE_W;

		for (; va < eva; va += PAGESIZE, fa += PAGESIZE)
		{
            int err;
			AllocPage(pid, va, perm, &err);
            if (err) {
                KERN_PANIC("AllocPage failed on ELF load");
            }

			if (va < RoundDown(zva, PAGESIZE))
			{
				/* copy a complete page */
				PTCopyOut((void *) fa, pid, va, PAGESIZE);
			}
			else if (va < zva && ph->p_filesz)
			{
				/* copy a partial page */
				PTMemSet(pid, va, 0, PAGESIZE);
				PTCopyOut((void *) fa, pid, va, zva - va);
			}
			else
			{
				/* zero a page */
				PTMemSet(pid, va, 0, PAGESIZE);
			}
		}
	}

}

void* ELFEntry(void* exePtr)
{
	elfhdr* eh = (elfhdr*)exePtr;
	KERN_ASSERT(eh->e_magic == ELF_MAGIC);

	return (void*)eh->e_entry;
}
