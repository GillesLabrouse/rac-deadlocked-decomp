#ifndef DEADLOCKED_EQUIPMENT_UI_INTERNAL_H
#define DEADLOCKED_EQUIPMENT_UI_INTERNAL_H
/* Include common.h first so native and SN builds retain their scalar ABI.
 * Shared declarations keep all complete UI drafts on the same call contract. */

#ifdef DEADLOCKED_NATIVE
#define UI_R32(address) dreadzone_ui_read32(ctx,(u32)(address))
#define UI_R8(address) dreadzone_ui_read8(ctx,(u32)(address))
#define UI_W32(address,value) dreadzone_ui_write32(ctx,(u32)(address),(u32)(value))
#define UI_DEFINITION(item) func_L01_006BF200(&ctx->equipment->inventory,(item),0)
#define UI_ADDRESS(pointer) (pointer)
#define UI_POINTER(address) (address)
#define UI_ACTOR(index) dreadzone_ui_actor(ctx,(index))
#define UI_COPY(out,address,bytes) func_L01_0055D3B8((out),dreadzone_ui_data(ctx,(address),(bytes)),(bytes))
#else
typedef u32 *UiWordPointer;
#define UI_CTX
#define UI_ONLY void
#define UI_PASS
#define UI_ARG
#define UI_R32(address) (*(volatile u32 *)(address))
#define UI_R8(address) (*(volatile u8 *)(address))
#define UI_W32(address,value) (*(volatile u32 *)(address)=(u32)(value))
#define UI_DEFINITION(item) func_L01_006BF200((item),0)
#define UI_ADDRESS(pointer) ((u32)(pointer))
#define UI_POINTER(address) ((UiWordPointer)(address))
#define UI_ACTOR(index) ((u32)func_L01_006658C0(index))
#define UI_COPY(out,address,bytes) func_L01_0055D3B8((out),(const void *)(address),(bytes))
s32 func_L01_005D7558(s32,s32,s32);
s32 func_L01_005E93D8(s32,s32,s32);
UiWordPointer func_L01_005EA270(s32);
s32 func_L01_006BDDF0(s32);
s32 func_L01_005D89E8(s32);
s32 func_L01_005EA230(s32);
void func_L01_005E97D8(s32,s32,s32);
s32 func_L01_005D8A08(s32);
s32 func_L01_005D8A40(s32,s32);
s32 func_L01_005D8AE0(s32,s32);
void func_L01_005E9350(s32);
void func_L01_005EA6A8(void);
void func_L01_005F0A40(void);
const u8 *func_L01_006BF200(s32,s32);
u8 *func_L01_006658C0(s32);
s32 func_L01_00688578(u8 *,s32);
#endif
s32 func_L01_00652FD0(const void *,s32,s32,s32);
void func_L01_0055D3B8(void *,const void *,u32);
#endif
