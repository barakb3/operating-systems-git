
#include "os.h"

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn)
{
    uint64_t phys_level_base = pt, *virt_level_base;
    int index, level = 1, valid;
    do
    {
        virt_level_base = phys_to_virt(phys_level_base); /* get the base of the current level */
        index = (vpn >> ((5 - level) * 9)) & 0x1ff; /* get the corresponding symbol */
        phys_level_base = virt_level_base[index]; /* get the next pte */
        level++;
        valid = phys_level_base % 2; /* LSB of the pte is the valid bit*/
    } while (level <= 5 && valid == 1);

    if (ppn == NO_MAPPING) /* destroy */
    {
        if (valid == 1)
        /* it must hold that level == 6 and virt_level_base points to the level where the pages are */
        {
            virt_level_base[index] = 0;
        }
        /* otherwise, valid == 0 and we have nothing to destroy */
    }
    else /* create */
    {
        while (level <= 5) /* we had valid == 0 */
        {
            phys_level_base = alloc_page_frame();
            virt_level_base[index] = phys_level_base;
            virt_level_base = phys_to_virt(phys_level_base);
            index = (vpn >> ((5 - level) * 9)) & 0x1ff;
            level++;
        }
        /* when level == 6 update as follows: */
        virt_level_base[index] = (ppn << 12) | 1;
    }
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn)
{

    uint64_t phys_level_base = pt, *virt_level_base;
    int index, level = 1, valid;
    do
    {
        virt_level_base = phys_to_virt(phys_level_base); /* get the base of the current level */
        index = (vpn >> ((5 - level) * 9)) & 0x1ff; /* get the corresponding symbol */
        phys_level_base = virt_level_base[index]; /* get the next pte */
        level++;
        valid = phys_level_base % 2; /* LSB of the pte is the valid bit*/
    } while (level <= 5 && valid == 1);

    if (level == 6 && valid == 1)
    /* virt_level_base points to the level where the pages are and the physical address is valid */
    {
        return virt_level_base[index];
    }
    else /* create */
    {
        return NO_MAPPING;
    }
}