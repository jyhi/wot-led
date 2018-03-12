#include <Arduino.h>
#include <Ethernet.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BUFSIZE_LINE 64

int http_read_line(char *buffer, const size_t size, EthernetClient & client)
{
  if (!buffer || !size)
    return 0;

  char c = '\0';
  char *p_buffer = buffer;
  size_t counter = 0; // Prevent buffer overflow

  while (client.connected() && client.available() && counter < size - 1) {
    c = client.read();
    if (c != '\r' && c != '\n') {
      *p_buffer++ = c;
      ++counter;
    } else {
      // Meet newline, reading finished
      break;
    }
  }

  // Have a NUL at the end of string
  *p_buffer = '\0';

#ifdef DEBUG
  Serial.print(F(">| http_read_line: '"));
  Serial.print(buffer);
  Serial.println(F("'"));
#endif

  // Then we need to move the cursor to the next line
  while ((c = client.read()) && c != '\n') {}

  return 1;
}

void server_println_P(EthernetServer & server, const char *str)
{
  static char buffer[BUFSIZE_LINE] = {0};
  size_t chunks = strlen_P(str) > BUFSIZE_LINE ? strlen_P(str) / BUFSIZE_LINE : 1;

#ifdef DEBUG
  Serial.print(F("I| server_println_P: "));
  Serial.print(chunks);
  Serial.println(F(" chunk(s) to be sent"));
#endif

  for (size_t i = 0; i < chunks; i++) {
#ifdef DEBUG
    Serial.print(F("<| server_println_P: Chunk "));
    Serial.println(i);
#endif
    strlcpy_P(buffer, str, BUFSIZE_LINE);
    server.println(buffer);
  }

#ifdef DEBUG
  Serial.println(F("I| server_println_P: Data sent"));
#endif

  // Don't forget to free the buffer
  // free(buffer);
}

void ethernet_maintain(void)
{
  switch(Ethernet.maintain()) {
    case 1:
      // renew failed
      Serial.println(F("E| DHCP IP renewal failed!"));
      break;
    case 2:
      // renew success
      Serial.println(F("I| DHCP IP renewal success"));
      Serial.print(F("I| "));
      Serial.println(Ethernet.localIP());
      break;
    case 3:
      // rebind fail
      Serial.println(F("E| DHCP IP rebind failed!"));
      break;
    case 4:
      // rebind success
      Serial.println(F("I| DHCP IP rebind success"));
      Serial.print(F("I| "));
      Serial.println(Ethernet.localIP());
      break;
    case 0: // fall through
    default:
      // nothing happened
      break;
  }
}

#ifdef __cplusplus
}
#endif
