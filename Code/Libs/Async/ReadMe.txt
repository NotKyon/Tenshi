ASYNC
=====

The Async library manages and aids in parallelism. Various synchronization
primitives are offered, as well as atomic operations and a task scheduling
system.


CPU INFORMATION
---------------
(See CPU.hpp)

You can retrieve the number of processing cores and hardware threads through
this system.


THREADING
---------
(See CThread.hpp)

Threads are effectively separate processes that share the same memory and most
of the same resources as the program they are spawned from. Each thread gets its
own stack and execution context.

You can create your own thread by instantiating a class derived from the
Ax::Async::CThread base class then calling Ax::Async::CThread::Start() on that
class.

You can signal a thread to stop execution by calling Ax::Async::CThread::Stop().
Inside of the overridden Ax::Async::CThread::OnRun() method, just check the
return-value of the Ax::Async::CThread::IsQuitting() method to determine when
it's time to exit the thread. (Returning from the function causes the thread to
exit.)


ATOMIC OPERATIONS
-----------------
(See Atomic.hpp)

Atomic (also known as "interlocked") operations are supported through this
system.

CAUTION: If you're familiar with InterlockedIncrement() and
-        InterlockedDecrement() then you should be aware that they return the
-        new value after the operation. AtomicInc() and AtomicDec() return the
-        value prior to the operation.


SYNCHRONIZATION: MUTEX
----------------------
(See CMutex.hpp)

Provides exclusive access to a resource, per-thread. The mutex provided is a
"recursive mutex." A recursive mutex can be locked multiple times by the same
thread without issue. (However, each lock needs a corresponding unlock.)

Call the Ax::Async::CMutex::Lock() method on the mutex to gain access to that
resource. When you're done with the resource call Ax::Async::CMutex::Unlock().
Each Lock() must have a corresponding Unlock() call.

It is less error-prone to use the Ax::Async::MutexGuard class. Its constructor
takes a reference to the mutex you want to acquire, which then locks that mutex.
Upon destruction of the MutexGuard instance, the mutex will be unlocked. This is
particularly helpful in functions that have many return points or that throw
exceptions (or call functions which might).


SYNCHRONIZATION: SIGNAL
-----------------------
(See CSignal.hpp)

Signals are effectively boolean flags. They can be triggered, which will cause
a thread waiting for that signal to resume execution.


SYNCHRONIZATION: BARRIER
------------------------
(See CBarrier.hpp)

These act very similarly to Signals. Instead of being triggered immediately upon
being signalled, they will only be triggered after a certain number of calls.

You call the Ax::Async::CBarrier::Raise() method on the barrier to increment its
"current level." Once the current level matches (or exceeds) the "target level,"
the barrier will be in a triggered state, which allows any threads waiting for
it to continue execution.

The "target level" can be specified in the barrier's constructor or the
Ax::Async::CBarrier::Reset() method. The target level can also be incremented
through the Ax::Async::CBarrier::IncrementTarget() method.


SYNCHRONIZATION: R/W LOCK
-------------------------
(See CRWLock.hpp)

Read/Write Locks allow read-access to multiple readers when no writer has
access, but exclusive access to writers.

The Ax::Async::CRWLock::ReadLock() and Ax::Async::CRWLock::ReadUnlock() methods
are used to lock and unlock read-access, respectively. ReadLock() will force the
calling thread to wait if a writer has access.

The Ax::Async::CRWLock::WriteLock() and Ax::Async::CRWLock::WriteUnlock() methods
are used to lock and unlock exclusive-access, respectively. WriteLock() will
force the calling thread to wait until there are no readers or writers with
access.


SYNCHRONIZATION: SEMAPHORE
--------------------------
(See CSemaphore.hpp)

Provides access to a set of resources. Internally, semaphores hold a current
count and a maximum count. When a semaphore's count is above zero it is in the
signalled-state. When a thread waits on a semaphore, it is waiting for the
semaphore to be signalled. Upon signalling, the wait operation will also
decrement the current count of the semaphore by one.

Semaphores can be used to limit the number of accessors to a resource. If for
example, you have an array of a given resource that is periodically added to,
and other threads that consume items from that array, you can use a semaphore to
wake-up threads when they need to access items from the array.


SCHEDULER AND TASK QUEUEUS
--------------------------
(See MScheduler.hpp)

The scheduler is responsible for distributing work (in the form of TaskQueues)
to a set of worker threads which it also manages. The scheduler is a singleton.
To get its instance call Ax::Async::TASKS().

TaskQueues (Ax::Async::TaskQueue) provide a set of jobs and synchronization
points which are to be executed. Each job has a corresponding "signal." When all
jobs corresponding to the given signal have completed execution, that signal is
then triggered. A synchronization point, called a "fence" will prevent further
execution of the queue until its corresponding signal is triggered. This enables
dependency management within a queue.

To use the scheduler it has to first be initialized. Upon initialization, the
scheduler instantiates a number of worker threads. Each worker thread consumes
and executes tasks submitted to the scheduler.

Calling the Ax::Async::TaskQueue::Submit() method on a queue will finalize a
TaskQueue then send it to the scheduler. The scheduler in turn notifies each of
its managed worker threads of the TaskQueue.

TaskQueues can also be given a latency setting upon submission. The options are:

	Ax::Async::TaskQueue::Latency::Unlimited

		No limit to how long this TaskQueue takes to complete.

	Ax::Async::TaskQueue::Latency::ThisFrame

		The TaskQueue will be forced to complete on the next call to
		Ax::Async::MScheduler::StepFrame(). (0-frame latency.)

	Ax::Async::TaskQueue::Latency::NextFrame

		The TaskQueue will be forced to complete on the call after the next to
		Ax::Async::MScheduler::StepFrame(). (1-frame latency.)

The latency of the TaskQueue does not affect its priority, so prior to
submitting the TaskQueue its priority should be set. See

	void Ax::Async::TaskQueue::SetPriority( priority_t priority );

TaskQueues with higher priority will be executed before task queues of lower
priority.

