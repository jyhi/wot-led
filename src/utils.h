#ifndef _UTILS_H
#define _UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

int http_read_line(char *buffer, const size_t size, EthernetClient & client);
void server_println_P(EthernetServer & server, const char *str);
void ethernet_maintain(void);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: _UTILS_H */
