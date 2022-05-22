# 半同步半异步线程池

## 原理

半同步半异步线程池分为三层：

- 同步服务层：处理来自上层的任务请求
- 同步排队层：来自上层的任务请求都会追加到排队层中等待处理
- 异步服务层：多个线程同时处理排队层中的任务

线程池执行任务的活动图

```mermaid
stateDiagram
direction LR
A: 启动一定数量线程
B: 等待每个线程退出
C: 轮询队列
D: 等待任务
E: 从队列中取出一个任务执行
F: 直到不为空时激活
if_state1: 是否停止
if_state2: 同步队列是否为空
[*] --> A
A --> if_state1
if_state1 --> B: 是
if_state1 --> C: 否
B --> [*]
C --> if_state2
if_state2 --> D: 是
if_state2 --> E: 否
D --> F
F --> E
E --> C
```

往线程池添加任务的活动图

```mermaid
stateDiagram
direction LR
if_state: 队列是否达到上限
A: 等待任务减少
B: 添加任务
C: 直到队列没有达到上限时
[*] --> if_state
if_state --> A: 是
if_state --> B: 否
A --> C
C --> B
B --> [*]
```

## 测试

我在 `main.cpp` 中添加了 100000 个线程对同一个数进行加操作，每个线程加 100 次。并在这些线程中使用互斥锁

在当前目录下执行

```bash
make run
```

得到如下输入

```bash
total: 10000000
```

## 参考

- 深入应用 C++11
