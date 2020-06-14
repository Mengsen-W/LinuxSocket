# Memory Order

## 1. 六种内存序
- ### std::memory_order_relaxed（__ATOMIC_RELAXED）
  - 只保证当前操作的原子性，其他线程可能读到新值也可能读到旧值
- ### std::memory_order_acquire （__ATOMIC_ACQUIRE）
  - 对读取施加 acquire 语义（load），在代码中这条语句后面所有读写操作都无法重排到这个操作之前，即 load-store 不能重排为 store-load, load-load 也无法重排为 store-store
  - 在这个原子变量上施加 release 语义的操作发生之后，acquire 可以保证读到所有在 release 前发生的写入
- ### std::memory_order_release （__ATOMIC_RELEASE）
  - 对写入施加 release 语义（store），在代码中这条语句前面的所有读写操作都无法被重排到这个操作之后，即 store-store 不能重排为 store-store, load-store 也无法重排为 store-load
  - 当前线程内的所有写操作，其他对这个原子变量进行 acquire 的线程可见
  - 当前线程内的与这块内存有关的所有写操作，其他对这个原子变量进行 consume 的线程可见
- ### std::memory_order_consume（__ATOMIC_CONSUME）
  - 对当前要读取的内存施加 release 语义（store），在代码中这条语句后面所有与这块内存有关的读写操作都无法被重排到这个操作之前
  - 在这个原子变量上施加 release 语义的操作发生之后，acquire 可以保证读到所有在 release 前发生的并且与这块内存有关的写入
- ### std::memory_order_acq_rel （__ATOMIC_ACQ_REL）
  - 对读取和写入施加 acquire-release 语义，无法被重排
  - 可以看见其他线程施加 release 语义的所有写入，同时自己的 release 结束后所有写入对其他施加 acquire 语义的线程可见
- ### std::memory_order_seq_cst （__ATOMIC_SEQ_CST）
  - 如果是读取就是 acquire 语义，如果是写入就是 release 语义，如果是读取+写入就是 acquire-release 语义
  - 同时会对所有使用此 memory order 的原子操作进行同步，所有线程看到的内存操作的顺序都是一样的，就像单个线程在执行所有线程的指令一样

1. 动态内存模型可理解为存储一致性模型，主要是从行为(behavioral)方面来看多个线程对同一个对象同时(读写)操作时(concurrency)所做的约束
2. 现在的编译器在编译程序时通常将指令重新排序。对编译器和 CPU 作出一定的约束才能合理正确地优化你的程序。C++程序员要想写出高性能的多线程程序必须理解内存模型，编译器会给你的程序做优化(静态)，CPU为了提升性能也有乱序执行(动态)，总之，程序在最终执行时并不会按照你之前的原始代码顺序来执行，因此内存模型是程序员、编译器，CPU 之间的契约，遵守契约后大家就各自做优化，从而尽可能提高程序的性能。
3. 这个原子操作为中心，其前后的代码。这些代码并不一定需要是原子操作，只是普通的代码就行。
