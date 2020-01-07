/*********************************Copyright(c)**********************************
** File Name:			eCS.c
** Author:				刘海成
** Copyright:			刘海成        
** author email:		liuhaicheng@126.com
** version:             V2.78
** Latest modified Date:2019-1-22

** Description:			调度器相关功能实现函数
******************************************************************************/

#include "eCS.h"

volatile _eCS_DLY_TYPE eCS_Ticks = 0;

#if   _eCS_Timetrigger_Type == 27
volatile static _eCS_UINT8 _eCS_TickSign = 0;
#endif

_eCS_DLY_TYPE eCS_Ticks_SincePOWERON = 0;

eCS_TS	*_eCS_this;

_eCS_UINT8  _eCS_TaskSum = 0; 

_eCS_UINT8  _eCS_runTaskSum = 0;


#if (_eCS_PrioritizeTask_ENABLE == 1)
eCS_TS	*_eCS_pPriorityTS_FIFO[4];
volatile eCS_UINT8 _eCS_pPriorityTaskPUSH = 0, _eCS_pPriorityTaskExecute = 0;
#endif

_eCS_UINT8 eCS_TaskCreate(	eCS_TS         *pNewTS,
							              void          (*pNewTask)(void),
						              	_eCS_DLY_TYPE	Ticks,
                            _eCS_UINT8      Priority)
{
	if (_eCS_TaskSum < 255) 
	{
		pNewTS->pTask = pNewTask;
        
		_eCS_setTimeTrigger(pNewTS, Ticks);
        
		pNewTS->RunLine = (_eCS_RunLine_TYPE)0;
		
    eCS_SetPriority(pNewTS, Priority); 
    pNewTS->PriorityCounter = 0;       
		
		if (_eCS_TaskSum == 0)           
		{
			_eCS_this = pNewTS;     
			pNewTS->pNextTS = pNewTS;	
#if _eCS_SaveRAM == 0         
            pNewTS->pPreviousTS = pNewTS;
#endif
		}
		else		
		{			
#if _eCS_SaveRAM == 0   
         _eCS_this->pNextTS->pPreviousTS = pNewTS;
         pNewTS->pPreviousTS = _eCS_this;
#endif
         pNewTS->pNextTS = _eCS_this->pNextTS;
         _eCS_this->pNextTS = pNewTS;
		}
		
		_eCS_TaskSum ++;
        _eCS_runTaskSum ++;
		
		return eCS_SUCCESS;	
	}
	return eCS_FAIL;
}
//-------------------------------------------------------------------------------------------
_eCS_UINT8 eCS_getTaskSum(void)
{
    return _eCS_TaskSum;
}
_eCS_UINT8 eCS_getRunTaskSum(void)
{
    return _eCS_runTaskSum;
}
//-------------------------------------------------------------------------------------------
void eCS_SetPriority(eCS_TS *pTS, _eCS_UINT8 Priority) 
{
    if(Priority > 3)Priority = 3; 
    if(Priority == 0)
    {
        pTS->Priority = 1;
    }
    else
    {
        pTS->Priority = 1 << Priority;
    } 
}

void _eCS_SysRestart( void(*pUserTasks_Create)(void) )
{
	_eCS_DisableIRQ();	
    
    eCS_Ticks_SincePOWERON += eCS_Ticks;
       
    eCS_Ticks = 0;
#if   _eCS_Timetrigger_Type == 27 
	_eCS_TickSign = 0;
#endif 
       
    _eCS_TaskSum = 0;
    _eCS_runTaskSum = 0;

#if (_eCS_PrioritizeTask_ENABLE == 1)
    _eCS_pPriorityTaskExecute = 0;
    _eCS_pPriorityTaskPUSH = 0;
#endif
	
	pUserTasks_Create();

	_eCS_SysTickInit();	
	
	_eCS_EnableIRQ();
}

_eCS_UINT32  eCS_millis(void)
{
    return (eCS_Ticks_SincePOWERON + eCS_Ticks) * eCS_SYSTICK_MS;
}

_eCS_UINT32  eCS_millis_HotReset(void)
{
    return eCS_Ticks * eCS_SYSTICK_MS;
} 

_eCS_UINT8 _eCS_QuitScheduling(eCS_TS *pTS, _eCS_UINT8 TaskState)
{
#if _eCS_SaveRAM != 0        
    eCS_TS *TS_temp = pTS;
    
    if((TS_temp->Priority != _eCS_TaskState_Suspended) && (TS_temp->Priority != _eCS_TaskState_Closed))
    {
        TS_temp->PriorityCounter = TS_temp->Priority; 
        TS_temp->Priority = TaskState;

        TS_temp = pTS->pNextTS;
        while(TS_temp->pNextTS != pTS)
        {
            TS_temp = TS_temp->pNextTS;
        }
        TS_temp->pNextTS = pTS->pNextTS;
        
        return eCS_SUCCESS;
    }
#else       
    if((pTS->Priority != _eCS_TaskState_Suspended) && (pTS->Priority != _eCS_TaskState_Closed))
    {
        pTS->PriorityCounter = pTS->Priority;    
        pTS->Priority = TaskState;  

        pTS->pPreviousTS->pNextTS = pTS->pNextTS;
        pTS->pNextTS->pPreviousTS = pTS->pPreviousTS;
       
        return eCS_SUCCESS;
    }
#endif   
    else
    {
        return eCS_FAIL;
    }
}

_eCS_UINT8 eCS_Resume(eCS_TS *pTS, _eCS_DLY_TYPE Ticks)
{
	if((_eCS_this == pTS) || ((pTS->Priority != _eCS_TaskState_Suspended) && (pTS->Priority != _eCS_TaskState_Closed)))
    {
        return eCS_FAIL;
    }
	
    pTS->Priority = pTS->PriorityCounter;  
    pTS->PriorityCounter = 0;
    
#if _eCS_SaveRAM == 0 
    _eCS_this->pNextTS->pPreviousTS = pTS;
    pTS->pPreviousTS = _eCS_this;
    pTS->pNextTS = _eCS_this->pNextTS;
    _eCS_this->pNextTS = pTS;
#else
    _eCS_this->pNextTS = pTS;
    pTS->pNextTS = _eCS_this->pNextTS;
#endif
    
	_eCS_runTaskSum ++;
    
	_eCS_setTimeTrigger(pTS, Ticks);
		
	return eCS_SUCCESS;
}

_eCS_UINT8 _eCS_WaitusQuery(void)
{		
    _eCS_SysTick_usAccType delt;
    _eCS_SysTick_usReadType _eCS_SysTickCounterNowVAL = _eCS_SysTick_Value; 

#if _eCS_SysTick_IsUpCounted == 1  
	if(_eCS_SysTickCounterNowVAL > _eCS_this->temp2.SysTickCounterOldVAL)
	{
		delt = _eCS_SysTickCounterNowVAL - _eCS_this->temp2.SysTickCounterOldVAL;
	}
	else 
	{
    #if _eCS_SysTick_upReloadWay == 1    
		delt =  _eCS_SysTickCounterNowVAL - _eCS_this->temp2.SysTickCounterOldVAL - _eCS_SysTick_ReloadValue;
    #else                               
        #if _eCS_SysTick_upReloadWay == 0 
            delt = _eCS_SysTick_ReloadValue - _eCS_this->temp2.SysTickCounterOldVAL + SysTickCounterNowVAL;
        #endif
    #endif
	}
#else   
	if(_eCS_SysTickCounterNowVAL < _eCS_this->temp2.SysTickCounterOldVAL)
	{
		delt = _eCS_this->temp2.SysTickCounterOldVAL - _eCS_SysTickCounterNowVAL;
	}
	else 
	{
		delt = _eCS_SysTick_ReloadValue - _eCS_SysTickCounterNowVAL + _eCS_this->temp2.SysTickCounterOldVAL;
	}
#endif
	
	_eCS_this->temp2.SysTickCounterOldVAL = _eCS_SysTickCounterNowVAL;
    
    if(delt < _eCS_this->temp1.SysTickNeedCounted)
    {
        _eCS_this->temp1.SysTickNeedCounted -= delt;
        return eCS_FAIL;
    }
    else
    {	
        return eCS_SUCCESS;
    }
}

void _eCS_SysTick_OverflowIRQ()
{
#if _eCS_SysTick_IsReload == 0
	_eCS_SysTick_LoadInitialValue();
#endif  

	eCS_Ticks ++;

#if   _eCS_Timetrigger_Type == 27	
	_eCS_TickSign ++;  
#endif

#if _eCS_SysTick_InterruptFlag_IsAutoClear == 0
	_eCS_SysTick_InterruptFlagClear(); 
#endif
}

void eCS_Start(void)
{		
	_eCS_SysTickInit();	
	
#if _eCS_GLOBAL_INTERRUPT_DEFAULT_ENABLE == 0
	_eCS_EnableIRQ();
#endif
}

void eCS_TaskSchedule(void)
{
#if _eCS_Timetrigger_Type == 27
	eCS_TS     *pTS;
	_eCS_UINT8  i;
#endif    

#if (_eCS_PrioritizeTask_ENABLE == 1)
	if(_eCS_pPriorityTaskExecute != _eCS_pPriorityTaskPUSH)
	{
		do{					
			_eCS_pPriorityTaskExecute ++;
			_eCS_pPriorityTaskExecute &= 0x03;
				
			_eCS_this = _eCS_pPriorityTS_FIFO[_eCS_pPriorityTaskExecute];
			_eCS_this->pTask();	
				
		}while(_eCS_pPriorityTaskExecute != _eCS_pPriorityTaskPUSH);
		
        return ;
	}	
#endif
	
	
#if _eCS_Timetrigger_Type == 27	    
    if(_eCS_TickSign)     
    {
        pTS = _eCS_this;
        for(i = 0; i < _eCS_runTaskSum; i++)
        {
            if (pTS->Timetrigger_Tick) 
            {
                pTS->Timetrigger_Tick --;
            }
            pTS = pTS->pNextTS;
        }
        
        _eCS_TickSign --;
    }
#endif
    
	_eCS_this = _eCS_this->pNextTS;   
    if(_eCS_TimeTriggerFail())  
    {
        return ;
    }
    
    if(++_eCS_this->PriorityCounter >= _eCS_this->Priority)
    {
        _eCS_this->pTask();  
        _eCS_this->PriorityCounter = 0;
    }      
    
}
