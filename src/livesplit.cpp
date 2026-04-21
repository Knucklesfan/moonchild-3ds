#include "livesplit.hpp"

#include <cstdio>
#include <string>

#include "zed_net.h"

using std::string;

static zed_net_socket_t g_socket;
static bool g_connected;

void livesplit::Init(const char* host, uint16_t port) {
  int ret;
  zed_net_address_t addr {};
  if (zed_net_get_address(&addr, host, port) < 0) {
    fprintf(stderr, "LiveSplit: could not connect (failed to resolve): %s\n", zed_net_get_error());
    return;
  }
  memset(&g_socket, 0, sizeof(g_socket));
  if ((ret = zed_net_tcp_connect(&g_socket, addr)) != 0) {
    fprintf(stderr, "LiveSplit: could not connect (failed to connect): %d %s\n", ret, zed_net_get_error());
    return;
  }
  fprintf(stderr, "Connected to LiveSplit on %s:%u\n", host, port);
  g_connected = true;
}

bool livesplit::Running() {
  return g_connected;
}

void livesplit::SendSplitConfig() {
  if (!g_connected) return;
  zed_net_tcp_socket_send(&g_socket, "setsplitname 0 1-1\n", 19);
  zed_net_tcp_socket_send(&g_socket, "setsplitname 1 1-2\n", 19);
  zed_net_tcp_socket_send(&g_socket, "setsplitname 2 1-3\n", 19);
  zed_net_tcp_socket_send(&g_socket, "setsplitname 3 1-4\n", 19);
  zed_net_tcp_socket_send(&g_socket, "setsplitname 4 2-1\n", 19);
  zed_net_tcp_socket_send(&g_socket, "setsplitname 5 2-2\n", 19);
  zed_net_tcp_socket_send(&g_socket, "setsplitname 6 2-3\n", 19);
  zed_net_tcp_socket_send(&g_socket, "setsplitname 7 2-4\n", 19);
  zed_net_tcp_socket_send(&g_socket, "setsplitname 8 3-1\n", 19);
  zed_net_tcp_socket_send(&g_socket, "setsplitname 9 3-2\n", 19);
  zed_net_tcp_socket_send(&g_socket, "setsplitname 10 3-3\n", 20);
  zed_net_tcp_socket_send(&g_socket, "setsplitname 11 3-4\n", 20);
  zed_net_tcp_socket_send(&g_socket, "setsplitname 12 4-1\n", 20);
}

void livesplit::SendStart() {
  if (!g_connected) return;
  zed_net_tcp_socket_send(&g_socket, "starttimer\n", 11);
  fprintf(stderr, "LiveSplit: start\n");
}

void livesplit::SendSplit(uint32_t time) {
  if (!g_connected) return;
  char fmt[256] {0};
  uint64_t total_milliseconds = ((uint64_t)time * 1000) / 60;
  uint32_t hours = total_milliseconds / 3600000;
  uint32_t minutes = (total_milliseconds / 60000) % 60;
  uint32_t seconds = (total_milliseconds / 1000) % 60;
  uint32_t milliseconds = total_milliseconds % 1000;
  snprintf(fmt, sizeof(fmt), "setgametime %u:%02u:%02u.%03u\n", hours, minutes, seconds, milliseconds);
  zed_net_tcp_socket_send(&g_socket, fmt, strlen(fmt));
  fprintf(stderr, "LiveSplit: split time set\n");
}

void livesplit::SendPause() {
  if (!g_connected) return;
  zed_net_tcp_socket_send(&g_socket, "pause\n", 6);
  fprintf(stderr, "LiveSplit: pause\n");
}

void livesplit::SendReset() {
  if (!g_connected) return;
  zed_net_tcp_socket_send(&g_socket, "reset\n", 6);
  fprintf(stderr, "LiveSplit: reset\n");
}

void livesplit::SendPauseGameTime() {
  if (!g_connected) return;
  zed_net_tcp_socket_send(&g_socket, "pausegametime\n", 14);
}

void livesplit::SendUnpauseGameTime() {
  if (!g_connected) return;
  zed_net_tcp_socket_send(&g_socket, "unpausegametime\n", 16);
}

void livesplit::Quit() {
  g_connected = false;
  zed_net_socket_close(&g_socket);
}
