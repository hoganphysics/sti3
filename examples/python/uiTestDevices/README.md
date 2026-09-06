# UI Test Devices

This manual integration fixture starts three lightweight output devices plus an
event-checking device. It is useful for exercising device lists, attributes,
channel controls, event parsing, and scheduler updates in an STI UI.

Start an omniORB name service and an STI server, then run:

```bash
python3 examples/python/uiTestDevices/main.py
```

The defaults expect both services on the local machine. A lab setup can be
selected with command-line options:

```bash
python3 examples/python/uiTestDevices/main.py \
  --name-service 192.0.2.10:2809 \
  --target-server "lab-host/0/STI Server"
```

The same values can be supplied through `STI_NAME_SERVICE` and
`STI_TARGET_SERVER`. Set `STI_SCHEDULER_SERVER` as well when scheduler updates
come from a different server.

The devices are:

| Device | Output channels |
| --- | ---: |
| `EventCheckingDevice` | 2 |
| `Device1` | 2 |
| `Device2` | 1 |
| `Device3` | 3 |

Use `test-shot.ipynb` to create and play a simple shot against this device set.
Runtime persistence and shot-cache data are written below `.sti/`, which Git
ignores.
