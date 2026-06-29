"""Regression tests for Python sequence-run cancellation handling."""

import pytest

from stipy.python import server as server_module
from stipy.stidevicepy.stidevicepy import EngineJobStatus


pytestmark = pytest.mark.integration


class _FakeEntry:
    overwritten = {}


class _FakeSequence:
    shotmaker = None

    def __init__(self, keys):
        self.sequenceTable = {key: _FakeEntry() for key in keys}


class _FakeSequenceEntryID:
    def __init__(self):
        self.seqID = None
        self.seqIndex = None


class _WaitTicket:
    def __init__(self, on_wait=None):
        self._on_wait = on_wait

    def wait(self):
        if self._on_wait is not None:
            self._on_wait()


class _FakeScheduler:
    def __init__(self):
        self.status = EngineJobStatus.New
        self.checked_sequence_ids = []

    def getStatus(self, sequence_id):
        self.checked_sequence_ids.append(sequence_id)
        return self.status


class _FakeServer:
    def __init__(self, scheduler, parse_wait=None, play_wait=None):
        self.scheduler = scheduler
        self.parse_wait = parse_wait
        self.play_wait = play_wait
        self.parsed_indices = []
        self.played_indices = []

    def getEngineScheduler(self):
        return self.scheduler

    def parse(self, shot, sequence_entry_id):
        self.parsed_indices.append(sequence_entry_id.seqIndex)
        return _WaitTicket(self.parse_wait)

    def play(self, parse_ticket):
        self.played_indices.append(self.parsed_indices[-1])
        return _WaitTicket(self.play_wait)


def _patch_run_shots_dependencies(monkeypatch):
    monkeypatch.setattr(server_module, "makeshot", lambda *args, **kwargs: object())
    monkeypatch.setattr(server_module, "SequenceEntryID", _FakeSequenceEntryID)


@pytest.mark.parametrize("terminal_status", [EngineJobStatus.Canceled, EngineJobStatus.Completed])
def test_run_shots_stops_submitting_parses_after_sequence_is_no_longer_live_during_play(
    monkeypatch,
    terminal_status,
):
    _patch_run_shots_dependencies(monkeypatch)

    scheduler = _FakeScheduler()
    server = _FakeServer(
        scheduler,
        play_wait=lambda: setattr(scheduler, "status", terminal_status),
    )
    progress = []
    sequence_id = object()

    server_module.run_shots(
        server,
        _FakeSequence([0, 1, 2]),
        sequence_id,
        progress=lambda shot_number, total: progress.append((shot_number, total)),
    )

    assert server.parsed_indices == [0]
    assert server.played_indices == [0]
    assert progress == [(1, 3)]
    assert scheduler.checked_sequence_ids
    assert all(checked is sequence_id for checked in scheduler.checked_sequence_ids)


def test_run_shots_does_not_submit_play_after_sequence_is_canceled_during_parse(monkeypatch):
    _patch_run_shots_dependencies(monkeypatch)

    scheduler = _FakeScheduler()
    server = _FakeServer(
        scheduler,
        parse_wait=lambda: setattr(scheduler, "status", EngineJobStatus.Canceled),
    )

    server_module.run_shots(server, _FakeSequence([0, 1]), object())

    assert server.parsed_indices == [0]
    assert server.played_indices == []
