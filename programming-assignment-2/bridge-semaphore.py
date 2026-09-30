from threading import Thread, Lock, Condition, Semaphore
import time
import random

NORTH = 0
SOUTH = 1

class OneLaneBridge:
    """
    A one-lane bridge allows multiple cars to pass in either direction, but at any
    point in time, all cars on the bridge must be going in the same direction.
    Directions are represented by the constants NORTH and SOUTH defined at the top
    of this module.

    Cars wishing to cross will call the cross() function, which blocks until it is
    safe for the caller to cross in the desired direction. Once they have crossed
    they will call the finished() function to indicate they have left the bridge.
    """

    def __init__(self):
        self.north_count = 0
        self.south_count = 0
        self.north_lock = Semaphore(1)
        self.south_lock = Semaphore(1)
        self.occupied = Semaphore(1) 
        self.direction = 0

    def cross(self, direction):
        """Wait for permission to cross the bridge. Direction should be either
        NORTH (0) or SOUTH (1)."""
        if direction == 0:
            self.north_lock.acquire()
            self.north_count += 1
            if self.north_count == 1:
                self.occupied.acquire()        
                self.direction = 0 
            self.north_lock.release()
        else:
            self.south_lock.acquire()
            self.south_count += 1
            if self.south_count == 1:
                self.occupied.acquire()
                self.direction = 1 
            self.south_lock.release()

    def finished(self):
        """Signals that the caller is finished crossing the bridge."""
        a_direction = self.direction          
        if a_direction == NORTH:
            self.north_lock.acquire()
            self.north_count -= 1
            if self.north_count == 0:
                self.occupied.release()
            self.north_lock.release()
        else:
            self.south_lock.acquire()
            self.south_count -= 1
            if self.south_count == 0:
                self.occupied.release()
            self.south_lock.release()

# You do not need to write any code below this line

DIRECTION_NAMES = ["NORTH", "SOUTH"]

class Car(Thread):
    def __init__(self, bridge):
        Thread.__init__(self)
        self.direction = random.randrange(2)
        self.wait_time = random.uniform(0.1,0.5)
        self.bridge = bridge

    def run(self):
        # drive to the bridge
        time.sleep(self.wait_time)

        # request permission to cross
        print(f"Waiting to cross in direction {DIRECTION_NAMES[self.direction]}")
        self.bridge.cross(self.direction)
        print(f"Crossing in direction {DIRECTION_NAMES[self.direction]}")

        # drive across
        time.sleep(0.01)

        # signal that we have finished crossing
        self.bridge.finished()
        print(f"Finished crossing in direction {DIRECTION_NAMES[self.direction]}")


if __name__ == "__main__":

    my_bridge = OneLaneBridge()
    for i in range(100):
        Car(my_bridge).start()

