"""Reusable STIPy shot builders for integration scenarios."""

from .devices import require_stipy


def single_device_output(device_id, channel=0, time_ns=1000, value=1.0):
    def shot():
        stipy, _ = require_stipy()
        stipy.event(stipy.ch(stipy.dev(device_id), channel), time_ns, value)

    return shot


def multi_device_output(events):
    normalized = list(events)

    def shot():
        stipy, _ = require_stipy()
        for device_id, channel, time_ns, value in normalized:
            stipy.event(stipy.ch(stipy.dev(device_id), channel), time_ns, value)

    return shot


def delegated_trigger_output(trigger_device_id, event_device_id, channel=0, time_ns=1000, value=1.0):
    return delegated_trigger_events(
        trigger_device_id,
        [(event_device_id, channel, time_ns, value)],
    )


def delegated_trigger_events(trigger_device_id, events):
    normalized = list(events)

    def shot():
        stipy, _ = require_stipy()
        stipy.set_trigger(stipy.dev(trigger_device_id))
        for device_id, channel, time_ns, value in normalized:
            stipy.event(stipy.ch(stipy.dev(device_id), channel), time_ns, value)

    return shot


def generated_output_events(device_ids, events_per_device, first_time_ns=1000, spacing_ns=1000, channel=0, value=1.0):
    events = []
    time_ns = first_time_ns
    for device_id in device_ids:
        for _ in range(events_per_device):
            events.append((device_id, channel, time_ns, value))
            time_ns += spacing_ns
    return multi_device_output(events)
