"""Bounded wait helpers for STI network integration tests."""

import time


class WaitTimeout(AssertionError):
    pass


def status_name(status):
    name = getattr(status, "name", None)
    if name:
        return name
    text = str(status)
    if "." in text:
        return text.rsplit(".", 1)[-1]
    return text


def wait_for(predicate, timeout_s=5.0, interval_s=0.05, describe=None, diagnostics=None):
    deadline = time.time() + timeout_s
    last_error = None

    while time.time() < deadline:
        try:
            result = predicate()
        except Exception as exc:
            last_error = exc
            result = False
        if result:
            return result
        time.sleep(interval_s)

    detail = describe() if describe is not None else "condition was not met"
    if last_error is not None:
        detail = "{0}; last error: {1}".format(detail, last_error)
    if diagnostics is not None:
        detail = "{0}\n\nDiagnostics:\n{1}".format(detail, diagnostics())
    raise WaitTimeout("Timed out after {0:.3f}s: {1}".format(timeout_s, detail))


def wait_for_ticket(ticket, timeout_s=10.0, interval_s=0.05, complete_statuses=None, terminal_statuses=None, diagnostics=None):
    complete_statuses = set(complete_statuses or ["Complete"])
    terminal_statuses = set(terminal_statuses or ["Complete", "Canceled", "NotFound"])
    observed = []

    def poll():
        name = status_name(ticket.status())
        observed.append(name)
        if name in complete_statuses:
            return ticket
        if name in terminal_statuses:
            raise AssertionError("Ticket reached terminal status {0}, expected one of {1}".format(name, sorted(complete_statuses)))
        return False

    def describe():
        last = observed[-1] if observed else "<none>"
        return "ticket did not reach {0}; last status: {1}".format(sorted(complete_statuses), last)

    return wait_for(poll, timeout_s=timeout_s, interval_s=interval_s, describe=describe, diagnostics=diagnostics)


def wait_for_device_ids(hub, expected_ids, timeout_s=5.0, interval_s=0.05, diagnostics=None):
    expected = set([_device_id_text(device_id) for device_id in expected_ids])
    observed = []

    def poll():
        current = set([_device_id_text(device_id) for device_id in hub.getDeviceIDs()])
        observed.append(sorted(current))
        return expected.issubset(current)

    def describe():
        last = observed[-1] if observed else []
        return "missing device IDs {0}; observed: {1}".format(sorted(expected), last)

    return wait_for(poll, timeout_s=timeout_s, interval_s=interval_s, describe=describe, diagnostics=diagnostics)


def _device_id_text(device_id):
    if hasattr(device_id, "getID"):
        return device_id.getID()
    return str(device_id)
