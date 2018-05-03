
#ifndef __mydll_h__
#define __mydll_h__

typedef struct
{
	char *module_name;
	char *api_name;
	int param_count;
	char *my_api_name;
	int start_pos;
	char *friend_my_api_name;
}MYAPIINFO;

/*
typedef struct
{
	char dest_ip_addr[128];
	int iPort;
}SERVERINFO;


SERVERINFO *serverInfo;*/

char dest_ip_addr[128];
int iPort;


#endif
