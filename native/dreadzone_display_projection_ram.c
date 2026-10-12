#include "dreadzone_display_projection_ram.h"
#include <fenv.h>
#include <string.h>

static void word(uint8_t *p,uint32_t offset,uint32_t value) {memcpy(p+offset,&value,4);}
static void real(uint8_t *p,uint32_t offset,float value) {memcpy(p+offset,&value,4);}
static float div32(float a,float b) {volatile float v=a/b;return v;}
static float mul32(float a,float b) {volatile float v=a*b;return v;}

/* Bounded RAM-only L01 5469A0 + genuine 546918 helper. Explicit descriptor
 * inputs are component-test inputs, not a proven live graphics startup.
 * No EE/VU/register/stack or platform outputs. Memory is exclusively owned.
 * The complete output areas are prepared locally before any publication. */
int dreadzone_display_projection_ram_005469A0(uint8_t *storage,uint32_t base,uint32_t bytes) {
    uint8_t staged[0x50],screen[0x26C];
    int16_t width,height;
    int32_t half_width,half_height;
    float w,h,invw,invh,fw,fh;
    if(!storage||!bytes||bytes>0x1000000U||base>0x7FFFFFFFU||bytes>0x80000000U-base)return -1;
    if(base<0x70004000U&&(uint64_t)base+bytes>0x70000000U)return -1;
    if(base>0x1D3F40U||(uint64_t)base+bytes<0x2400ACU)return -1;
    memcpy(&width,storage+(0x206B90U-base),2);
    memcpy(&height,storage+(0x206B92U-base),2);
    if(width<=0||height<=0||fegetround()!=FE_TONEAREST)return -2;
    memcpy(staged,storage+(0x1D3F40U-base),sizeof(staged));
    memcpy(screen,storage+(0x23FE40U-base),sizeof(screen));
    half_width=width/2;half_height=height/2;
    word(staged,0,(uint32_t)width);word(staged,4,(uint32_t)height);
    word(staged,8,(uint32_t)half_width);word(staged,12,(uint32_t)half_height);
    word(staged,0x10,(uint32_t)(2048-half_width)*16U);
    word(staged,0x14,(uint32_t)(2048-half_height)*16U);
    word(staged,0x18,(uint32_t)(2048+half_width)*16U);
    word(staged,0x1C,(uint32_t)(2048+half_height)*16U);
    w=(float)width;h=(float)height;
    invw=div32(1.f,w);invh=div32(1.f,h);
    fw=mul32(w,.5f);fh=mul32(h,.5f);
    real(staged,0x20,invw);real(staged,0x24,invh);
    real(staged,0x30,fw);real(staged,0x34,fh);
    word(staged,0x38,0);word(staged,0x3C,0);
    real(staged,0x40,invw);real(staged,0x44,invh);
    word(staged,0x48,0);word(staged,0x4C,0);
    real(screen,0xA0,32.f);real(screen,0xA4,1048576.f);
    word(screen,0xB0,0x3F1EB852U);
    word(screen,0x1A0,0x45000000U);word(screen,0x1A4,0x45000000U);
    real(screen,0x218,mul32(fw,4.f));real(screen,0x21C,mul32(fh,4.f));
    real(screen,0x22C,524288.f);real(screen,0x238,255.f);
    word(screen,0x244,(uint32_t)width-1);word(screen,0x24C,(uint32_t)height-1);
    word(screen,0x268,0);real(screen,0x210,fw);real(screen,0x214,fh);
    word(screen,0x228,0);word(screen,0x23C,0);word(screen,0x240,0);word(screen,0x248,0);
    memcpy(storage+(0x1D3F40U-base),staged,sizeof(staged));
    memcpy(storage+(0x23FE40U-base),screen,sizeof(screen));
    return 1;
}
