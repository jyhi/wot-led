#ifndef _HTML_HEADERS_H
#define _HTML_HEADERS_H

static const char html_header_100[] PROGMEM =
  "HTTP/1.1 100 Continue\r\n";

static const char html_header_200[] PROGMEM =
  "HTTP/1.1 200 OK";

// static const char html_header_201[] PROGMEM =
//   "HTTP/1.1 201 Created";

static const char html_header_204[] PROGMEM =
  "HTTP/1.1 204 No Content\r\n";

static const char html_header_400[] PROGMEM =
  "HTTP/1.1 400 Bad Request\r\n";

static const char html_header_404[] PROGMEM =
  "HTTP/1.1 404 Not Found\r\n";

static const char html_header_405[] PROGMEM =
  "HTTP/1.1 405 Method Not Allowed\r\n";

static const char html_header_500[] PROGMEM =
  "HTTP/1.1 500 Internal Server Error\r\n";

static const char html_header_content_html[] PROGMEM =
  "Content-Type: text/html; charset=UTF-8\r\n";

static const char html_header_content_json[] PROGMEM =
  "Content-Type: application/json; charset=UTF-8\r\n";

#endif /* end of include guard: _HTML_HEADERS_H */
