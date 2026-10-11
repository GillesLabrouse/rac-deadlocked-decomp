#include "dreadzone_actor_counter_step.h"
#include <string.h>
s32 dreadzone_actor_counter_step_0055D148(s32 *counter) {
    s32 value;
    if(!counter)return -1;
    memcpy(&value,counter,sizeof(value));
    if(!value)return 1;
    if(value<1)value=1;
    --value;
    memcpy(counter,&value,sizeof(value));
    return value>0?0:2;
}
