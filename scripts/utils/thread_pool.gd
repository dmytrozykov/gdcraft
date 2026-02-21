## A fixed-size pool of background threads that executes submitted [Callable] tasks.
##
## Workers sleep via a [Semaphore] when the queue is empty, so there is no
## spin-waiting. Tasks are executed in FIFO order. The pool is not
## automatically shut down — call [method shutdown] before freeing it.
##
## [codeblock]
## var pool = ThreadPool.new(4)
## var ticket = pool.submit(func(): generate_chunk(pos))
## ticket.cancel()   # no-op if already running; skips it if still queued
## pool.shutdown()   # blocks until all workers finish
## [/codeblock]
class_name ThreadPool

## Handle returned by [method submit].
## Cancellation is cooperative: a task that has already started cannot be
## interrupted — only tasks still waiting in the queue will be skipped.
class TaskTicket:
	var _cancelled = false

	## Cancel this task. Safe to call from any thread.
	func cancel() -> void:
		_cancelled = true

	## Returns [code]true[/code] if [method cancel] has been called.
	func is_cancelled() -> bool:
		return _cancelled

var _threads: Array[Thread] = []
var _queue: Array = [] # entries are [Callable, TaskTicket]
var _mutex = Mutex.new()
var _semaphore = Semaphore.new()
var _running = true

## Creates the pool and starts [param thread_count] worker threads.
## Defaults to [method OS.get_processor_count].
func _init(thread_count: int = OS.get_processor_count()) -> void:
	for i in thread_count:
		var t = Thread.new()
		t.start(_worker)
		_threads.append(t)

## Enqueues [param task] and returns a [TaskTicket] that can cancel it.
func submit(task: Callable) -> TaskTicket:
	var ticket = TaskTicket.new()
	_mutex.lock()
	_queue.append([task, ticket])
	_mutex.unlock()
	_semaphore.post()
	return ticket

## Signals all workers to stop and waits for them to finish.
## Must be called before the pool is freed to avoid thread leaks.
func shutdown() -> void:
	_running = false
	# Wake every blocked worker so they can see _running is false and exit.
	for i in _threads.size():
		_semaphore.post()
	for t in _threads:
		t.wait_to_finish()
	_threads.clear()

func _worker() -> void:
	while true:
		_semaphore.wait()
		if not _running:
			break
		_mutex.lock()
		# Guard against spurious wakes with an empty queue.
		if _queue.is_empty():
			_mutex.unlock()
			continue
		var entry: Array = _queue.pop_front()
		_mutex.unlock()
		var task: Callable = entry[0]
		var ticket: TaskTicket = entry[1]
		if not ticket._cancelled:
			task.call()
