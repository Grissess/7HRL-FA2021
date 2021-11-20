import operator, itertools

class Event(object):
    priority = 0
    period = 1.0
    delay = 0.0

    def init_timebase(self, sched):
        self.next_time = sched.time + self.delay

    def perform(self, ctx):
        self.next_time += self.period

class Context(object):
    def __init__(self, sched):
        self.sched = sched
        self.index = 0
        self.event = None
        self.removed = False

    def set_event(self, event, index):
        self.event, self.index, self.removed = event, index, False

    def remove_self(self):
        if not self.removed:
            self.sched.remove(self.index)
            self.removed = True

class SortedList(object):
    def __init__(self, key, init = ()):
        self.key = key
        self.values = []
        self.add(*init)

    def __getitem__(self, item):
        return self.values[item]

    def __setitem__(self, item, val):
        self.values[item] = val

    def __delitem__(self, item):
        del self.values[item]

    def __len__(self):
        return len(self.values)

    def __iter__(self):
        return iter(self.values)

    def add(self, *values):
        # bisect doesn't have key until 3.10; this might be quicker anyway
        self.values = sorted(itertools.chain(self.values, values), key = self.key)

    def remove(self, *values):
        for val in values:
            self.values.remove(val)

class Scheduler(object):
    SORT_KEY = operator.attrgetter('priority')
    BEFORE_KEY = operator.attrgetter('before_priority')
    AFTER_KEY = operator.attrgetter('after_priority')
    TIME_KEY = operator.attrgetter('next_time')
    EVENT_KEY = staticmethod(lambda x: x[1].next_time)

    def __init__(self, events = (), before = (), after = ()):
        self.time = 0.0
        self.events = SortedList(self.SORT_KEY, events)
        self.before = SortedList(self.BEFORE_KEY, before)
        self.after = SortedList(self.AFTER_KEY, after)
        for ev in self.events:
            ev.init_timebase(self)
        self.ctx = Context(self)

    def __iter__(self):
        return self

    def remove(self, idx):
        del self.events[idx]

    def add(self, *events):
        # avoid issues with monotonicity
        for ev in events:
            ev.init_timebase(self)
        self.events.add(*events)

    def __next__(self):
        if not self.events:
            raise StopIteration()

        while True:
            idx, first = min(enumerate(self.events), key = self.EVENT_KEY)
            self.time = first.next_time

            self.ctx.set_event(first, idx)

            for before in self.before:
                before.before(self.ctx)
                if self.ctx.removed:
                    break
            if self.ctx.removed:
                continue

            ret = first.perform(self.ctx)

            for after in self.after:
                after.after(self.ctx)

            return ret

if __name__ == '__main__':
    class IteratedEvent(Event):
        def __init__(self, name, times):
            self.name = name
            self.times = times
            print(f'Initialized {self.name} to {self.times} times')

        def perform(self, ctx):
            print(f'Event {self.name} at time {ctx.sched.time}')
            super().perform(ctx)
            self.times -= 1
            if self.times <= 0:
                print(f'Finished {self.name}')
                ctx.remove_self()
            else:
                print(f'Event {self.name} has {self.times} times left...')
            return self.name, self.times

    sched = Scheduler()
    iev1 = IteratedEvent('ev1', 5)
    iev2 = IteratedEvent('ev2', 3)
    iev2.priority = 1
    iev3 = IteratedEvent('ev3', 3)
    iev3.priority = -1
    iev4 = IteratedEvent('ev4', 10)
    iev4.period = 0.3
    sched.add(iev1, iev2, iev3, iev4)

    for value in sched:
        print(f'Iteration: {value}')

    print('Done')
