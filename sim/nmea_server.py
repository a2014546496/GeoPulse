#!/usr/bin/env python3
"""
NMEA 模拟 TCP 服务器
读取 NMEA 日志文件，按时间间隔发送到已连接的 TCP 客户端。

用法:
    python3 sim/nmea_server.py [文件] [端口] [间隔(秒)]

    python3 sim/nmea_server.py sim/sample.nmea 5000 1.0
"""

import socket
import time
import sys
import os


def main():
    # 参数解析，带默认值
    filepath = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.path.dirname(__file__), "sample.nmea"
    )
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 5000
    interval = float(sys.argv[3]) if len(sys.argv) > 3 else 1.0

    if not os.path.exists(filepath):
        print(f"❌ 文件不存在: {filepath}")
        sys.exit(1)

    # 读取所有 NMEA 语句（过滤空行）
    with open(filepath) as f:
        sentences = [
            line.strip() for line in f if line.strip()
        ]

    if not sentences:
        print("❌ 文件中无 NMEA 语句")
        sys.exit(1)

    print(f"📡 加载 {len(sentences)} 条 NMEA 语句  |  TCP 服务端口: {port}  |  间隔: {interval}s")
    print(f"   文件: {filepath}")
    print(f"   等待 GeoPulse 连接... (TCP 模式填写地址: 127.0.0.1:{port})")
    print()

    # 创建 TCP 服务器
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("0.0.0.0", port))
    server.listen(1)
    server.settimeout(2.0)  # 每 2 秒检查一次，方便 Ctrl+C 退出

    while True:
        try:
            # 等待客户端连接
            try:
                client, addr = server.accept()
            except socket.timeout:
                continue  # 循环检查中断信号

            print(f"✅ GeoPulse 已连接 ({addr[0]}:{addr[1]})")
            print("   按 Ctrl+C 停止发送\n")

            idx = 0
            try:
                while True:
                    line = sentences[idx % len(sentences)]
                    try:
                        client.send((line + "\r\n").encode())
                    except (BrokenPipeError, ConnectionResetError):
                        print("⚠️  连接断开，等待重连...\n")
                        client.close()
                        break

                    # 显示进度
                    visible = line[:60] + ("..." if len(line) > 60 else "")
                    print(f"\r[{idx+1:4d}] {visible:<65s}", end="", flush=True)

                    idx += 1
                    time.sleep(interval)

            except KeyboardInterrupt:
                client.close()
                print("\n\n⏸️  模拟已停止")
                return

        except KeyboardInterrupt:
            print("\n⏸️  服务已停止")
            return


if __name__ == "__main__":
    print()
    print("  ╔══════════════════════════════════╗")
    print("  ║   GeoPulse NMEA 模拟服务器      ║")
    print("  ╚══════════════════════════════════╝")
    print()
    try:
        main()
    except KeyboardInterrupt:
        print("\n👋 再见")
