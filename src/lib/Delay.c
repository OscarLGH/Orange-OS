#include "type.h"
#include "const.h"
/*======================================================================*
                               delay
 *======================================================================*/
PUBLIC void Delay(int milli_sec)
{
	int t = GetTicks();
	while(((GetTicks() - t) * 1000 / Hz) < milli_sec){}
}