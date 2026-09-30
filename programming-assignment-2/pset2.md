## Problem Set 2

1.a. the while loop takes 3 steps - the check, read, and write, which means x could be an range from 50 - 101. the race conditions happen when the threads interleave between their reads and writes.
- For x to equal 50, T0 would need to test and read x as zero then T1 would then count itself up to 100 before returning to T0 who would then continue and overwrite the x value for the 0 it previously read. Then it would count itself up to 50.
- For the value to be 101, T0 would test but not read x=0 then T1 begins its counting. When T0 wakes up after T1 has finished it will continue its read and write because it has already passed the test phase. The final value would then be the value T1 arrived at +1. So, 101.
- For the values between 50 and 101, that would require T0 to test 0, then T1 to begin counting and surpas 50. Then, if T0 wakes up it could read in the value of T1 then return to sleep. Once T1 finishes counting at 100, T0 wakes up and replaces the value with the old read-in and adds 1.

1.b. The final value of x is either 50 or 51. 50 is the expected behavior, but there could be a case when the threads interleaves between a check and a read and despite x=50 it continues by reading 50 then writing a +1. 

1.c the final value is 50 assuming it finishes. The expected behavior is 50, but if the threads interleave where T0 sees x=50 (meaning that T0 will never increment again) but T1 has run its test and sees x<50 prior to T0 incrementing, then it would just decrement x until the program crashes.

2.a.

Time 1: C arrives (needs 15 time units). CPU is idle, so select C. Switch 1-4, C runs from 4. \
Time 3: A arrives (needs 45 time units). That's longer than C's 15, so A waits.  \
Time 7: B arrives (needs 6 time units). C has 12 time units left, so preempt C and select B. Switch 7-10, B runs 10–16.  \
Time 16: B goes to I/O until 49. Select C (12 time units left vs. A's 45). Switch 16-19, C runs 19–31.  \
Time 31: C goes to I/O until 41. Select A. Switch 31-34, A runs from 34.  \
Time 41: C returns (needs 10 time units). A has 38 time units left, so preempt A and select C. Switch 41-44, C runs 44–54.  \
Time 49: B returns (needs 9 time units). C has only 5 time units left, so no preemption.  \
Time 54: C goes to I/O until 69. Select B (9 time units vs. A's 38). Switch 54-57, B runs 57–66.  \
Time 66: B goes to I/O until 90. Select A. Switch 66-69.  \
Time 69: C returns (needs 14 time units). A has 38 time units left, so preempt A before it runs and select C. Switch 69-72, C runs 72–86.  \
Time 86: C finishes. Select A. Switch 86-89, A runs from 89.  \
Time 90: B returns (needs 19 time units). A has 37 time units left, so preempt A and select B. Switch 90-93, B runs 93–112.  \
Time 112: B finishes. Select A. Switch 112-115, A runs 115–152.  \
Time 152: A goes to I/O until 162. CPU idle.  \
Time 162: A returns (needs 15 time units). Select A. Switch 162-165, A runs 165–180.  \
Time 180: A goes to I/O until 188. CPU idle.  \
Time 188: A returns (needs 35 time units). Select A. Switch 188-191, A runs 191–226.  \
Time 226: A finishes.  \
  
### Turnaround   \
A: 226 - 3 = 223  \
B: 112 - 7 = 105  \
C: 86 - 1 = 85  \
  
### WAiting  \
A: 3-31 (28) + 41–66 (25) + 69–86 (17) + 90–112 (22) = 92  \
B: 49-54 (5) = 5  \
C: 7-16 (9) = 9  \
  \
2.b.  \
  \
Time 1: C arrives. Select C. Switch 1-4, C runs 4–12. (queue: empty)  \
Time 3: A arrives. (queue: A)   \
Time 7: B arrives. (queue: A, B)  \
Time 12: C's quantum ends (7 time units left). Select A. Switch 12-15, A runs 15–23. (queue: B, C)  \
Time 23: A's quantum ends (37 time units left). Select B. Switch 23-26, B runs 26–32. (queue: C, A)  \
Time 32: B goes to I/O until 65. Select C. Switch 32-35, C runs 35–42. (queue: A)  \
Time 42: C goes to I/O until 52. Select A. Switch 42-45, A runs 45–53. (queue: empty)  \
Time 52: C returns (needs 10 time units). (queue: C)  \
Time 53: A's quantum ends (29 time units left). Select C. Switch 53-56, C runs 56–64. (queue: A)  \
Time 64: C's quantum ends (2 time units left). Select A. Switch 64-67, A runs 67–75. (queue: C)  \
Time 65: B returns (needs 9 time units). (queue: C, B)  \
Time 75: A's quantum ends (21 time units left). Select C. Switch 75-78, C runs 78–80. (queue: B, A)  \
Time 80: C goes to I/O until 95. Select B. Switch 80-83, B runs 83–91. (queue: A)  \
Time 91: B's quantum ends (1 time unit left). Select A. Switch 91-94, A runs 94–102. (queue: B)  \
Time 95: C returns (needs 14 time units). (queue: B, C)  \
Time 102: A's quantum ends (13 time units left). Select B. Switch 102-105, B runs 105–106. (queue: C, A)  \
Time 106: B goes to I/O until 130. Select C. Switch 106-109, C runs 109–117. (queue: A)  \
Time 117: C's quantum ends (6 time units left). Select A. Switch 117-120, A runs 120–128. (queue: C)  \
Time 128: A's quantum ends (5 time units left). Select C. Switch 128-131, C runs 131–137. (queue: A)  \
Time 130: B returns (needs 19 time units). (queue: A, B)  \
Time 137: C finishes. Select A. Switch 137-140, A runs 140–145. (queue: B)  \
Time 145: A goes to I/O until 155. Select B. Switch 145-148, B runs 148–156. (queue: empty)  \
Time 155: A returns (needs 15 time units). (queue: A)  \
Time 156: B's quantum ends (11 time units left). Select A. Switch 156-159, A runs 159–167. (queue: B)  \
Time 167: A's quantum ends (7 time units left). Select B. Switch 167-170, B runs 170–178. (queue: A)  \
Time 178: B's quantum ends (3 time units left). Select A. Switch 178-181, A runs 181–188. (queue: B)  \
Time 188: A goes to I/O until 196. Select B. Switch 188-191, B runs 191–194. (queue: empty)  \
Time 194: B finishes. CPU idle.  \
Time 196: A returns (needs 35 time units). Select A. Switch 196-199, A runs 199–234. Its quantum expires at 207, 215, 223, and 231, but the queue is empty each time, so it keeps running with no switch.  \
Time 234: A finishes.  \
 
## Turnaround  \
A: 234 - 3 = 231  \
B: 194 - 7 = 187  \
C: 137 - 1 = 136  \
  
## Waiting   \
A: 3-12 (9) + 23–42 (19) + 53–64 (11) + 75–91 (16) + 102–117 (15) + 128–137 (9) + 155–156 (1) + 167–178 (11) = 91  \
B: 7-23 (16) + 65–80 (15) + 91–102 (11) + 130–145 (15) + 156–167 (11) + 178–188 (10) = 78  \
C: 12-32 (20) + 52–53 (1) + 64–75 (11) + 95–106 (11) + 117–128 (11) = 54  \
  \
2.c.  \
Time 1: C arrives. Select C. Switch 1-4, C runs 4–19. (queue: empty)  \
Time 3: A arrives. (queue: A)  \
Time 7: B arrives. (queue: A, B)  \
Time 19: C goes to I/O until 29. Select A. Switch 19-22, A runs 22–52. (queue: B)  \
Time 29: C returns (needs 10 time units). (queue: B, C)  \
Time 52: A's quantum ends (15 time units left). Select B. Switch 52-55, B runs 55–61. (queue: C, A)  \
Time 61: B goes to I/O until 94. Select C. Switch 61-64, C runs 64–74. (queue: A)  \
Time 74: C goes to I/O until 89. Select A. Switch 74-77, A runs 77–92. (queue: empty)  \
Time 89: C returns (needs 14 time units). (queue: C)  \
Time 92: A goes to I/O until 102. Select C. Switch 92-95, C runs 95–109. (queue: empty)  \
Time 94: B returns (needs 9 time units). (queue: B)  \
Time 102: A returns (needs 15 time units). (queue: B, A)  \
Time 109: C finishes. Select B. Switch 109-112, B runs 112–121. (queue: A)  \
Time 121: B goes to I/O until 145. Select A. Switch 121-124, A runs 124–139. (queue: empty)  \
Time 139: A goes to I/O until 147. CPU idle.  \
Time 145: B returns (needs 19 time units). Select B. Switch 145-148, B runs 148–167. (queue: empty)  \
Time 147: A returns (needs 35 time units). (queue: A)  \
Time 167: B finishes. Select A. Switch 167-170, A runs 170–205. Its quantum expires at 200, but the queue is empty, so it keeps running.  \
Time 205: A finishes.  \
  
## Turnaround  \
A: 205 - 3 = 202  \
B: 167 - 7 = 16  \
C: 109 - 1 = 108  \
  
## Waiting  \
A: 3-19 (16) + 52–74 (22) + 102–121 (19) + 147–167 (20) = 77  \
B: 7-52 (45) + 94–109 (15) = 60  \
C: 29-61 (32) + 89–92 (3) = 35  \
  
