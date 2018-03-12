#include <Arduino.h>
#include <Ethernet.h>
#include <SD.h>
#include <stdint.h>

#ifdef USE_MDNS
#include <ArduinoMDNS.h>
#endif

#include "thing-def.h"
#include "html_headers.h"
#include "thing-op.h"
#include "utils.h"

// DEBUG information include _DEBUG's
#ifdef DEBUG
#define _DEBUG
#endif

#ifdef _DEBUG
#ifndef BAUD
#define BAUD 115200
#endif
#endif

// Port number that the server listens on
#ifndef PORT
#define PORT 80
#endif

// Buffer sizes
#define BUFSIZE_METHOD       7
#define BUFSIZE_METHOD_SCANF "6"
#define BUFSIZE_PATH         75
#define BUFSIZE_PATH_SCANF   "74"
#define BUFSIZE_LINE         81
#define BUFSIZE_LINE_SCANF   "80"

#ifdef USE_MDNS
EthernetUDP udp; // UDP class used by mDNS
MDNS mdns(udp);  // MDNS class
#endif

// MAC address of the Thing
static uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// Main TCP server
EthernetServer server(PORT);

void setup(void)
{
#ifdef _DEBUG
  Serial.begin(BAUD);
  while (!Serial) {}

  Serial.println(F("I| Starting board..."));
#endif

  // Pin init
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

#ifdef DEBUG
  Serial.print(F("I| Configuring SD card at pin "));
  Serial.println(SD_SS);
#endif

  // SD Card Init
  if (!SD.begin(SD_SS)) {
#ifdef _DEBUG
    Serial.println(F("W| SD card init fail! Halting..."));
#endif
    // What can I do?
    for (;;) {}
  }

#ifdef DEBUG
  Serial.println(F("I| SD card init succeed"));
  Serial.println(F("I| Configuring network..."));
#endif

#ifdef USE_DHCP
  Ethernet.begin(mac);
#else
  static const byte _ip[] = {192, 168, 1, 141};
  static const byte _dns[] = {192, 168, 1, 1};
  static const byte _gateway[] = {192, 168, 1, 1};
  static const byte _subnet[] = {255, 255, 255, 0};
  Ethernet.begin(mac, _ip, _dns, _gateway, _subnet);
#endif

#ifdef DEBUG
  Serial.print(F("I| MAC: "));
  Serial.print(mac[0], HEX);
  for (int i = 1; i < 6; i++) {
    Serial.print(F(":"));
    Serial.print(mac[i], HEX);
  }
  Serial.println();
#ifdef USE_DHCP
  Serial.println(F("<| DHCP..."));
#endif
#endif

  server.begin();
  IPAddress ip = Ethernet.localIP();

#ifdef DEBUG
#ifdef USE_DHCP
  Serial.print(F(">| DHCP get: "));
  Serial.print(ip);
  Serial.println();
#endif
#ifdef USE_MDNS
  Serial.println(F("I| Starting mDNS service..."));
#endif
#endif

#ifdef USE_MDNS
  mdns.begin(ip, THING_NAME);

#ifdef DEBUG
  Serial.println(F("I| Registering mDNS service record..."));
#endif

  mdns.addServiceRecord(THING_DESCRIPTION "._http", PORT, MDNSServiceTCP);
#endif

#ifdef _DEBUG
  Serial.println(F("I| OK, service listening on:"));
#ifdef USE_MDNS
  Serial.print  (F("I|   "));
  Serial.print  (THING_NAME);
  Serial.print  (F(".local:"));
  Serial.println(PORT);
#endif
  Serial.print  (F("I|   "));
  Serial.print  (ip);
  Serial.print  (F(":"));
  Serial.println(PORT);
#endif
}

void loop(void)
{
#ifdef USE_MDNS
  mdns.run();
#endif

#ifdef USE_DHCP
  ethernet_maintain();
#endif

  EthernetClient client = server.available();
  if (!client)
    return;

#ifdef DEBUG
  Serial.println(F(">| New connection"));
#endif

  // For return value checking
  int r = 0;

  // Allocate memory for HTTP method, HTTP path, and line buffer
  // make it static, no more overlapping bugs
  static char method[BUFSIZE_METHOD] = {0};
  static char path[BUFSIZE_PATH]     = {0};
  static char linebuf[BUFSIZE_LINE]  = {0};

  // Read the first line of request to obtain HTTP method and path
  // <METHOD> <PATH> <HTTP_VERSION>
  http_read_line(linebuf, BUFSIZE_LINE, client);
  r = sscanf_P(
    linebuf,
    PSTR("%" BUFSIZE_METHOD_SCANF "s %" BUFSIZE_PATH_SCANF "s"),
    method, path
  );
  if (r != 2) {
    // And since we know insufficient information, we can only stop here
#ifdef DEBUG
    Serial.print(F("W| HTTP req parsing: sscanf returns "));
    Serial.println(r);
    Serial.println(F("X| Can do nothing, closing connection"));
#endif
    client.stop();
    // free(linebuf);
    // free(path);
    // free(method);
    return;
  }

#ifdef DEBUG
  Serial.print(F("I| HTTP Method: "));
  Serial.println(method);
  Serial.print(F("I| Request Path: "));
  Serial.println(path);
#endif

  // Parse the whole HTTP header, looking for:
  // - Accept: to see if the client wants API or UI (not checking now)
  // - Content-Type: to see what is in the payload (not checking it now)
  // - Content-Length: to see how long on earth is the payload (not checking it now)
  // Memory sucks.
  // NOTE: However, we should pass the header first!
  while (http_read_line(linebuf, BUFSIZE_LINE, client) && strcmp_P(linebuf, PSTR("")) != 0) {}

  // Check path to determine what to do next
  if (strcasecmp_P(path, PSTR("/")) == 0) {
    // '/' -> Device portal page
    // thing_resp_portal_page(server, method);
    thing_resp_thing(server, method);
  } else if (strcasecmp_P(path, PSTR("/things")) == 0) {
    // '/things' -> Things resource (3.5)
    thing_resp_things(server, method);
  } else if (strcasecmp_P(path, PSTR("/things/" THING_NAME)) == 0) {
    // '/things/<name>' -> Thing resource (3.1)
    thing_resp_thing(server, method);
  } else if (strncasecmp_P(path, PSTR("/things/" THING_NAME "/properties"), strlen_P(PSTR("/things/" THING_NAME "/properties"))) == 0) {
    // '/things/<name>/properties(/...)' -> This does not make sense (3.2)
    // Sub APIs processed in the function
    // NOTE: PUT is only supported in properties
    thing_proceed_properties(server, client, method, path);
  } else if (strncasecmp_P(path, PSTR("/things/" THING_NAME "/actions"), strlen_P(PSTR("/things/" THING_NAME "/actions"))) == 0) {
    // '/things/<name>/actions(/...)' -> Actions resource (3.3)
    // Sub APIs processed in the function
    // NOTE: POST is only supported in actions
    thing_proceed_actions(server, client, method, path);
  } else if (strncasecmp_P(path, PSTR("/things/" THING_NAME "/events"), strlen_P(PSTR("/things/" THING_NAME "/events")) )== 0) {
    // '/things/<name>/events(/...)' -> Events resource (3.4)
    // Sub APIs processed in the function
    thing_proceed_events(server, client, method, path);
  } else {
    // No such path
    thing_resp_not_found(server, method);
  }

#ifdef DEBUG
  Serial.println(F("X| Closing connection"));
#endif

  client.stop();
}
