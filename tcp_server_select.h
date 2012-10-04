#ifndef __tcp_server_select_h__
#define tcp_server_select

int manage_new_connection(int, int [], int);
int find_empty_slot(int[], unsigned int);
int find_max(int [], unsigned int);


#endif /* tcp_server_select */
