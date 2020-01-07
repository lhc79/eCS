#ifndef __ECS_CFG_H
#define __ECS_CFG_H


//定义数据类型,据不同CPU和C编译器要修改
typedef unsigned char 				_eCS_UINT8;
typedef signed char 				_eCS_INT8;
typedef unsigned short int  		_eCS_UINT16;
typedef signed   short int  		_eCS_INT16;
typedef unsigned long  				_eCS_UINT32;
typedef signed long  				_eCS_INT32;
typedef float  						_eCS_FLOAT32;


//定义CPU的类型
#define CPU_CortexM                 0 
#define CPU_AVR8                    1      //Timer1作为嘀嗒定时器
#define CPU_51                      100    //具体的51 CPU要定义为大于100的值
#define CPU_51_MCS51TIMER2          101    //与经典型51的Timer2兼容的51单片机，Timer2作为嘀嗒定时器:AT89/STC89/MPC82G516/C8051F020
#define CPU_51_STC12C5A60S2         102    //PCA作为嘀嗒定时器
#define CPU_51_IAP15W4K58S4         103    //Timer4作为嘀嗒定时器

//确定是哪个CPU类型
#define CPU_TYPE                    CPU_CortexM


//包含对应处理器的头文件。没有这行,清溢出中断标志的寄存器和嘀嗒定时器寄存器就没有定义
#include "msp.h"  
//#include "stm32f10x.h" 
//#include "stm32f4xx.h"
//#include "M051Series.h"
//#include "M451Series.h"     
//#include <avr/io.h>
//#include "reg52.h"
//#include "REG_MPC82G516.h"
//#include "C8051F020.h"  



//调度器节拍与硬件系统定时器相关定义
#define eCS_SYSTICK_MS 	            1                //定义调度系统时钟节拍时间(ms)

//节省内存，还是要效率。1为节省内存，采用单向链表，0为要效率，采用双向链表
#define _eCS_SaveRAM                0

//若是GUN或C99编译器，要优先将该宏设置为1，因为运行效率会更高
#define _eCS_CompilerIsGUN_C99      1

//使能和关闭总中断
#if CPU_TYPE == CPU_CortexM          
  #define _eCS_EnableIRQ()          __enable_irq()
  #define _eCS_DisableIRQ()         __disable_irq()
#endif
#if CPU_TYPE == CPU_AVR8          
  #define _eCS_EnableIRQ()          asm(  "sei")
  #define _eCS_DisableIRQ()         asm(  "cli")
#endif
#if CPU_TYPE > CPU_51               
  #define _eCS_EnableIRQ()			EA = 1
  #define _eCS_DisableIRQ()			EA = 0
#endif


//开机总中断默认是否已经使能（开启）：1-开启 0-关闭
#if CPU_TYPE == CPU_CortexM    
  #define _eCS_GLOBAL_INTERRUPT_DEFAULT_ENABLE  1
#endif
#if CPU_TYPE == CPU_AVR8    
  #define _eCS_GLOBAL_INTERRUPT_DEFAULT_ENABLE  0
#endif
#if CPU_TYPE > CPU_51  
  #define _eCS_GLOBAL_INTERRUPT_DEFAULT_ENABLE  0
#endif


//自动重载嘀嗒定时器中断函数规范化宏,不同的单片机要修改这个宏的内容,以自动适应不同单片机中断函数的写法
//该函数中不用处理总中断的使能问题，只要使能嘀嗒定时器中断即可。
#if CPU_TYPE == CPU_CortexM                      //Cortex-Mx的嘀嗒定时器作为系统嘀嗒
  #define _eCS_SysTick_OverflowIRQ()             SysTick_Handler(void) 
#endif
#if CPU_TYPE == CPU_AVR8                         //Timer1作为嘀嗒定时器
  #include <avr/interrupt.h>                     //GCCAVR
  #define _eCS_SysTick_OverflowIRQ()             ISR(TiMER1_OVF_vect) 
#endif
#if CPU_TYPE == CPU_51_MCS51TIMER2               //T2作为嘀嗒定时器
  #define _eCS_SysTick_OverflowIRQ()             Timer2_ISR(void) interrupt 5
#endif

//调度器滴答定时器初始化-执行ISR时是否自动自动重载-执行ISR时是否自动清除中断标志
#if CPU_TYPE == CPU_CortexM                      //Cortex-Mx的嘀嗒定时器作为系统嘀嗒
  #define _eCS_SysTickInit()                     SysTick_Config( SystemCoreClock/1000 *eCS_SYSTICK_MS )
  #define _eCS_SysTick_IsReload                  1       //执行ISR时是否自动重载
//  #define _eCS_SysTick_IsReload                0       //若为0则要给出重载语句
//  #define _eCS_SysTick_LoadInitialValue()      XXXXX;  //手动重载
  #define _eCS_SysTick_InterruptFlag_IsAutoClear 1       //执行ISR时自动清除OVF标志
#endif
#if CPU_TYPE == CPU_AVR8                         //Timer1作为嘀嗒定时器,16位的CTC
  void TIM1_Config(void);                        //声明
  #define _eCS_SysTickInit()                     TIM1_Config()  
  #define _eCS_SysTick_IsReload                  1       //CTC模式(WGM=4)，执行ISR时等效为自动重载
  #define _eCS_SysTick_InterruptFlag_IsAutoClear 1       //执行T1的ISR时自动清除标志
#endif 
#if CPU_TYPE == CPU_51_MCS51TIMER2               //T2作为嘀嗒定时器,16位自动重载
  void TIM2_Config(void);                        //声明
  #define _eCS_SysTickInit()                     TIM2_Config()
  #define _eCS_SysTick_IsReload                  1       //执行ISR时自动重载
  #define _eCS_SysTick_InterruptFlag_IsAutoClear 0  
  #define _eCS_SysTick_InterruptFlagClear()      TF2 = 0 //清溢出中断标志
#endif
 

//用于us延时
#if CPU_TYPE == CPU_CortexM            //Cortex-Mx的嘀嗒定时器作为系统嘀嗒
  typedef _eCS_UINT32                  _eCS_SysTick_usReadType; //滴答定时器的统计计数器位宽
  typedef _eCS_UINT32                  _eCS_SysTick_usAccType;  //滴答定时器的统计计算数值位宽
  #define _eCS_SysTick_Value		  (SysTick->VAL)
  #define _eCS_SysTick_ReloadValue	  (SystemCoreClock/1000 * eCS_SYSTICK_MS) 
  #define _eCS_SysTick_IsUpCounted 	   0	                    //1:加法计数器; 0:减法计数器
  #define _eCS_SysTick_CyclesPerUs     CyclesPerUs
#endif
#if CPU_TYPE == CPU_AVR8               //Timer1作为嘀嗒定时器
  typedef _eCS_UINT16                  _eCS_SysTick_usReadType; //滴答定时器的统计计数器位宽
  typedef _eCS_UINT16                  _eCS_SysTick_usAccType;  //滴答定时器的统计计算数值位宽
  #define _eCS_SysTick_Value	      (TCNT1)
  #define _eCS_SysTick_ReloadValue    (65535-(((SystemCoreClock)/8/1000)*eCS_SYSTICK_MS)+1) //16M时钟-8分频-CTC(WGM=4)
  #define _eCS_SysTick_IsUpCounted 	   1
  #define _eCS_SysTick_upReloadWay     0         //1:全1再加1溢出后重载初值型计数器,此时_eCS_SysTick_ReloadValue为重载初值
                                                 //0:比较匹配清零型加法计数器,此时_eCS_SysTick_ReloadValue为比较匹配值
  #define _eCS_SysTick_CyclesPerUs	  (2)        //16000000/8/1000000=2  
#endif
#if CPU_TYPE == CPU_51_MCS51TIMER2     //T2作为嘀嗒定时器
  typedef _eCS_UINT16                  _eCS_SysTick_usReadType; //滴答定时器的统计计数器位宽
  typedef _eCS_UINT16                  _eCS_SysTick_usAccType;  //滴答定时器的统计计算数值位宽
  /*
  _eCS_UINT16 Read_CPU51_CT2(void);
  _eCS_UINT16 Read_CPU51_CT2(void)
  {
	  union{
		  _eCS_UINT8  i[2];
		  _eCS_UINT16 j;
	  }u;
	  _eCS_UINT8 h8;
	  u.i[0] = TH2;
	  u.i[1] = TL2;
	  h8 = TH2;
	  if(h8 != u.i[0])
	  {
		  u.i[0] = h8;
	      u.i[1] = TL2;
	  }
	  return u.j;
  }
  */
  #define _eCS_SysTick_Value	      Read_CPU51_CT2()
  #define _eCS_SysTick_ReloadValue    (65535-(((SystemCoreClock)/12/1000)*eCS_SYSTICK_MS)+1) 
  #define _eCS_SysTick_IsUpCounted 	   1	     //1:加法计数器; 0:减法计数器
  #define _eCS_SysTick_upReloadWay     2         //1:全1再加1溢出后重载初值型计数器,此时_eCS_SysTick_ReloadValue为重载初值
                                                 //0:比较匹配清零型加法计数器,此时_eCS_SysTick_ReloadValue为比较匹配值
  #define _eCS_SysTick_CyclesPerUs	  (2)        //24000000/12/1000000=2  
#endif

																				
#endif      
//__ECS_CFG_H



