
/* =========================================================================
 *  MODULE:     list_class.C
 * =========================================================================
 * PURPOSE:     c++ list class methods
 * ========================================================================*/
#include <ListClass.h>

/*====================== Constructor/Destructor debug =====================*/

int mem_debug=0;
int alloc_cnt=0;

void set_mem_debug(int n)
{
	mem_debug=n;
}

void debug_malloc(const char *cls, const char *name, int size)
{
	alloc_cnt+=size;
	if(size)
		printf("\t%-15s %-12s : MALLOC %-5d : %d\n",cls,name,size,alloc_cnt);
	else
		printf("\t%-15s %-12s\n",cls,name);
}

void debug_free(const char *cls, const char *name, int size)
{
	alloc_cnt-=size;
	if(size)
		printf("\t%-15s %-12s : FREE   %-5d : %d\n",cls,name,size,alloc_cnt);
	else
		printf("\t%-15s %-12s\n",cls,name);
}


void debug_malloc(const char *cls, int id, int size)
{
	alloc_cnt+=size;
	if(size)
		printf("\t%-15s %-12x : MALLOC %-5d : %d\n",cls,id,size,alloc_cnt);
	else
		printf("\t%-15s %-12x\n",cls,id);
}

void debug_free(const char *cls, int id, int size)
{
	alloc_cnt-=size;
	if(size)
		printf("\t%-15s %-12x : FREE   %-5d : %d\n",cls,id,size,alloc_cnt);
	else
		printf("\t%-15s %-12x\n",cls,id);
}
