import os
import platform
import socket

def _default_route_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("8.8.8.8", 1))
    return s.getsockname()[0]

def _in_container():
    if os.path.exists("/.dockerenv"):
        return True
    try:
        with open("/proc/1/cgroup", "r", encoding="ascii") as f:
            data = f.read()
        return "docker" in data or "containerd" in data
    except OSError:
        return False

def get_local_ip_address():
    env_ip = os.getenv("STI3_PUBLISH_IP")
    if env_ip:
        return env_ip

    if platform.system().lower() == "windows":
        return _default_route_ip()

    try:
        import fcntl, struct, ipaddress
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

    def _default_iface_linux():
        try:
            with open("/proc/net/route", "r", encoding="ascii") as f:
                for line in f.readlines()[1:]:
                    fields = line.strip().split()
                    if len(fields) >= 2 and fields[1] == "00000000":
                        return fields[0]
        except OSError:
            return None

    # Optional subnet override (Linux)
    env_subnet = os.getenv("STI3_PUBLISH_SUBNET")
    if env_subnet:
        net = ipaddress.ip_network(env_subnet, strict=False)
        for _, ifname in socket.if_nameindex():
            ip = _ifaddr(ifname)
            if ip and ipaddress.ip_address(ip) in net:
                return ip

    ips = {}
    for _, ifname in socket.if_nameindex():
        if ifname == "lo":
            continue
        ip = _ifaddr(ifname)
        if ip:
            ips[ifname] = ip

    if not ips:
        return _default_route_ip()

    if len(ips) == 1:
        return next(iter(ips.values()))

    default_iface = _default_iface_linux()
    if _in_container() and default_iface in ips:
        for ifname, ip in ips.items():
            if ifname != default_iface:
                return ip

    if default_iface in ips:
        return ips[default_iface]

    return next(iter(ips.values()))
