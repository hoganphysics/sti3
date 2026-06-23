"""stidevicepy task last-run-time API coverage."""


def test_python_task_last_run_time_updates_after_run_now(stipy_modules):
    stipy, _ = stipy_modules

    class CountingTask(stipy.Task):
        def __init__(self):
            super().__init__("python-counting-task")
            self.runs = 0

        def run(self):
            self.runs += 1

    task = CountingTask()

    assert task.getLastRunTime() is None
    assert not task.hasLastRunTime()

    run_time = task.runNow()

    assert task.runs == 1
    assert task.hasLastRunTime()
    assert task.getLastRunTime() == run_time
    assert isinstance(run_time, stipy.TimeStamp)
