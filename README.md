
# eCS

## embedded Coroutine Scheduler
   eCS is a lightweight event triggered cooperative scheduler. 
Each thread automatically processes the stack, instead of manually setting 
and processing the stack. All threads share the same stack space, and the implementation 
and style are similar to the operating system.  
   eCS supports three types of threads: time triggered threads, coroutine threads 
(likes protothreads) and FSM threads, all of which support time triggering, message 
and priority scheduling.  
   Relevant documents of dispatcher: "eCS_cfg.h", "eCs.h", "eCS.c".  
   1.Include "eCS.h" in the project main file, which automatically includes "eCS_cfg.h".  
   2."eCS.c" and "eCs.h" have been set as read-only properties, and modification is not   
recommended without special circumstances.  
   3. "eCS_cfg.h" is an open interface for users to revise CPU related items.
    
==============================================================================

   eCS是一个轻量级的事件触发合作式调度器，各个线程自动处理堆栈，而不用人工设定堆栈和处理堆栈，所有线程共享同一个堆栈空间，同时在实现与风格近似于操作系统。  
   eCS支持3种类型的线程：时间触发型线程、（借鉴Protothreads思想的）协程型线程和FSM型线程，且都支持时间触发、消息和优先级调度。  
   调度器相关文件："eCS_cfg.h"、"eCS.h"、"eCS.c":  
   1. 在工程主文件中包含“eCS.h”，其会自动包含“eCS_cfg.h”；  
   2. “eCS.c”和"eCS.h"已经设置为只读属性，无特殊情况不建议修改；  
   3. "eCS_cfg.h"则为开放给用户的接口，用于修订与CPU有关的项。	
