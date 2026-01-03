# STI3 CORBA Network Configuration Guide
*(omniORB endpoints, NameService, and device callbacks)*

This document explains how to correctly configure CORBA networking for:

- **omniNames (NameService)**
- **STIServer**
- **STI devices** (which support callbacks, so they are also CORBA servers)

It focuses on *multi-host lab environments*, where machines may have multiple NICs and firewalls.

Your configuration format is:

```
[NetworkHub]
NameService = <ip>:<port>

[omniORB]
key = value
```

All `omniORB` settings described below become `-ORB...` arguments internally.

---

## 1️⃣ NameService (omniNames)

Run omniNames on your Ubuntu server and force it to publish a reachable address.

On the **omniNames host** (e.g., `192.168.1.242`):

```
-ORBendPoint giop:tcp::2809
-ORBendPointPublish giop:tcp:192.168.1.242:2809
```

This ensures:

- omniNames listens on port **2809** on all interfaces  
- It **publishes** an address all clients can reach  
  (critical on multi-NIC systems)

---

## 2️⃣ All Clients Must Know Where NameService Lives

Configure every STI3 component to use the same NameService:

```
[NetworkHub]
NameService = 192.168.1.242:2809
```

Your code should translate this into:

```
-ORBInitRef NameService=corbaname::192.168.1.242:2809/NameService
```

---

## 3️⃣ STIServer Endpoint Configuration

STIServer is a **CORBA server**. Give it a fixed endpoint and published address:

```
[omniORB]
traceLevel = 25
endPoint = giop:tcp::2810
endPointPublish = giop:tcp:192.168.1.242:2810
```

Meaning:

- Listen on port **2810**
- Publish **exactly this host + port** in IORs

🔥 This avoids omniORB choosing the wrong NIC / hostname.

✔️ Open firewall if needed (Ubuntu `ufw` or upstream lab firewall):

```
allow TCP 2810
```

---

## 4️⃣ Devices Are Also Servers (Callbacks)

Even though devices “connect to” STIServer,
**STIServer calls back into devices**.

Therefore devices must expose a stable CORBA endpoint.

Example device configuration:

```
[omniORB]
traceLevel = 25
endPoint = giop:tcp::2820
endPointPublish = giop:tcp:192.168.1.247:2820
```

Rules:

- `endPoint` = where the device listens  
- `endPointPublish` = what STIServer connects to  
- **They must match**
- Use the device’s actual lab IP

✔️ Open Windows firewall inbound:

- Allow TCP port `2820`
- Or allow the Python / C++ STI program

---

## 5️⃣ Minimal Working Connectivity Checklist

### On STIServer host

Check that listeners exist:

```
sudo ss -tulpen | grep 28
```

Expect:

```
*:2809 omniNames
*:2810 STIServer
```

### From device machine (PowerShell)

```
Test-NetConnection 192.168.1.242 -Port 2809
Test-NetConnection 192.168.1.242 -Port 2810
```

Should say:

```
TcpTestSucceeded : True
```

### From STIServer to device

```
nc -vz 192.168.1.247 2820
```

Must succeed.

---

## 6️⃣ Recommended Timeout Settings

These make failures fast instead of multi‑minute hangs:

```
[omniORB]
clientConnectTimeOutPeriod = 2000     # ms
clientCallTimeOutPeriod    = 15000    # ms
scanGranularity            = 1        # seconds
```

---

## 7️⃣ Common Failure Modes & Symptoms

| Symptom | Cause |
|--------|--------|
Client hangs forever when calling STIServer | Wrong STIServer published address |
Client resolves NameService locally only | NameService not publishing correct IP |
Device appears connected but calls timeout | Callback path cannot reach device |
Server log shows connection attempt to wrong device port | `endPointPublish` typo |
Works at home but not lab | Multi‑NIC + missing publish address |
Device shows connected but nothing works | Windows firewall blocking device port |

---

## 8️⃣ Debugging Playbook

### Verify TCP Reachability First

- Windows → Ubuntu:
  ```
  Test-NetConnection <server-ip> -Port 2810
  ```

- Ubuntu → Windows:
  ```
  nc -vz <device-ip> 2820
  ```

If TCP fails, omniORB will never work.

### Look at omniORB Trace

Run with:

```
-ORBtraceLevel 25
```

Useful signs:

- `Client attempt to connect to ...`  
  Shows what address omniORB really believes

- `TRANSIENT_CallTimedOut` or `TIMEOUT_CallTimedOutOnClient`  
  Usually means server is stuck trying to call you back

- `LocateRequest to remote: root/bidir<0>`  
  Reminder that callbacks / bidir are in play

---

## 9️⃣ Summary Rules

1️⃣ Always give **NameService**, **STIServer**, and **Devices** explicit `endPoint` + `endPointPublish`.  
2️⃣ All components must use the same NameService IP via `InitRef`.  
3️⃣ Devices must allow inbound firewall connections.  
4️⃣ Prefer IPs over hostnames in lab environments.  
5️⃣ If something feels random: inspect trace logs and confirm the actual published IOR addresses.

---

If desired, this can be turned into an automated validation tool for STI3 deployment checks.
