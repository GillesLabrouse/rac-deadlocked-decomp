/* Complete256-byte pose-mask producer. NONMATCHING. */
#include "common.h"
static void mask_zero(void) {__asm__ volatile("viswr.w $vi0,($vi2)" : : : "memory");}
static void mask_one(void) {__asm__ volatile("viswr.w $vi1,($vi2)" : : : "memory");}
static void mask_next(void) {__asm__ volatile("viaddi $vi2,$vi2,1" : : : "memory");}
void func_L01_005893F0(s32 first,s32 count,const u8 *first_list,const u8 *second_list) {
    volatile u8 *record=(volatile u8 *)(0x70000000U+((u32)first<<6));
    const volatile s8 *cursor;s32 next,end=first+count;
    __asm__ volatile("ctc2.ni %0,$vi2\n\tctc2.ni %1,$vi1" : : "r"(first),"r"(1) : "memory");
    if(!first_list) {
        do {mask_zero();record[0x3D]=0;record+=64;++first;mask_next();}while(first!=end);
        record[0x3D]=1;mask_one();return;
    }
    cursor=(const volatile s8 *)(first_list+*(const volatile u16 *)first_list+4);next=*cursor++;
    do {
        if(next==first){next=*cursor++;record[0x3D]=0;mask_zero();}
        else {record[0x3D]=1;mask_one();}
        ++first;record+=64;mask_next();
    }while(first!=end);
    record[0x3D]=1;mask_one();
    if(!second_list)return;
    cursor=(const volatile s8 *)(second_list+*(const volatile u16 *)second_list+4);first=*cursor++;
    next=first;record=(volatile u8 *)(0x70000000U+((u32)first<<6));
    __asm__ volatile("ctc2.ni %0,$vi2" : : "r"(first) : "memory");
    do {
        if(next==first){next=*cursor++;record[0x3D]=0;mask_zero();}
        ++first;record+=64;mask_next();
    }while(next>=0);
}
