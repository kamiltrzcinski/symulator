#!/usr/bin/env python3
# Run with: .venv/bin/python3 scripts/e2e_smoke_test.py
# Requires: pip install flatbuffers  (already in .venv)
"""
E2E smoke test for the railway simulator server.

Verifies:
  1. HANDSHAKE (0x01) → HANDSHAKE_ACK (0x02) with session_id
  2. SNAPSHOT_REQUEST (0x30) → SNAPSHOT_CHUNK(s) (0x31), last chunk flagged

Usage:
    python3 scripts/e2e_smoke_test.py [--host HOST] [--port PORT] [--timeout SECONDS]

Exits 0 on success, non-zero on failure.
Requires: pip install flatbuffers
"""

import argparse
import binascii
import socket
import struct
import sys
import time

import flatbuffers

# ── Frame constants ──────────────────────────────────────────────────────────

MAGIC_0 = 0x53  # 'S'
MAGIC_1 = 0x52  # 'R'
HEADER_SIZE = 16

MSG_HANDSHAKE        = 0x01
MSG_HANDSHAKE_ACK    = 0x02
MSG_HEARTBEAT        = 0x03
MSG_HEARTBEAT_ACK    = 0x04
MSG_COMMAND          = 0x10
MSG_COMMAND_ACK      = 0x11
MSG_COMMAND_NAK      = 0x12
MSG_DOMAIN_EVENT     = 0x20
MSG_SNAPSHOT_REQUEST = 0x30
MSG_SNAPSHOT_CHUNK   = 0x31

FLAG_IS_LAST_CHUNK = 0x01

MSG_TYPE_NAMES = {
    0x01: "HANDSHAKE",
    0x02: "HANDSHAKE_ACK",
    0x03: "HEARTBEAT",
    0x04: "HEARTBEAT_ACK",
    0x10: "COMMAND",
    0x11: "COMMAND_ACK",
    0x12: "COMMAND_NAK",
    0x20: "DOMAIN_EVENT",
    0x30: "SNAPSHOT_REQUEST",
    0x31: "SNAPSHOT_CHUNK",
    0x40: "SESSION_NOTICE",
}


# ── Frame encoding / decoding ────────────────────────────────────────────────

def _crc32(data: bytes) -> int:
    """CRC-32/ISO-HDLC (same poly/init/finalXOR as zlib crc32 / Ethernet / PNG)."""
    return binascii.crc32(data) & 0xFFFF_FFFF


def encode_frame(msg_type: int, flags: int, seq_id: int, payload: bytes) -> bytes:
    """Build a complete 16-byte header + payload frame."""
    header_nocrc = struct.pack("<BBBBII", MAGIC_0, MAGIC_1, msg_type, flags, seq_id, len(payload))
    crc = _crc32(header_nocrc + payload)
    return header_nocrc + struct.pack("<I", crc) + payload


# ── FlatBuffers helpers ──────────────────────────────────────────────────────

def build_handshake(player_id: str, auth_token: str) -> bytes:
    """
    Build a Handshake FlatBuffer (proto/session.fbs).

    table Handshake {
        proto_version:  uint32;          // field 0
        player_id:      string;          // field 1
        auth_token:     string;          // field 2
        client_version: string;          // field 3 (omitted)
    }
    """
    b = flatbuffers.Builder(256)
    pid_off  = b.CreateString(player_id)
    auth_off = b.CreateString(auth_token)
    b.StartObject(4)
    b.PrependUint32Slot(0, 1, 0)                        # proto_version = 1
    b.PrependUOffsetTRelativeSlot(1, pid_off,  0)       # player_id
    b.PrependUOffsetTRelativeSlot(2, auth_off, 0)       # auth_token
    table = b.EndObject()
    b.Finish(table)
    return bytes(b.Output())


def read_string_field_0(payload: bytes) -> str | None:
    """
    Read the first string field (field index 0) from a FlatBuffers table payload.
    Used to extract session_id from HandshakeAck.
    Returns None if the payload is malformed.
    """
    if len(payload) < 8:
        return None
    try:
        buf = bytearray(payload)
        # Root offset: 4-byte LE uint at start of buffer
        root_off = struct.unpack_from("<I", buf, 0)[0]
        # Inline int32 at root_off points backwards to vtable
        vtable_soff = struct.unpack_from("<i", buf, root_off)[0]
        vtable_off = root_off - vtable_soff
        # vtable layout: vtable_size (uint16), object_size (uint16), field_offsets...
        #   field 0 starts at vtable_off + 4
        field_off = struct.unpack_from("<H", buf, vtable_off + 4)[0]
        if field_off == 0:
            return None  # field not present
        str_indirect = root_off + field_off
        str_off = str_indirect + struct.unpack_from("<I", buf, str_indirect)[0]
        str_len = struct.unpack_from("<I", buf, str_off)[0]
        str_start = str_off + 4
        return buf[str_start:str_start + str_len].decode("utf-8")
    except Exception:
        return None


# ── TCP connection wrapper ───────────────────────────────────────────────────

class Connection:
    def __init__(self, host: str, port: int, connect_timeout: float = 5.0):
        self._sock = socket.create_connection((host, port), timeout=connect_timeout)
        self._sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        self._buf = b""
        self._seq = 0

    def send(self, msg_type: int, payload: bytes, flags: int = 0) -> None:
        frame = encode_frame(msg_type, flags, self._seq, payload)
        self._seq += 1
        self._sock.sendall(frame)

    def recv_frame(self, timeout: float) -> tuple[int, int, int, bytes]:
        """
        Receive one complete frame, blocking up to `timeout` seconds.
        Returns (msg_type, flags, seq_id, payload).
        Raises TimeoutError or ConnectionError on failure.
        """
        deadline = time.monotonic() + timeout
        while True:
            if len(self._buf) >= HEADER_SIZE:
                if self._buf[0] != MAGIC_0 or self._buf[1] != MAGIC_1:
                    raise ValueError(
                        f"Bad magic: 0x{self._buf[0]:02x} 0x{self._buf[1]:02x}"
                    )
                msg_type = self._buf[2]
                flags    = self._buf[3]
                seq_id, payload_len, wire_crc = struct.unpack_from("<III", self._buf, 4)
                if payload_len > 65535:
                    raise ValueError(f"Oversized payload: {payload_len}")
                total = HEADER_SIZE + payload_len
                if len(self._buf) >= total:
                    header_nocrc = bytes(self._buf[:12])
                    payload      = bytes(self._buf[HEADER_SIZE:total])
                    self._buf    = self._buf[total:]
                    expected_crc = _crc32(header_nocrc + payload)
                    if wire_crc != expected_crc:
                        raise ValueError(
                            f"CRC mismatch for msg_type=0x{msg_type:02x}: "
                            f"got 0x{wire_crc:08x}, expected 0x{expected_crc:08x}"
                        )
                    return msg_type, flags, seq_id, payload

            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise TimeoutError("Timed out waiting for server frame")
            self._sock.settimeout(min(remaining, 0.5))
            try:
                chunk = self._sock.recv(65536)
            except socket.timeout:
                continue
            if not chunk:
                raise ConnectionError("Server closed the connection")
            self._buf += chunk

    def close(self) -> None:
        try:
            self._sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        self._sock.close()


# ── Test steps ───────────────────────────────────────────────────────────────

def step_handshake(conn: Connection, timeout: float) -> str:
    """Send HANDSHAKE and verify HANDSHAKE_ACK. Returns session_id."""
    payload = build_handshake("smoke-test", "test-token")
    conn.send(MSG_HANDSHAKE, payload)
    print(f"  → HANDSHAKE sent ({len(payload)} byte payload)")

    msg_type, flags, seq_id, resp = conn.recv_frame(timeout=timeout)
    name = MSG_TYPE_NAMES.get(msg_type, f"0x{msg_type:02x}")
    if msg_type != MSG_HANDSHAKE_ACK:
        raise AssertionError(f"Expected HANDSHAKE_ACK (0x02), got {name}")
    if not resp:
        raise AssertionError("HANDSHAKE_ACK payload is empty")

    session_id = read_string_field_0(resp) or "(unreadable)"
    print(f"  ← HANDSHAKE_ACK  session_id='{session_id}'  ({len(resp)} bytes)")
    return session_id


def step_snapshot(conn: Connection, timeout: float) -> int:
    """
    Send SNAPSHOT_REQUEST and collect all SNAPSHOT_CHUNKs.
    Returns total number of bytes received across all chunks.
    """
    # Server ignores SnapshotRequest payload — send empty.
    conn.send(MSG_SNAPSHOT_REQUEST, b"")
    print("  → SNAPSHOT_REQUEST sent")

    total_bytes = 0
    chunk_count = 0
    deadline = time.monotonic() + timeout
    while True:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            raise TimeoutError(
                "Timed out waiting for SNAPSHOT_CHUNK "
                "(server may not have a snapshot yet — try increasing --timeout)"
            )
        msg_type, flags, seq_id, payload = conn.recv_frame(timeout=remaining)
        name = MSG_TYPE_NAMES.get(msg_type, f"0x{msg_type:02x}")
        if msg_type != MSG_SNAPSHOT_CHUNK:
            # Unexpected frame — server might not have a snapshot yet (no engine tick)
            if msg_type in (MSG_HEARTBEAT, MSG_HEARTBEAT_ACK):
                continue  # ignore heartbeats
            raise AssertionError(
                f"Expected SNAPSHOT_CHUNK (0x31), got {name} (0x{msg_type:02x})"
            )
        chunk_count += 1
        total_bytes += len(payload)
        is_last = bool(flags & FLAG_IS_LAST_CHUNK)
        print(
            f"  ← SNAPSHOT_CHUNK #{chunk_count}  {len(payload)} bytes"
            + ("  [LAST]" if is_last else "")
        )
        if is_last:
            break

    if total_bytes == 0:
        raise AssertionError("Snapshot was empty (zero payload bytes)")
    return total_bytes


# ── Retry wrapper for snapshot ───────────────────────────────────────────────

def step_snapshot_with_retry(conn: Connection, timeout: float, retries: int = 3) -> int:
    """
    Retry the snapshot request if the server responds with no data.
    The engine may not have ticked yet on the very first request.
    """
    delay = 0.2
    for attempt in range(1, retries + 1):
        try:
            return step_snapshot(conn, timeout=timeout)
        except (TimeoutError, AssertionError) as exc:
            if attempt == retries:
                raise
            print(f"  ! Snapshot attempt {attempt} failed: {exc} — retrying in {delay:.1f}s")
            time.sleep(delay)
            delay *= 2


# ── Main ─────────────────────────────────────────────────────────────────────

def main() -> int:
    parser = argparse.ArgumentParser(description="E2E smoke test for the simulator server")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=9420)
    parser.add_argument("--timeout", type=float, default=10.0,
                        help="Per-step timeout in seconds (default: 10)")
    args = parser.parse_args()

    print(f"Connecting to {args.host}:{args.port} …")
    try:
        conn = Connection(args.host, args.port, connect_timeout=args.timeout)
    except (ConnectionRefusedError, TimeoutError) as exc:
        print(f"FAIL: Cannot connect — {exc}")
        print("      Is the server running?  Example:")
        print("        build/ninja-debug/server/symulator-server \\")
        print("          --scenario scenarios/gdynia_orlowo \\")
        print("          --data data/ --port 9420")
        return 1

    print("Connected.\n")
    ok = True
    try:
        print("[1/2] Handshake")
        session_id = step_handshake(conn, timeout=args.timeout)

        print(f"\n[2/2] Snapshot  (waiting up to {args.timeout:.0f}s for engine tick)")
        total_bytes = step_snapshot_with_retry(conn, timeout=args.timeout)
        print(f"      Total snapshot payload: {total_bytes} bytes")

    except (AssertionError, ValueError, TimeoutError, ConnectionError, OSError) as exc:
        print(f"\nFAIL: {exc}")
        ok = False
    finally:
        conn.close()

    if ok:
        print("\nPASS — smoke test completed successfully.")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
