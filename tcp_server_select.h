#ifndef __tcp_server_select_h__
#define tcp_server_select

int handle_reading_slot(int);
int listen_newsocket(int);
int accept_new_connection(int, int [], int);
int find_empty_slot(int[], unsigned int);
int find_max(int [], unsigned int);


#endif /* tcp_server_select */
