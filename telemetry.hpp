#ifndef TELEMETRY_HPP
#define TELEMETRY_HPP

namespace Telemetry {
  bool setup();

  void sendPacket();

  void rcvPacket();
}

#ifdef // TELEMETRY_HPP