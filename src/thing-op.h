#ifndef _THING_OP_H
#define _THING_OP_H

#include <Arduino.h>
#include <Ethernet.h>

#ifdef __cplusplus
extern "C" {
#define restrict __restrict__ // C++ do not have standard restrict keyword as in C99
#endif

void thing_resp_portal_page(EthernetServer & server, const char *restrict method);
void thing_resp_things(EthernetServer & server, const char *restrict method);
void thing_resp_thing(EthernetServer & server, const char *restrict method);
void thing_proceed_properties(EthernetServer & server, EthernetClient & client, const char *restrict method, const char *path);
void thing_proceed_actions(EthernetServer & server, EthernetClient & client, const char *restrict method, const char *path);
void thing_proceed_events(EthernetServer & server, EthernetClient & client, const char *restrict method, const char *path);
void thing_resp_not_found(EthernetServer & server, const char *restrict method);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: _THING_OP_H */
