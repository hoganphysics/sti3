import os
import platform
import socket
import ipaddress
import subprocess

def _default_route_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("8.8.8.8", 1))
    return s.getsockname()[0]

def _default_iface():
    system = platform.system().lower()
    if system == "linux":
        try:
            with open("/proc/net/route", "r", encoding="ascii") as f:
                for line in f.readlines()[1:]:
                    fields = line.strip().split()
                    if len(fields) >= 2 and fields[1] == "00000000":
                        return fields[0]
        except OSError:
            return None
    if system == "darwin":
        try:
            import netifaces  # optional
            return netifaces.gateways()["default"][netifaces.AF_INET][1]
        except Exception:
            pass
        try:
            out = subprocess.check_output(["route", "-n", "get", "default"], text=True)
            for line in out.splitlines():
                line = line.strip()
                if line.startswith("interface:"):
                    return line.split(":", 1)[1].strip()
        except Exception:
            return None
    return None

def get_local_ip_address():
    env_ip = os.getenv("STI3_PUBLISH_IP")
    if env_ip:
        return env_ip

    system = platform.system().lower()
    if system == "windows":
        return _default_route_ip()

    try:
        import fcntl
        import struct
    except ModuleNotFoundError:
        return _default_route_ip()

    def _ifaddr(ifname):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            return socket.inet_ntoa(
                fcntl.ioctl(
                    s.fileno(),
                    0x8915,  # SIOCGIFADDR
                    struct.pack("256s", ifname[:15].encode("utf-8"))
                )[20:24]
            )
        except OSError:
            return None

    env_subnet = os.getenv("STI3_PUBLISH_SUBNET")
    if env_subnet:
        net = ipaddress.ip_network(env_subnet, strict=False)
        for _, ifname in socket.if_nameindex():
            ip = _ifaddr(ifname)
            if ip and ipaddress.ip_address(ip) in net:
                return ip

    default_iface = _default_iface()
    for _, ifname in socket.if_nameindex():
        if ifname in ("lo", "lo0"):
            continue
        ip = _ifaddr(ifname)
        if ip and ifname != default_iface:
            return ip

    return _default_route_ip()
