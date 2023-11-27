//===- muxpp.cpp - Port multiplex proxy -------------------------*- C++ -*-===//
//
/// \file
/// Support raw TCP/UDP/SCTP proxy with header.
//
// Author:  zxh
// Date:    2023/11/27 13:54:57
//===----------------------------------------------------------------------===//

#include <arpa/inet.h>
#include <netinet/in.h>

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

static void setLogger() {
  // format:
  //   I 20230927 11:28:32 1076340  Listen on TCP 127.0.0.1:5550
  // rotate: 10MB 3Counts
  spdlog::set_pattern("%^%l%$ %Y%m%d %H:%M:%S %t  %v");
}

static const uint16_t kListenPort = 4396;

static struct event_base *g_base;

static void readCallback(struct bufferevent *bev, void *arg) {
  struct evbuffer *src = bufferevent_get_input(bev);
  struct evbuffer *dst = bufferevent_get_output(bev);
  evbuffer_add_buffer(dst, src);
}

static void eventCallback(struct bufferevent *bev, short what, void *arg) { bufferevent_free(bev); }

static void newConnCallback(struct evconnlistener *, evutil_socket_t fd, struct sockaddr *addr,
                            int socklen, void *arg) {
  char buf[INET_ADDRSTRLEN];
  ::inet_ntop(AF_INET, &((struct sockaddr_in *)addr)->sin_addr.s_addr, buf, socklen);
  spdlog::info("Received new connection on {}:{}", buf,
               ntohs(((struct sockaddr_in *)addr)->sin_port));

  struct bufferevent *bev =
      bufferevent_socket_new(g_base, fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
  if (!bev) {
    ::close(fd);
    return;
  }

  bufferevent_setcb(bev, readCallback, NULL, eventCallback, NULL);
  bufferevent_enable(bev, EV_READ | EV_WRITE);
}

int main() {
  setLogger();

  spdlog::info("===== muxpp start =====");

  g_base = event_base_new();

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(kListenPort);
  addr.sin_addr.s_addr = htons(INADDR_ANY);

  if (!evconnlistener_new_bind(g_base, newConnCallback, NULL,
                               LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC | LEV_OPT_REUSEABLE,
                               -1, (struct sockaddr *)&addr, sizeof(addr)))
    spdlog::critical("Fatal to listen");

  spdlog::info("Listening on {}", kListenPort);

  event_base_dispatch(g_base);
}