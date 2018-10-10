# eCS
embedded Coroutine Scheduler

  eCS是基于协程（Coroutine）技术的事件触发合作式调度器，各个任务自动处理堆栈，
而不用人工设定堆栈和处理堆栈，所有任务共享同一个堆栈空间，同时在实现与风格
近似于操作系统（Operating System，OS）。

  eCS支持3种类型的任务：时间触发型任务、（借鉴Protothreads思想的）线程型
（Thread）任务和FSM型任务，且都支持时间触发、消息和被优先调度一次排队。
  调度器相关文件：“eCS_cfg.h”、 “eCS.h”、 “eCS.c”
  1. 在工程主文件中包含“eCS.h”，其会自动包含“eCS_cfg.h”
  2. “eCS.c”和"eCS.h"已经设置为只读属性，无特殊情况不建议修改
  3. "eCS_cfg.h"则为开放给用户的接口，用于修订与CPU有关的项	
