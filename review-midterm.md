## Monitors and Condition Variables

1. _pseudo_ answer 
```py
class ReadWriteLock:
    def __init__(self):
        self.rw_lock = Lock()
        self.can_read = Condition(self.rw_lock)
        self.can_write = Condition(self.rw_lock)
        self.read_counter = 0
        self.write_counter = 0

    def aquire_read(self):
        with self.rw_lock:
            while self.write_counter > 0:
                self.can_read.wait():
            self.num_readers += 1

    def release_read(self):
        self.num_readers -= 1
        if self.num_readers == 0:
            self.can_write.notify()

    def acquire_write(self):
        with self.rw_lock:
            while (self.num_readers > 0 or self.num_writers =1):
                self.can_write.wait()
            self.num_writers = 1

    def release_write(self):
        with self.rw_lock:
            self.write_counter -= 1
            self.can_read.notify_all()
            self.can_write.notify()

```
2. _builds on problem 1. review slides_ \
essentially adding a second counter which tracks if a writer is waiting, then you can order your notifies differently to give priority to the writers. _solves the fairness problem_

## Scheduling

3. 
Round Robin 
```
5   5 units compute - A; Waiting - B; 
9   4 units compute - B; B does I/O request;
14  5 units compute - A; B finishes I/O request; Waiting - B;
19  5 units compute - B; A does I/O request;
24  5 units compute - A; B does I/O request;
27  3 units compute - B; Waiting - A; -- B finishes
32  5 units compute - A; -- A finishes 

A: Completion Time = 32. Waiting Time = 7
B: Completion Time = 27. Waiting Time = 6
```

[!alert] Process does not get quantum if it is not ready to use CPU

Shortest Remaining Time First 
```
4 units compute - B; Waiting - A;
10 units of compute - A; B does I/O request;
```












