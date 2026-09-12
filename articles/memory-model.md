
# Notes On Memory Models 

# One Core

SC - Single Core
```
        Memory
          |
          | read/write
          |
  CPU ----+
```
This is the Von Neuman architecture you are likely failiar when thinking about computers. We have a single CPU core, one RAM stick. To ease in I will begin with a theoretical simple computer, then, gradually make it more complex and hopefully closer to some theoretical model the author had in mind. 

Why make it more complex?

Because no one wants simple and slow machine, we all want fast even if complex machines, and today's computers are very very fast but complex.

A program:
```
x = 10
y = x
z = 20
```

The processor does:
```
write x, 10  // write 10 into x
    |
    V
read r1, x   // read x into a register
    |
    V
write y, r1  // write from the register to y
    |
    V
write z, 10 // write 20 into z
```

Nothing is confusing. 

This is the general mental model programmers have when writing simple code. But of-course hardware itself says nothing about how it will actually execute the program, instead it makes sure the final result is identical to the serially reading and executing the instructions. Regardless of what the hardware does, to a programmer it is as simple as "The CPU just reads instructions one by one and executes them". That being said, at some point if you care about pushing performance, you will have to deal with the consequences the hardware is trying to hide from you.

If so, let's list all of the assumptions we make about programs, and then as I introduce more complex CPU's how these assumptions are challanged. Here I will be talking about CPU's only. I am, for the time being, excluding GPU's and other architectures with a different set of assumptions. 

Assumptions: 

1. Correctness
    - Given some input, the output will always be the same
2. Strong order
    - The order of operations is the order given by the program

As far as I know, all CPU's today have a strong garantee on correcntess of it's instructions. If you want x+1 the result will not be x+2; the result will be _correct_ and you will see x+1. Some tasks might allow for relaxing this strong garantee on correctness. In Machine Learning where you can allow for small mistakes to happen and even sometimes encourage them, you can imagine a machines with relaxed garantee on correctness provide value with their "flaw". A 4bit floating point computation has worse precision than a 32bit one, but it is still used today because this precision loss is okay for machine learning applications. 

Strong order of instructions is assumed on a single core. From our perspective, we see linear instructions that get executed one after the other. See upper figure. When we have multiple cores the CPU can't enforce strong order, instead we allow the programmer to enforce strong order himself as needed. That said, there are still problems that we need to deal with when talking about multi-core CPU's.

## Cache

Logic in processors have seen great speed improvements over years, with memory lagging behind. 

If going to main memory is taking 200 steps and adding two numbers is just one step, you are bottlenecked by how fast can you get memory to work on, not the parts that do the work. 

So, let's introduce a smaller but faster place to store values. 

This place is called a *cache*. 
```
   Main Memory
       ^ 
       | 
    *Cache*           *Cache*
       ^                 ^
       |                 |
      CPU               CPU

-- 200 cycles --   -- 5 cycles --
```
Now, if I need value 'x', the CPU first checks whether it exists in the cache. If it does, great, it quickly gives me the value I wanted. Otherwise, it goes to main memory fetches the value, replicates to cache and gives it to me.

Replication gives us: 
    - Faster access to wanted values if in cache. 
    - Less traffic to RAM. 

Modern CPU's have multiple levels of cache to hide the latency of going to main memory. That is because they try to find a good balance between size and latency of the cache. As you increase size latency also goes up, having multiple layers of cache allows for better management of this trade-off. So the actual cache higherarchy looks more like: 
```
 [ Main Memory ...         ]
         ^^^^
         ||||
 [        L3       ]
 [  L2    ][   L2  ]
 [ L1 ]    [ L1 ]
 [ Core 1 ][ Core 2]
```

The cache has a way of management of what is inside of the cache and what is not. This management is done using a hash table if you are familiar with software terms. 

It will contain for example:
address,    tag,       data 
0x12345100, 'invalid', 0xcoffee



reading after reading

valid case: 

Fetch data
     | 
     v
Cache
    tag: vb (valid buffer)
    data: 0xcofee
     |                       *cache hit*
     v
    Done


Fetch data
     | 
     v
Cache
     |                       *cache miss*
     v
Main Memory
    tag: vb (valid buffer)
    data: 0xcofee
     |
     v
    Done


Writing: (invalidate all replicas, write)

modify (across all cores)
 tag: invalid buffer
    |
    v
modify (in cache)
 data: 0xbeef
    |                The cache is responble for updating main memory when it can, cpu is not stalled
    v
modify (in memory)  
 data: 0xbeef
   Done


read after write:

Fetch data (goes to cache first)
    |
    v
Cache
    tag: ib (invalid buffer)
    data: 0x00000
    |                 *communication miss*
    v
Main memory
    tag: vb
    data: 0xbeef



## Cache-Line

### Cache-Coherence Protocols

There are ways to make sure that across cache-lines we have coherence. These are called cache-coherence protocols. They can be quite complex, so I will look right now at a simple four-state MESI cache-coherence protocol.

### MESI

The state we will be dealing with are: 'modified', 'exclusive', 'shared', 'invalid'. 

modified - recent memory change, guaranteed to not appear in any other cpu's cache, responsible for write back to main memory before overriding the table entry and losing the data
exclusive - data has yet to be modified, main memory value is valid, and the data is owned by the cpu. It can modify the data at any point in time without caring about other cpu's. The cache might discard this data without write back to main memory. 
shared - data is replicated in at least one other cpu, it is not permitted to store anything into that line before consolting all the other cpu's. And just like the exclusive state, we can discard that data without writing back to main memory.
invalid - holds no data. When new data enters the cache, it preferably replaces invalid entries, since data with any other state might be requested by the cpu and result in a cache-miss. 

Since all CPUs must maintain a coherent view of the data carried in the cache lines, the cache-coherence protocol provides messages that coordinate the movement of cache lines through the system. 

### MESI Protocol Messages 

NOTE(ziv): caches can communicate one with the other

Read: The “read” message contains the physical address of the cache line to be read. 

Read Response: The “read response” message contains the data requested by an earlier “read” message. This “read response” message might be supplied either by memory or by one of the other caches. For example, if one of the caches has the desired data in “modified” state, that cache must supply the “read response” message. 

Invalidate: The “invalidate” message contains the physical address of the cache line to be invalidated. All other caches must remove the corresponding data from their caches and respond. 

Invalidate Acknowledge: A CPU receiving an “invalidate” message must respond with an “invalidate acknowledge” message after removing the specified data from its cache. 

Read Invalidate: The “read invalidate” message contains the physical address of the cache line to be read, while at the same time directing other caches to remove the data. Hence, it is a combination of a “read” and an “invalidate”, as indicated by its name. A “read invalidate” message requires both a “read response” and a set of “invalidate acknowledge” messages in reply. 

Writeback: The “writeback” message contains both the address and the data to be written back to memory (and perhaps “snooped” into other CPUs’ caches along the way). This message permits caches to eject lines in the “modified” state as needed to make room for other data. 


# Dealing with more cores

Now let's say we want a faster CPU.

There are many ways to make a CPU faster, but let's explore adding more cores. This is an obvious way anyone can just magically increase performance even a child can come up with.

DC - Dual core 

     Shared Memory
          | |
          | | read/write
          | |
 *CORE1* ---+ +--- *CORE2*

Here the two CPU cores have access to the same shared memory. 

If a single core wants to read some piece of memory, and another core wants to read a different part of memory, there is no problem. Both get to see the values they requested. If both want to read the same memory, that is also fine, they just get a copy of the same value. Only when we include writing do problems begin to arise.

I will make an analogy to a library. You are in a library with photoscanners. You take a book on the shelf and photoscan a copy for yourself. Another person enters the library and does the same. Both of you now have a copy and you can use for whatever you may want. It does not matter how many people do so. On the other hand if you write something on the book, the photoscan other people may take is now wrong, and they will need to return to the library and take another scan.  

## Race Condition

If core 1 writes to something, and core 2 writes or reads it, how do we know what order should this execute in? 

For example: 

```
Core 1: 
x = 1

Core 2:
x = 2
```
What result will get written out to memory 1 or 2?

We can think of this as a race: 

Core 1 might win
```
Core 1 ----x=1------> Memory
Core 2 ----x=2--->    Memory
```

But every once in a while core 2 might win 
```
Core 1 ----x=1---->   Memory
Core 2 ----x=2------> Memory
```

This type of bug is called a *race condition*. 

The effects of race conditions is programs that work correctly 99 times only to fail once, randomly. Race conditions and their non-obvious behavior, is what makes so hard to spot and debug. And so long as you need a deterministic output, taming this chaos becomes extremely important.

Another exmaple of a race condition: 

```
Core 1: 
x = 100

Core 2: 
print(x)
```

This is still considered a race condition, as you don't know what value will get printed: is it 0 or rather 100?

Now that we understand the problem, we are ready to ask how should we solve it. 

In other words: How can we enforce order?

### Lock

One solution is to give exclusive access to memory, one core at a time. Such that the entire memory can be written to only by a single core. Exclusive access is called a *lock*. 

#### What happens if the granularity is coarse? 

Core 1 -> [ Locked Memory ] -> write -> unlock
Core 2 ->       WAIT ----------------> lock -> [ Locked Memory ] -> write -> unlock

If we lock the entire memory per core we make everything easy because locking serializes. This is effectively acting like a single core with more steps killing any speed having more cores added. 

What can we do instead? 

We can divide RAM into sections you can claim access to. While Core 1 is working on section A, Core 2 may work on section B and so on... 

This is the idea behind splitting RAM into *granules*.

Instead of: 

lock EVERYTHING

We have: 

lock section A
lock section B 
lock section C
...

#### What happens if the granularity is fine? 

Each section is only 1 byte:

```
--------- RAM ---------- 
[A][B][C][D][E][F][G][H]
```

Now we have very percise control over over locking different memory, we allow more parallelism with the cores we have. 1 core can work on byte A, another on byte B, and so on...

This comes with a price: 

1. Tagging states becomes expensive. You may end up spending more memory on tagging than on the data itself. 
2. Tracking states becomes expensive. Tracking 
3. Poor locality. 


*Increased memory usage*. Each lock requires a bit of memory. Making memory granularity very fine requires many more locks, and so much more memory usage to implement those locks. 

*Increased complexity*. Now that we have many locks, we need to manage each and every one of them. This introduces more complexity in managing all of these locks.

Locking granularity is at the end of the day a trade-off. 

#### To summerize 

A lock is used to restric access to a shared resource to only one thread at a time, enforcing an order -- serilization. 

To make sure we don't revert back to single core performance, on tasks that don't share resources, RAM is split into granules we can access and modify simulatinously. This helps us keep more parallelism but increases complexity and memory usage.

TODO(ziv): Think about how to include this actually later. 
Here again I am simplifying. Even while locked, values can be read, and that has implications; but writing is never-the-less exclusive. That said, we will explore this behavior later. 

## Reader-Writer-Lock  







## Memory Coherance 

If core 1 writes a value and then core 2 reads it, when and in what order should core 2 see the change.

From here onwards assume 'x' is initialized to 0

For example:

```
Core 1: 
x = 1
y = 1

What is core 2 allowed to observe?

Can it see:

x = 1
y = 1

but also temporarily: 

x = 0
y = 1
```

Who tells us what behavior to expect?

The memory model tells us what observations are legal in these cases.

## Atomic Read/Write

Now let's consider what happens when one core wants to read something that another core is part-way done writing. 


Atomic operations -- These are operations that either happen completely, or don't happen at all. 



```
ld r1, [x]  // load x into register1 
st [x], 1   // store 1 into x
```





# How to syncronize threads but not loose all performance? - Readers-writers lock

What is a mutex?

A mutex (short for mutual exclusion) is a syncronization primitive in programming. It allows for takinng ownership over a resource by locking, and giving up ownership by unlocking. While a resource is locked, all other threads are blocked from accessing it via reading or writing. This ensures data integrity, and serial order of execution, helping prevent race conditions.  

Thread 1 -> [ Locked ] -> read -> unlock
Thread 2 ->    WAIT            -> [ Locked ] -> read -> unlock

Here thread 1 completely blocks any other thread wanting access to the shared resource they are trying to read. When thread 1 unlocks the resource, then and only then, can thread 2 lock it and read it. 

A Mutex solves syncronizing threads, but we lose performance. Since reading is non-distructive, thus, can be done safely by multiple threads, we would like to read in parallel. Writing on the other hand is distructive. Any modification done has to flow through the memory system and update all threads. Otherwise, threads will read a wrong value, and we end up with a race condition. A synchronization tool called a reader-writer lock is the answer to our problem. It allows many readers share access while keeping writers completely separate. 

First Solution: *readers-preference*

In the solution, we want readers to lock the resource from writers but, allow for more readers to still read in parallel. After all readers finished, we allow unlock the resource and allow readers/writers to lock again. Note that in this solution, every writer must claim the resource individually. 

```
Thread 1 --- read -> ...                           R  
Thread 2 --- read -> ...                           R
Thread 3 ------ read -> unlock                     R
Thread 4 ---- WAIT -----------> write -> unlock    W
[------][--][ R-Lock ------- ][ W-Lock T4 --]    [REQ]

R-Lock    - only readers can access the resource. 
W-Lock TX - only a single writer (Thread X) can access the resource. 
REQ       - request type R/W
```

This first solution has a downside in that it can result in resource starvation, where it becomes impossible to write to a file if it is constantly being read from. -- [wiki](https://en.wikipedia.org/wiki/Readers%E2%80%93writers_problem)

For that reason, we want to make sure that readers don't keep on reading if there is a writer, unless absolutely neccessary.
```
R R R R R R R R R R R
                    │
                    v
                    W
                    ↑
                keeps waiting
```

Second Solution: *writers-preference*

In this solution we force readers to release their lock if a writer wants to write. This makes it possible to write even if multiple readers want to read at the same time and hold the lock. 

```
Thread 1 --- read -> ...                                                         R    
Thread 2 ---- read -> unlock (*forced* writer wants to write)                    R 
Thread 4 --- WAIT ---------> write -> unlock                                     W
Thread 5 ----------------- WAIT -----------> write -> unlock                     W 
Thread 3 ------- WAIT --------------------------------------> read -> unlock     R
[------][--][ R-Lock -------][W-Lock T4 ----][W-Lock T5 -----][R-Lock ------]  [REQ]
```

Now we don't starve the writers, but we might starve the readers. If many writers one after the other are trying to write, readers will not get their opoprtunity to read. If this becomes a problem, it might be that we don't want to prefer readers or writers, instead we want fairness. 
```
W W W W W W W W W W
                    │
                    ▼
                    R
                    ↑
            keeps waiting
```
Third Solution: *no-preference*

_Make sure threads can lock, and do so in bounded time._

The third solution creates a queue that services all threads in a first in first out (FIFO) order. It allows readers that are queued one after the other to be run in parallel, but, on a writer it forces a release of the lock. This solution on the one hand assumes the least about this problem, and creates a mechanism to be better than the naive implementation, but in certain cases will be slower than the first or second solutions. 
```
                THREADS ARRIVE
                       │
                       ▼
            ┌─────────────────────┐
            │     FIFO QUEUE      │
            │                     │
            │ R → R → W → R → R   │
            │                     │
            └──────────┬──────────┘
                       │
                       ▼
                Examine FRONT
                       │
            ┌──────────┴────────────┐
            │                       │
         Reader?                 Writer?
            │                       │
            ▼                       ▼
    More readers            Wait for ALL
    immediately             current readers
    behind it?              to finish
            │                       │
            ▼                       ▼
    ┌───────────┐          ┌───────────┐
    │ RUN READERS│          │ RUN WRITER│
    │ IN PARALLEL│          │ ALONE     │
    └─────┬─────┘          └─────┬─────┘
            │                      │
            └──────────┬───────────┘
                    │
                    ▼
                NEXT FIFO ENTRY
```

The result is:
```
R R R     W     R R       W      R R R
─────     ─     ───       ─      ─────
↑         ↑       ↑       ↑        ↑
parallel  alone  parallel alone  parallel
```



# Addenum 




What everything means: 

Race Conditions - Multiple threads r/w to a shared resource and race to be the first to modify/read leading to unknown behavior.
Memory granules - Memory is split into sections it manages locks for.
Locking - Mechanism for giving ownership over a resource. Preventing other threads access to the resource. By doing so: enforcing order -- serilization.

Memory consistency model - The memory model tells programmers what kinds of observations are legal.


Cache - Smaller faster memory & replication 
Cache Line - Smallest management unit of memory the cpu handles. Both coherence unit, and blocks size for data movement.
Coherence unit - Granule size for locking
Cache coherence - Rules for how to keep all the replicas across memory, caches updated.
Memory Coherance - 
Memory consistency -

Coherence vs consistency


Atomics & writing atomicly.

MCA - multi-copy atomicity
nMCA - non-multi-copy atomicity

MCA = "A write becomes a shared fact." nMCA = "A write can become a fact for CPU A before it's a fact for CPU B."


RMCA - read-own-write-early
OMCA - other-multi-copy atomic


invalidation buffer
Deferred invalidation
sequential consistency (SC)

Out-of-order execution vs. memory reordering
This distinction is VERY important.

TODO(ziv): semaphore check what is it!!!!! 

TODO(ziv): Look at he default behavior of VSCODE for search and try to implemenet it in 4coder