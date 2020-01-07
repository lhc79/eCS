/*********************************Copyright(c)**********************************
** File Name:			eCS.h
** Author:				刘海成
** Copyright:			刘海成        
** author email:		liuhaicheng@126.com
** version:             V2.78
** Latest modified Date:2019-1-22

** Description:embedded Coroutine Scheduler  
   协程(Coroutine)原理调度器(scheduler):
       eCS是基于协程（Coroutine）技术的事件触发合作式调度器，各个任务自动处理堆
   栈，而不用人工设定堆栈和处理堆栈，所有任务共享同一个堆栈空间，同时在实现与风
   格近似于操作系统（Operating System，OS）。
       eCS支持三种类型的线程：
       (1)时间触发型线程
       (2)（借鉴Protothreads思想的）LOOP型线程
       (3)FSM型线程
   三种线程具有相互嵌套机制，且都支持时间触发、消息和插队式优先调度。
 
   调度器相关文件：“eCS_cfg.h”、 “eCS.h”、 “eCS.c”
   1. 在工程主文件中包含“eCS.h”，其会自动包含“eCS_cfg.h”
   2. “eCS.c”和"eCS.h"已经设置为只读属性，无特殊情况不建议修改
   3. "eCS_cfg.h"则为开放给用户的接口，用于修订与CPU有关的项				 
******************************************************************************/

#ifndef __ECS_H__
#define __ECS_H__

#include "eCS_cfg.h"


#define	eCS_SUCCESS              1
#define	eCS_FAIL                 0

#define	eCS_Priority_0           0
#define	eCS_Priority_1           1 
#define	eCS_Priority_2           2
#define	eCS_Priority_3           3

#define _eCS_TaskState_Closed     0
#define _eCS_TaskState_Suspended  0xff

extern  volatile _eCS_UINT32  eCS_Ticks;
extern _eCS_UINT8  _eCS_runTaskSum;
extern _eCS_UINT8  _eCS_TaskSum;


typedef _eCS_UINT32           _eCS_DLY_TYPE; 	    

#define _eCS_Timetrigger_Type    23
#if !((_eCS_Timetrigger_Type == 23) || (_eCS_Timetrigger_Type == 24) || (_eCS_Timetrigger_Type == 27))
#error "_eCS_Timetrigger_Type = 23,24,27 must chose one!"     
#endif

#if   _eCS_Timetrigger_Type == 27
 #define _eCS_setTimeTrigger(pTS, Ticks)   (pTS)->Timetrigger_Tick = (Ticks)
 #define _eCS_TimeTriggerFail()            (_eCS_this->Timetrigger_Tick)
#elif _eCS_Timetrigger_Type == 24
 #define _eCS_setTimeTrigger(pTS, Ticks) do{(pTS)->Timetrigger_Tick = eCS_Ticks;   \
                                           (pTS)->Timetrigger_Length = (Ticks);}while(0)
 #define _eCS_TimeTriggerFail()            ((eCS_Ticks - _eCS_this->Timetrigger_Tick) < _eCS_this->Timetrigger_Length)
#elif _eCS_Timetrigger_Type == 23
 #define _eCS_setTimeTrigger(pTS, Ticks)   (pTS)->Timetrigger_Tick = eCS_Ticks + (Ticks)
 #define _eCS_TimeTriggerFail()            (((_eCS_INT32)(eCS_Ticks - _eCS_this->Timetrigger_Tick)) < 0)
#else
#endif

#define eCS_ms(ms)          ((ms) / eCS_SYSTICK_MS)
#define eCS_second(s)       (((s) * 1000) / eCS_SYSTICK_MS)
#define eCS_minute(m)       (((m) * (60 *1000)) / eCS_SYSTICK_MS)
#define eCS_hour(h)         (((h) * (60 * 60 *1000)) / eCS_SYSTICK_MS)
#define eCS_date(d)         (((d) * (24 * 60 * 60 *1000)) / eCS_SYSTICK_MS)

#if _eCS_CompilerIsGUN_C99 == 0
    typedef _eCS_UINT16      _eCS_RunLine_TYPE;
#else
    typedef void  *_eCS_RunLine_TYPE;
#endif

#if _eCS_CompilerIsGUN_C99 == 0
    #define _eCS_CURR_LINE                  (_eCS_RunLine_TYPE)(__LINE__+(!__LINE__))
    #define _eCS_LABEL                      case _eCS_CURR_LINE
#else
    #define _eCS_StringMerger2(str1, str2)	str1##str2
    #define _eCS_StringMerger(str1, str2)	_eCS_StringMerger2(str1, str2)
    #define _eCS_CURR_LINE                  (&&_eCS_StringMerger(eCS_BP, __LINE__))
    #define _eCS_LABEL                      _eCS_StringMerger(eCS_BP, __LINE__)
#endif


typedef void (*_eCS_TaskType)(void);

typedef struct _eCS_TCB                              
{    
    _eCS_UINT8                   Priority; 
    _eCS_UINT8                   PriorityCounter;
	
#if _eCS_SaveRAM == 0   
    struct _eCS_TCB              *pPreviousTS;
#endif
    struct _eCS_TCB              *pNextTS;

    _eCS_TaskType                pTask; 
    _eCS_DLY_TYPE                Timetrigger_Tick;
#if _eCS_Timetrigger_Type == 24
    _eCS_DLY_TYPE                Timetrigger_Length;
#endif
    _eCS_RunLine_TYPE            RunLine; 
    
    union 
    {
        _eCS_RunLine_TYPE       RunLine_Context; 
		    _eCS_SysTick_usAccType  SysTickNeedCounted;
		    _eCS_UINT16             Timeout_times; 
		    _eCS_DLY_TYPE           Timeout_Ticks;
    }temp1; 
    union    
    {
        _eCS_TaskType           pTask_Context; 
		    _eCS_SysTick_usReadType SysTickCounterOldVAL;
    }temp2;
} eCS_TS;

extern  eCS_TS  *_eCS_this;

#define def_eCS_TS(TS_name)     eCS_TS  TS_name = {0}

#if _eCS_CompilerIsGUN_C99 == 0
 #define eCS_Begin();   switch(_eCS_this->RunLine){ case 0:
#else
 #define eCS_Begin()    if (_eCS_this->RunLine != (_eCS_RunLine_TYPE)0) goto *_eCS_this->RunLine
#endif

#if _eCS_CompilerIsGUN_C99 == 0
#define eCS_End()       }                                                       \
                        _eCS_this->RunLine = (_eCS_RunLine_TYPE)0
#else
#define eCS_End()       _eCS_this->RunLine = (_eCS_RunLine_TYPE)0
#endif

#define eCS_SubBegin()  static _eCS_TaskType        pTask_Context;              \
                        static _eCS_RunLine_TYPE    RunLine_Context;            \
                        eCS_Begin();                                            \
                        RunLine_Context = _eCS_this->temp1.RunLine_Context;     \
                        pTask_Context   = _eCS_this->temp2.pTask_Context

#if _eCS_CompilerIsGUN_C99 == 0
 #define eCS_SubEnd()   }                                                       \
                        _eCS_this->RunLine = RunLine_Context;                   \
                        _eCS_this->pTask = pTask_Context
#else
 #define eCS_SubEnd()   _eCS_this->RunLine = RunLine_Context;                   \
                        _eCS_this->pTask = pTask_Context
#endif

#define eCS_SubFSMBegin()  static _eCS_TaskType    pTask_Context;              \
                        static _eCS_RunLine_TYPE    RunLine_Context;            \
                        if (_eCS_this->RunLine == (_eCS_RunLine_TYPE)0)         \
                        {   RunLine_Context = _eCS_this->temp1.RunLine_Context; \
                            pTask_Context   = _eCS_this->temp2.pTask_Context					

#define eCS_SubFSMEnd() }                                                       \
                        _eCS_this->RunLine = RunLine_Context;                   \
                        _eCS_this->pTask = pTask_Context


#define eCS_Thread(vThreadName)      void vThreadName(void)
#define eCS_LOOP(vThreadName)        eCS_Thread(vThreadName)
#define eCS_TT(TT_ThreadFunction) eCS_Thread(TT_ThreadFunction)
#define eCS_SubThread(SubThreadName) eCS_Thread(SubThreadName)
#define eCS_State(StateFunction)                    eCS_Thread(StateFunction)

#define eCS_StateTurnTo(pNextStateFunction, Ticks)  do{ _eCS_this->pTask = pNextStateFunction;     \
                                                        _eCS_this->RunLine = (_eCS_RunLine_TYPE)0; \
                                                        _eCS_setTimeTrigger(_eCS_this, Ticks);     \
                                                        return;                                    \
                                                      }while(0)

#define eCS_StateTransferTo(pNextStateFunction)     do{ _eCS_this->pTask = pNextStateFunction;     \
                                                        _eCS_this->RunLine = (_eCS_RunLine_TYPE)0; \
                                                        return;                                    \
                                                      }while(0)

#define eCS_Yield()                 do{ _eCS_this->RunLine = _eCS_CURR_LINE; return; _eCS_LABEL: ; \
                                      }while(0)
#define eCS_YieldWait(Ticks)        do{ _eCS_setTimeTrigger(_eCS_this, Ticks);         \
                                        eCS_Yield();                                   \
                                      }while(0)

#define eCS_YieldWhile(condition, Ticks) do{ _eCS_setTimeTrigger(_eCS_this, Ticks);    \
                                             eCS_Yield();                              \
                                             if(condition){                            \
                                                _eCS_setTimeTrigger(_eCS_this, Ticks); \
                                                return;                                \
                                             }                                         \
                                           }while(0) 

#define eCS_YieldUtill(condition, Ticks)  eCS_YieldWhile(!(condition), Ticks)
 
#define eCS_YieldWhen(condition)     do{ eCS_Yield();                               \
                                         if(condition){ return; }                   \
                                       }while(0) 

#define eCS_YieldUnless(condition)   eCS_YieldWhen(!(condition))

#define eCS_YieldWhile_TimeoutControl(condition, Ticks, times, dealwith)                \
                                     do{ _eCS_setTimeTrigger(_eCS_this, Ticks);         \
                                         _eCS_this->temp1.Timeout_times = times;        \
                                         eCS_Yield();                                   \
                                         if(condition){                                 \
                                             _eCS_setTimeTrigger(_eCS_this, Ticks);     \
                                             if(--_eCS_this->temp1.Timeout_times == 0)  \
                                             {  do{dealwith;}while(0);                  \
												_eCS_this->temp1.Timeout_times = times; \
                                             }                                          \
                                             return;                                    \
                                         }                                              \
                                        }while(0) 

#define eCS_YieldUtill_TimeoutControl(condition, Ticks, times, dealwith)   eCS_YieldWhile_TimeoutControl(!(condition), Ticks, times, dealwith)

#define eCS_YieldWhen_TimeoutControl(condition, Ticks, dealwith)                       \
								 do{ _eCS_setTimeTrigger(_eCS_this, 0);                \
									 _eCS_this->temp1.Timeout_Ticks = eCS_Ticks;       \
									 eCS_Yield();                                      \
									 if(condition){                                    \
										 if((eCS_Ticks - _eCS_this->temp1.Timeout_Ticks) >= (Ticks)){ \
											 do{dealwith;}while(0);                    \
											 _eCS_setTimeTrigger(_eCS_this, 0);        \
											 _eCS_this->temp1.Timeout_Ticks = eCS_Ticks;  \
										 }                                             \
										 return;                                       \
									 }                                                 \
								 }while(0) 
#define eCS_YieldUnless_TimeoutControl(condition, Ticks, dealwith)  eCS_YieldWhen_TimeoutControl(!(condition), Ticks, dealwith)

#define eCS_CallSub(SubTaskName)	do{ _eCS_this->temp2.pTask_Context = _eCS_this->pTask;     \
										_eCS_this->pTask = SubTaskName;						   \
										_eCS_this->RunLine = (_eCS_RunLine_TYPE)0; 			   \
										_eCS_this->temp1.RunLine_Context = _eCS_CURR_LINE; SubTaskName(); return; _eCS_LABEL: ;	\
									} while(0)

#define eCS_SubReturn() 			do{ _eCS_this->RunLine = RunLine_Context;                  \
										_eCS_this->pTask   = pTask_Context;                    \
										return;                                                \
									} while(0)

#define eCS_CallFSM(subFSM_IDLE_Name)    eCS_CallSub(subFSM_IDLE_Name) 

#define eCS_SubFSMReturn(pFSM_IDLE_Name) do{_eCS_this->RunLine = (_eCS_RunLine_TYPE)1;     \
                                            _eCS_this->pTask = pFSM_IDLE_Name;             \
                                            return;                                        \
                                           } while(0)

#define eCS_StateWaitWhile(condition, Ticks) do{ if(condition){					           \
                                                    _eCS_setTimeTrigger(_eCS_this, Ticks); \
                                                    return;						           \
                                                 }								           \
                                               } while(0)
#define eCS_StateWaitUtill(condition, Ticks)	eCS_StateWaitWhile(!(condition), Ticks)
#define eCS_StateWaitWhen(condition)	        do{ if(condition){ return; } } while(0)
#define eCS_StateWaitUnless(condition)          eCS_StateWaitWhen(!(condition))


#define eCS_StateWaitWhile_TimeoutControl(condition, Ticks, times, dealwith)                   \
                                     do{ if(condition){                                        \
                                             if(_eCS_this->RunLine == (_eCS_RunLine_TYPE)0)    \
                                             {  _eCS_this->RunLine = (_eCS_RunLine_TYPE)1;     \
                                                _eCS_setTimeTrigger(_eCS_this, Ticks);         \
                                                _eCS_this->temp1.Timeout_times = times;        \
                                             }                                                 \
                                             else{                                             \
                                                _eCS_setTimeTrigger(_eCS_this, Ticks);         \
                                                if(!(--_eCS_this->temp1.Timeout_times))        \
                                                {  do{dealwith;}while(0);                      \
                                                   _eCS_this->temp1.Timeout_times = times;     \
                                                }                                              \
                                             }                                                 \
                                             return;                                           \
                                         }                                                     \
										}while(0)
#define eCS_StateWaitUtill_TimeoutControl(condition, Ticks, times, dealwith)   eCS_StateWaitWhile_TimeoutControl(!(condition), Ticks, times, dealwith)
	
#define eCS_StateWaitWhen_TimeoutControl(condition, Ticks, dealwith)                           \
                                     do{ if(condition){                                        \
                                           if(_eCS_this->RunLine == (_eCS_RunLine_TYPE)0){     \
                                             _eCS_setTimeTrigger(_eCS_this, Ticks);            \
											 _eCS_this->temp1.Timeout_Ticks = eCS_Ticks;       \
                                             _eCS_this->RunLine = (_eCS_RunLine_TYPE)1;        \
                                           }                                                   \
                                           else{                                               \
                                             if((eCS_Ticks - _eCS_this->temp1.Timeout_Ticks) >= (Ticks)){ \
                                                do{dealwith;}while(0);                         \
                                                _eCS_setTimeTrigger(_eCS_this, Ticks);         \
												_eCS_this->temp1.Timeout_Ticks = eCS_Ticks;    \
                                             }                                                 \
                                           }                                                   \
                                           return;                                             \
                                         }                                                     \
                                    }while(0)
#define eCS_StateWaitUnless_TimeoutControl(condition, Ticks, dealwith)  eCS_StateWaitWhen_TimeoutControl(!(condition), Ticks, dealwith) 


extern _eCS_UINT8 eCS_TaskCreate(eCS_TS       	*pNewTS,
								 void          (*pNewTask)(void),
								 _eCS_DLY_TYPE	 Ticks,
                                 _eCS_UINT8      Priority);
												
extern void _eCS_SysRestart( void(*pUserTasks_Create)(void) );

#define eCS_SysRestart(pUserTasksCreate)      do{   _eCS_SysRestart(pUserTasksCreate);      \
                                                    return;     	                        \
												} while(0)


extern _eCS_UINT8 _eCS_QuitScheduling(eCS_TS *pTS, _eCS_UINT8 TaskState);
                                                
extern  _eCS_UINT8 _eCS_WaitusQuery(void);

extern  void eCS_Start(void);

extern void eCS_TaskSchedule(void);	

extern _eCS_UINT8 eCS_getTaskSum(void);                                                
extern _eCS_UINT8 eCS_getRunTaskSum(void);

extern _eCS_UINT32  eCS_millis(void);

extern _eCS_UINT32  eCS_millis_HotReset(void);

#define eCS_Sleep(pTS, Ticks)       do{_eCS_setTimeTrigger(pTS, Ticks);         \
                                       if(_eCS_this == (pTS))return;            \
                                      }while(0)

#define eCS_Ready(pTS)				do{if(_eCS_this != (pTS))                           \
                                       {if(((pTS->Priority == _eCS_TaskState_Suspended) || (pTS->Priority == _eCS_TaskState_Closed)))  \
                                        { eCS_Resume(pTS, 0);                           \
										}else                                           \
										{eCS_Sleep(pTS, 0);                             \
										}                                               \
									   }                                                \
                                      }while(0)

#define eCS_Restart(pTS, pInitTask, Ticks)  do{ (pTS)->pTask = pInitTask;              \
	                                            (pTS)->RunLine = (_eCS_RunLine_TYPE)0; \
	                                            _eCS_setTimeTrigger(pTS, Ticks);       \
										        if(_eCS_this == (pTS))return;          \
									          } while(0)

extern void eCS_SetPriority(eCS_TS *pTS, _eCS_UINT8 Priority);

#define eCS_Replace(pTS, pOtherTaskFunction, Ticks, Priority)   	                  \
                                   do{ (pTS)->pTask = pOtherTaskFunction;             \
	                                   (pTS)->RunLine = (_eCS_RunLine_TYPE)0;         \
	                                   _eCS_setTimeTrigger(pTS, Ticks);               \
                                       eCS_SetPriority(pTS, Priority);                \
                                       if(_eCS_this == (pTS))return;                  \
                                    } while(0)                                  

#define eCS_Suspend(pTS)      do{_eCS_QuitScheduling(pTS, _eCS_TaskState_Suspended); _eCS_runTaskSum --;}while(0)

extern _eCS_UINT8 eCS_Resume(eCS_TS *pTS, _eCS_DLY_TYPE Ticks);

#define eCS_Close(pTS)      do{_eCS_QuitScheduling(pTS, _eCS_TaskState_Closed); _eCS_runTaskSum --;_eCS_TaskSum --;}while(0)



#define eCS_SleepMe(Ticks)      do{ _eCS_setTimeTrigger(_eCS_this, Ticks);         \
                                    return;                                        \
                                   }while(0)
#define eCS_YieldAndSuspend()        do{ eCS_Suspend(_eCS_this);                  \
                                         eCS_Yield();                             \
                                       } while(0)
#define eCS_SuspendMeAtState(pNextStateFunction)                                  \
									 do{ eCS_Suspend(_eCS_this);                  \
                                         eCS_StateTransferTo(pNextStateFunction); \
                                       } while(0)

#define eCS_CloseMe()		         do{ eCS_Close(_eCS_this);                    \
                                         return;                                  \
                                       } while(0)

#define eCS_RestartMe(pInitTask, Ticks) eCS_Restart(_eCS_this, pInitTask, Ticks)
#define eCS_SetMePriority(pri)	        eCS_SetPriority(_eCS_this, pri)
#define eCS_ReplaceMe(pOtherTaskFunction, Ticks, Priority)	eCS_Replace(_eCS_this, pOtherTaskFunction, Ticks, Priority)	                                  



typedef struct _eCS_Massage
{
    volatile _eCS_UINT16  Count;
    
    _eCS_UINT16           pProducer;
    _eCS_UINT16           pConsumer;
	
    void	             *pMsg;
}eCS_SM;

#define eCS_SEM_NULL	            0

#define eCS_SemSignal(pSM)          (pSM)->Count++

#define eCS_SemReduce(pSM)          (pSM)->Count--
												
#define eCS_SemSet(pSM, v)          (pSM)->Count = (v)

#define eCS_SemClr(pSM)             (pSM)->Count = eCS_SEM_NULL	

#define eCS_Sem(pSM)                ((pSM)->Count)	

#define eCS_Msg(pSM, index, elementType)	 ((elementType *)((pSM)->pMsg))[index]

#define eCS_MsgPost(pSM, pDat, Len)  do{(pSM)->pMsg = pDat; eCS_SemSet(pSM, Len);     \
									   } while(0)

#define eCS_ProducerConsumer_init(pSM, pQueue)                                        \
                                     do{(pSM)->pMsg = pQueue; eCS_SemClr(pSM);        \
                                        (pSM)->pProducer = 0; (pSM)->pConsumer = 0;   \
									   } while(0)

#define eCS_Production(pSM, pushData, elementType, QueueLength)                       \
                                     do{if((pSM)->pProducer < ((QueueLength) - 1)) (pSM)->pProducer ++;  \
                                        else (pSM)->pProducer = 0;                    \
                                        eCS_Msg(pSM, (pSM)->pProducer, elementType) = pushData;  \
                                        eCS_SemSignal(pSM);                           \
									   } while(0)

#define eCS_Consumption(pSM, getData, elementType, QueueLength)                       \
                                     do{if((pSM)->pConsumer < ((QueueLength) - 1)) (pSM)->pConsumer ++;  \
                                        else (pSM)->pConsumer = 0;                    \
                                        *(getData) = eCS_Msg(pSM, (pSM)->pConsumer, elementType);  \
                                        eCS_SemReduce(pSM);                           \
									   } while(0)


#define	eCS_WaitusSet(tus)	    do{   _eCS_this->temp2.SysTickCounterOldVAL = _eCS_SysTick_Value; \
                                      _eCS_this->temp1.SysTickNeedCounted = (_eCS_SysTick_usAccType)(tus) * _eCS_SysTick_CyclesPerUs;\
                                  } while(0)

#define eCS_StateWaitUnless_WaitusOK()   do{ if(_eCS_WaitusQuery() == eCS_FAIL)return; } while(0)

#define	eCS_YieldWaitus(tus)   do{   eCS_WaitusSet(tus);                                     \
                                     eCS_Yield();	                                         \
                                     eCS_StateWaitUnless_WaitusOK();                         \
                                 } while(0)									

#define eCS_Waitus(tus)		   do{   eCS_WaitusSet(tus);                                     \
                                     while(_eCS_WaitusQuery() == eCS_FAIL);                  \
                                 } while(0)


#define   _eCS_PrioritizeTask_ENABLE   0

#if (_eCS_PrioritizeTask_ENABLE == 1)
extern eCS_TS	*_eCS_pPriorityTS_FIFO[4]; 
extern volatile eCS_UINT8 _eCS_pPriorityTaskPUSH, _eCS_pPriorityTaskExecute;

#define eCS_Prioritize(pTS)         do{ _eCS_pPriorityTaskPUSH ++;  _eCS_pPriorityTaskPUSH &= 0x03;  \
                                        _eCS_pPriorityTS_FIFO[_eCS_pPriorityTaskPUSH] = pTS;}while(0)

#define eCS_YieldForHighPriority()	do{	if(_eCS_pPriorityTaskExecute != _eCS_pPriorityTaskPUSH)      \
                                       {eCS_Yield();}}while(0)
#endif


#endif 	

//__ECS_H

