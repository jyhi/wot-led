#include <Arduino.h>
#include <Ethernet.h>
#include <SD.h>
#include <ArduinoJson.h>

#include <avr/wdt.h>

#include "thing-def.h"
#include "thing-op.h"
#include "html_headers.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#define restrict __restrict__ // C++ do not have standard restrict keyword as in C99
#endif

void thing_reboot(void) {
  wdt_enable(WDTO_15MS);
  for (;;) {}
}

void thing_resp_portal_page(EthernetServer & server, const char *restrict method)
{
  if (!(strcasecmp_P(method, PSTR("GET")) == 0)) {
#ifdef DEBUG
    Serial.println(F("W| thing_resp_portal_page: unsupported method"));
    Serial.println(F("<| thing_resp_portal_page: send 405 back"));
#endif
    server_println_P(server, html_header_405);
    return;
  }

  File f = SD.open(F("/index.htm"), FILE_READ);
  if (!f) {
#ifdef DEBUG
    Serial.println(F("E| thing_resp_portal_page: failed to open index.htm"));
#endif
    return;
  }

#ifdef DEBUG
  Serial.println(F("<| thing_resp_portal_page: sending index.htm"));
#endif

  server_println_P(server, html_header_200);
  server_println_P(server, html_header_content_html);
  while (f.available())
    server.write(f.read());

#ifdef DEBUG
  Serial.println(F("<| thing_resp_portal_page: sent index.htm"));
#endif

  f.close();
}

void thing_resp_things(EthernetServer & server, const char *restrict method)
{
  if (!(strcasecmp_P(method, PSTR("GET")) == 0)) {
#ifdef DEBUG
    Serial.println(F("W| thing_resp_things: unsupported method"));
    Serial.println(F("<| thing_resp_things: send 405 back"));
#endif
    server_println_P(server, html_header_405);
    return;
  }

  File f = SD.open(F("/thing.jsn"), FILE_READ);
  if (!f) {
#ifdef DEBUG
    Serial.println(F("E| thing_resp_things: failed to open thing.jsn"));
#endif
    return;
  }

#ifdef DEBUG
    Serial.println(F("<| thing_resp_things: sending thing.jsn"));
#endif

  server_println_P(server, html_header_200);
  server_println_P(server, html_header_content_json);
  server.println('[');
  while (f.available())
    server.write(f.read());
  server.println(']');

#ifdef DEBUG
    Serial.println(F("<| thing_resp_things: sent thing.jsn"));
#endif

  f.close();
}

void thing_resp_thing(EthernetServer & server, const char *restrict method)
{
  if (!(strcasecmp_P(method, PSTR("GET")) == 0)) {
#ifdef DEBUG
    Serial.println(F("W| thing_resp_thing: unsupported method"));
    Serial.println(F("<| thing_resp_thing: send 405 back"));
#endif
    server_println_P(server, html_header_405);
    return;
  }

  File f = SD.open(F("/thing.jsn"), FILE_READ);
  if (!f) {
#ifdef DEBUG
    Serial.println(F("E| thing_resp_thing: failed to open thing.jsn"));
#endif
    return;
  }

#ifdef DEBUG
    Serial.println(F("<| thing_resp_thing: sending thing.jsn"));
#endif

  server_println_P(server, html_header_200);
  server_println_P(server, html_header_content_json);
  while (f.available())
    server.write(f.read());

#ifdef DEBUG
    Serial.println(F("<| thing_resp_thing: sent thing.jsn"));
#endif

  f.close();
}

void thing_proceed_properties(EthernetServer & server, EthernetClient & client, const char *restrict method, const char *path)
{
  // Shrink the URL to the rest of property ('/' is prefixed)
  const char *p_property_url = path + strlen_P(PSTR("/things/" THING_NAME "/properties"));
  StaticJsonBuffer<32> json_buffer; // Buffer used by ArduinoJson

  // NOTE: IMPLEMENTATION STARTS HERE
  if (strcasecmp_P(p_property_url, PSTR("/on")) == 0) { // Property "on"
    if (strcasecmp_P(method, PSTR("GET")) == 0) { // Getting property detail
      File f = SD.open(F("/property/on.jsn"), FILE_READ);
      if (!f) {
#ifdef DEBUG
        Serial.println(F("W| thing_proceed_properties: on.jsn not found"));
        Serial.println(F("<| thing_proceed_properties: send 500 back"));
#endif
        server_println_P(server, html_header_500);
        return;
      }

      JsonObject & j_on = json_buffer.parseObject(f);
      f.close();

      if (!j_on.success()) {
#ifdef DEBUG
        Serial.println(F("W| thing_proceed_properties: on.jsn parsing error"));
        Serial.println(F("<| thing_proceed_properties: send 500 back"));
#endif
        server_println_P(server, html_header_500);
        return;
      }

      // Alter JSON according to status first
      j_on["on"] = (bool)digitalRead(LED_PIN);

      // Send it back
      server_println_P(server, html_header_200);
      server_println_P(server, html_header_content_json);
      j_on.printTo(server);
      server.println();

    } else if (strcasecmp_P(method, PSTR("PUT")) == 0) { // Altering property detail
      JsonObject & j_on = json_buffer.parseObject(client);
      if (!j_on.success()) {
#ifdef DEBUG
        Serial.println(F("W| thing_proceed_properties: request JSON parsing error"));
        Serial.println(F("<| thing_proceed_properties: send 500 back"));
#endif
        server_println_P(server, html_header_500);
        return;
      }

      // Check if properties we need exist
      if (!j_on.containsKey(F("on"))) {
#ifdef DEBUG
        Serial.println(F("W| thing_proceed_properties: corrupted JSON"));
        Serial.println(F("<| thing_proceed_properties: send 400 back"));
#endif
        server_println_P(server, html_header_400);
        return;
      }

      // Apply requested properties
      digitalWrite(LED_PIN, j_on["on"]);

      // Send 200 back
      server_println_P(server, html_header_200);
      server.println();

    } else { // Unsupported method
#ifdef DEBUG
      Serial.println(F("W| thing_proceed_properties: unsupported method"));
      Serial.println(F("<| thing_proceed_properties: send 405 back"));
#endif
      server_println_P(server, html_header_405);
      return;
    }
  } else { // Unknown property
    thing_resp_not_found(server, method);
  }
}

void thing_proceed_actions(EthernetServer & server, EthernetClient & client, const char *restrict method, const char *path)
{
  // Shrink the URL to the rest of property ('/' is prefixed)
  const char *p_action_url = path + strlen_P(PSTR("/things/" THING_NAME "/actions"));
  StaticJsonBuffer<32> json_buffer; // Buffer used by ArduinoJson

  // NOTE: IMPLEMENTATION STARTS HERE
  if (strcasecmp_P(p_action_url, PSTR("/")) == 0 || strcasecmp_P(p_action_url, PSTR("")) == 0) {
    if (strcasecmp_P(method, PSTR("GET")) == 0) { // Get a list of actions
      // NOTE: I don't want to implement this here
      server_println_P(server, html_header_204);

    } else if (strcasecmp_P(method, PSTR("POST")) == 0) { // Action request
      JsonObject & j_reboot = json_buffer.parseObject(client);
      if (!j_reboot.success()) {
#ifdef DEBUG
        Serial.println(F("W| thing_proceed_actions: request JSON parsing error"));
        Serial.println(F("<| thing_proceed_actions: send 500 back"));
#endif
        server_println_P(server, html_header_500);
        return;
      }

      // Check if action we expect exist
      if (!j_reboot.containsKey(F("name"))) {
#ifdef DEBUG
        Serial.println(F("W| thing_proceed_actions: corrupted JSON"));
        Serial.println(F("<| thing_proceed_actions: send 400 back"));
#endif
        server_println_P(server, html_header_400);
        return;
      }

      const char *action_name = j_reboot["name"];

      if (strcmp_P(action_name, PSTR("reboot")) == 0) { // Reboot
        // First send acknowledgement
        // TODO: XXX: I can't be bothered to generate a UUID here
        server_println_P(server, html_header_204);

        // Close connection
        client.stop();

#ifdef DEBUG
        Serial.println(F("I| System is going down!"));
#endif

        thing_reboot();
        // NOTE: THIS LINE NEVER REACHED.

      } else { // Unknown name of action
#ifdef DEBUG
        Serial.println(F("W| thing_proceed_actions: unknown action name"));
        Serial.println(F("<| thing_proceed_actions: send 400 back"));
#endif
        server_println_P(server, html_header_400);
      }

    } else { // Unsupported method
#ifdef DEBUG
      Serial.println(F("W| thing_proceed_actions: unsupported method"));
      Serial.println(F("<| thing_proceed_actions: send 405 back"));
#endif
      server_println_P(server, html_header_405);
    }
  } else { // Action operation
    //
  }
}

void thing_proceed_events(EthernetServer & server, EthernetClient & client, const char *restrict method, const char *path)
{
  if (!(strcasecmp_P(method, PSTR("GET")) == 0)) {
#ifdef DEBUG
    Serial.println(F("W| thing_proceed_events: unsupported method"));
    Serial.println(F("<| thing_proceed_events: send 405 back"));
#endif
    server_println_P(server, html_header_405);
    return;
  }

  // Shrink the URL to the rest of property ('/' is prefixed)
  const char *p_event_url = path + strlen_P(PSTR("/things/" THING_NAME "/events"));

  // NOTE: IMPLEMENTATION STARTS HERE
}

void thing_resp_not_found(EthernetServer & server, const char *restrict method)
{
  // I don't care whatever method it is

#ifdef DEBUG
  Serial.println(F("<| thing_resp_not_found: send 404 back"));
#endif

  server_println_P(server, html_header_404);
  server.println();

#ifdef DEBUG
  Serial.println(F("<| thing_resp_not_found: sent 404 header"));
#endif
}

#ifdef __cplusplus
}
#endif
