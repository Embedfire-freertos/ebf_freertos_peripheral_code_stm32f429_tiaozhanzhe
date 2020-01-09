/**
  *********************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   FreeRTOS V9.0.0  + STM32 固件库例程
  *********************************************************************
  * @attention
  *
  * 实验平台:野火 STM32 全系列开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  **********************************************************************
  */ 
 
/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/ 
/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
/* 开发板硬件bsp头文件 */
#include "bsp_led.h"
#include "bsp_debug_usart.h"
#include "bsp_key.h"
#include "./sdram/bsp_sdram.h"
#include "./lcd/bsp_lcd.h"
#include <string.h>
/**************************** 任务句柄 ********************************/
/* 
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */
static TaskHandle_t AppTaskCreate_Handle = NULL;/* 创建任务句柄 */
static TaskHandle_t Test_Task_Handle = NULL;/* LED任务句柄 */
static TaskHandle_t KEY_Task_Handle = NULL;/* KEY任务句柄 */

/********************************** 内核对象句柄 *********************************/
/*
 * 信号量，消息队列，事件标志组，软件定时器这些都属于内核的对象，要想使用这些内核
 * 对象，必须先创建，创建成功之后会返回一个相应的句柄。实际上就是一个指针，后续我
 * 们就可以通过这个句柄操作这些内核对象。
 *
 * 内核对象说白了就是一种全局的数据结构，通过这些数据结构我们可以实现任务间的通信，
 * 任务间的事件同步等各种功能。至于这些功能的实现我们是通过调用这些内核对象的函数
 * 来完成的
 * 
 */


/******************************* 全局变量声明 ************************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些全局变量。
 */


/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void AppTaskCreate(void);/* 用于创建任务 */

static void Test_Task(void* pvParameters);/* Test_Task任务实现 */
static void KEY_Task(void* pvParameters);/* KEY_Task任务实现 */

static void BSP_Init(void);/* 用于初始化板载相关资源 */

void LCD_Test(void);

/*****************************************************************
  * @brief  主函数
  * @param  无
  * @retval 无
  * @note   第一步：开发板硬件初始化 
            第二步：创建APP应用任务
            第三步：启动FreeRTOS，开始多任务调度
  ****************************************************************/
int main(void)
{	
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
  
  /* 开发板硬件初始化 */
  BSP_Init();
  
  printf("这是一个[野火]-STM32全系列开发板-FreeRTOS固件库例程！\n\n");
  printf("按下KEY1挂起任务，按下KEY2恢复任务\n");
  
   /* 创建AppTaskCreate任务 */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
  /* 启动任务调度 */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* 启动任务，开启调度 */
  else
    return -1;  
  
  while(1);   /* 正常不会执行到这里 */    
}


/***********************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
static void AppTaskCreate(void)
{
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
  
  taskENTER_CRITICAL();           //进入临界区
  
  /* 创建Test_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )Test_Task, /* 任务入口函数 */
                        (const char*    )"Test_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )2,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&Test_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    printf("创建Test_Task任务成功!\r\n");
  /* 创建KEY_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )KEY_Task,  /* 任务入口函数 */
                        (const char*    )"KEY_Task",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )3, /* 任务的优先级 */
                        (TaskHandle_t*  )&KEY_Task_Handle);/* 任务控制块指针 */ 
  if(pdPASS == xReturn)
    printf("创建KEY_Task任务成功!\r\n");
  
  vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
  
  taskEXIT_CRITICAL();            //退出临界区
}



/**********************************************************************
  * @ 函数名  ： Test_Task
  * @ 功能说明： Test_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void Test_Task(void* parameter)
{	
  while (1)
  {
		LCD_Test();
    vTaskDelay(1000);   /* 延时500个tick */
  }
}

/**********************************************************************
  * @ 函数名  ： Test_Task
  * @ 功能说明： Test_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void KEY_Task(void* parameter)
{	
  while (1)
  {
    if( Key_Scan(KEY1_GPIO_PORT,KEY1_PIN) == KEY_ON )
    {/* K1 被按下 */
      printf("挂起Test_Task任务！\n");
      vTaskSuspend(Test_Task_Handle);/* 挂起LED任务 */
      printf("挂起Test_Task任务成功！\n");
    } 
    if( Key_Scan(KEY2_GPIO_PORT,KEY2_PIN) == KEY_ON )
    {/* K2 被按下 */
      printf("恢复Test_Task任务！\n");
      vTaskResume(Test_Task_Handle);/* 恢复LED任务！ */
      printf("恢复Test_Task任务成功！\n");
    }
    vTaskDelay(20);/* 延时20个tick */
  }
}

/***********************************************************************
  * @ 函数名  ： BSP_Init
  * @ 功能说明： 板级外设初始化，所有板子上的初始化均可放在这个函数里面
  * @ 参数    ：   
  * @ 返回值  ： 无
  *********************************************************************/
static void BSP_Init(void)
{
	/*
	 * STM32中断优先级分组为4，即4bit都用来表示抢占优先级，范围为：0~15
	 * 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断，
	 * 都统一用这个优先级分组，千万不要再分组，切忌。
	 */
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
	/* LED 初始化 */
	LED_GPIO_Config();

	/* 串口初始化	*/
	Debug_USART_Config();
  
  /* 按键初始化	*/
  Key_GPIO_Config();
	
  /*初始化液晶屏*/
  LCD_Init();
  LCD_LayerInit();
  LTDC_Cmd(ENABLE);
	
	/*把背景层刷黑色*/
  LCD_SetLayer(LCD_BACKGROUND_LAYER);  
	LCD_Clear(LCD_COLOR_BLACK);
	
  /*初始化后默认使用前景层*/
	LCD_SetLayer(LCD_FOREGROUND_LAYER); 
	/*默认设置不透明	，该函数参数为不透明度，范围 0-0xff ，0为全透明，0xff为不透明*/
  LCD_SetTransparency(0xFF);
	LCD_Clear(LCD_COLOR_BLACK);
	/*经过LCD_SetLayer(LCD_FOREGROUND_LAYER)函数后，
	以下液晶操作都在前景层刷新，除非重新调用过LCD_SetLayer函数设置背景层*/		
	
  LED_BLUE;    
}


/*用于测试各种液晶的函数*/
void LCD_Test(void)
{
	static uint8_t testCNT=0;
	char dispBuff[100];
	
	testCNT++;
	
	/*使用不透明前景层*/
	LCD_SetLayer(LCD_FOREGROUND_LAYER);  
  LCD_SetTransparency(0xff);
	
  LCD_Clear(LCD_COLOR_BLACK);	/* 清屏，显示全黑 */

	/*设置字体颜色及字体的背景颜色(此处的背景不是指LCD的背景层！注意区分)*/
  LCD_SetColors(LCD_COLOR_WHITE,LCD_COLOR_BLACK);
	
	/*选择字体，使用中英文显示时，尽量把英文选择成16*24的字体，
	*中文字体大小是24*24的，需要其它字体请自行制作字模*/
	/*这个函数只对英文字体起作用*/
  LCD_SetFont(&Font16x24); 	

 	LCD_DisplayStringLine_EN_CH(LINE(1),(uint8_t* )"秉火5.0英寸液晶屏参数，");
	LCD_DisplayStringLine_EN_CH(LINE(2),(uint8_t* )"分辨率:800x480 像素");
  LCD_DisplayStringLine_EN_CH(LINE(3),(uint8_t* )"触摸屏:5点电容触摸屏");
  LCD_DisplayStringLine_EN_CH(LINE(4),(uint8_t* )"使用STM32-LTDC直接驱动,无需外部液晶驱动器");
	LCD_DisplayStringLine_EN_CH(LINE(5),(uint8_t* )"支持RGB888/565,24位数据总线");
  LCD_DisplayStringLine_EN_CH(LINE(6),(uint8_t* )"触摸屏使用IIC总线驱动");
	
	/*使用c标准库把变量转化成字符串*/
	sprintf(dispBuff,"显示变量例子: testCount = %d ",testCNT);
	LCD_ClearLine(LINE(7));
	
	/*然后显示该字符串即可，其它变量也是这样处理*/
	LCD_DisplayStringLine_EN_CH(LINE(7),(uint8_t* )dispBuff);


  /* 画直线 */
  LCD_SetTextColor(LCD_COLOR_BLUE);
  
	LCD_ClearLine(LINE(8));
  LCD_DisplayStringLine_EN_CH(LINE(8),(uint8_t* )"画线:");
  
	LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_BLACK);
  LCD_DrawUniLine(50,250,750,250);  
  LCD_DrawUniLine(50,300,750,300);
  
	LCD_SetColors(LCD_COLOR_GREEN,LCD_COLOR_BLACK);
  LCD_DrawUniLine(300,250,400,400);  
  LCD_DrawUniLine(600,250,600,400);
  
  vTaskDelay(1000);
  
	LCD_SetColors(LCD_COLOR_BLACK,LCD_COLOR_BLACK);
  LCD_DrawFullRect(0,200,LCD_PIXEL_WIDTH,LCD_PIXEL_HEIGHT-200);
  
  
  /*画矩形*/
  LCD_SetTextColor(LCD_COLOR_BLUE);
	LCD_ClearLine(LINE(8));
  LCD_DisplayStringLine_EN_CH(LINE(8),(uint8_t* )"画矩形:");
	
	LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_BLACK);
  LCD_DrawRect(200,250,200,100);
	
	LCD_SetColors(LCD_COLOR_GREEN,LCD_COLOR_BLACK);
  LCD_DrawRect(350,250,200,50);
	
	LCD_SetColors(LCD_COLOR_BLUE,LCD_COLOR_BLACK);
  LCD_DrawRect(200,350,50,200);
  
  vTaskDelay(1000);
  
  
	LCD_SetColors(LCD_COLOR_BLACK,LCD_COLOR_BLACK);
  LCD_DrawFullRect(0,200,LCD_PIXEL_WIDTH,LCD_PIXEL_HEIGHT-200);
  

  /*填充矩形*/
  LCD_SetTextColor(LCD_COLOR_BLUE);
	LCD_ClearLine(LINE(8));
	LCD_DisplayStringLine_EN_CH(LINE(8),(uint8_t* )"填充矩形:");

 	LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_BLACK);
  LCD_DrawFullRect(200,250,200,100);
	
	LCD_SetColors(LCD_COLOR_GREEN,LCD_COLOR_BLACK);
  LCD_DrawFullRect(350,250,200,50);
	
	LCD_SetColors(LCD_COLOR_BLUE,LCD_COLOR_BLACK);
  LCD_DrawFullRect(200,350,50,200);
  
  vTaskDelay(1000);
  
	LCD_SetColors(LCD_COLOR_BLACK,LCD_COLOR_BLACK);
  LCD_DrawFullRect(0,200,LCD_PIXEL_WIDTH,LCD_PIXEL_HEIGHT-200);
	
  /* 画圆 */
	LCD_SetTextColor(LCD_COLOR_BLUE);
	LCD_ClearLine(LINE(8));
	LCD_DisplayStringLine_EN_CH(LINE(8),(uint8_t* )"画圆:");
  
	LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_RED);
  LCD_DrawCircle(200,350,50);
	
	LCD_SetColors(LCD_COLOR_GREEN,LCD_COLOR_GREEN);
  LCD_DrawCircle(350,350,75);
  
  vTaskDelay(1000);
  
	LCD_SetColors(LCD_COLOR_BLACK,LCD_COLOR_BLACK);
  LCD_DrawFullRect(0,200,LCD_PIXEL_WIDTH,LCD_PIXEL_HEIGHT-200);


  /*填充圆*/
	LCD_SetTextColor(LCD_COLOR_BLUE);
	LCD_ClearLine(LINE(8));
	LCD_DisplayStringLine_EN_CH(LINE(8),(uint8_t* )"填充圆:");
  
	LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_BLACK);
  LCD_DrawFullCircle(300,350,50);
	
	LCD_SetColors(LCD_COLOR_GREEN,LCD_COLOR_BLACK);
  LCD_DrawFullCircle(450,350,75);
  
  vTaskDelay(1000);
  
	LCD_SetColors(LCD_COLOR_BLACK,LCD_COLOR_BLACK);
  LCD_DrawFullRect(0,200,LCD_PIXEL_WIDTH,LCD_PIXEL_HEIGHT-200);
		
	LCD_ClearLine(LINE(8));

//这种操作效果会有花屏现象，不推荐使用。一般背景混合使用ARGB格式的颜色来混合	
//	/*透明效果 前景层操作*/
//	{
//		LCD_SetTextColor(LCD_COLOR_BLUE);
//		LCD_ClearLine(LINE(8));
//		LCD_DisplayStringLine_EN_CH(LINE(8),(uint8_t* )"双层透明效果：");
//		
//		/*设置前景层不透明度*/
//		LCD_SetLayer(LCD_FOREGROUND_LAYER); 	
//		LCD_SetTransparency(200);
//		
//		/*在前景画一个红色圆*/
//		LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_BLACK);
//		LCD_DrawFullCircle(400,350,75);
//	
//	}
//	
//	/*透明效果 背景层操作*/
//	{		
//		/*设置背景层不透明*/
//		LCD_SetLayer(LCD_BACKGROUND_LAYER); 	
//		LCD_SetTransparency(0xff);
//		LCD_Clear(LCD_COLOR_BLACK);

//		/*在背景画一个绿色圆*/
//		LCD_SetColors(LCD_COLOR_GREEN,LCD_COLOR_BLACK);
//		LCD_DrawFullCircle(450,350,75);
//		
//		/*在背景画一个蓝色圆*/
//		LCD_SetColors(LCD_COLOR_BLUE,LCD_COLOR_BLACK);
//		LCD_DrawFullCircle(350,350,75);
//	}
  
  vTaskDelay(1000);
	
	LCD_SetColors(LCD_COLOR_BLACK,LCD_COLOR_BLACK);
  LCD_DrawFullRect(0,200,LCD_PIXEL_WIDTH,LCD_PIXEL_HEIGHT-200);


}




/*"当"字符的字模*/ 	  
unsigned char charater_matrix[] =    
{ 
0x00,0x80,0x10,0x90,0x08,0x98,0x0C,0x90,0x08,0xA0,0x00,0x80,0x3F,0xFC,0x00,0x04,   
0x00,0x04,0x1F,0xFC,0x00,0x04,0x00,0x04,0x00,0x04,0x3F,0xFC,0x00,0x04,0x00,0x00  
};   
  
/**
  * @brief  使用串口在上位机打印字模
	*					演示字模显示原理
  * @retval 无
  */
void Printf_Charater(void)   
{   
	int i,j;   
	unsigned char kk; 
  
	/*i用作行计数*/
  for ( i=0;i<16;i++)   
	{   
		/*j用作一字节内数据的移位计数*/
		/*一行像素的第一个字节*/
    for(j=0; j<8; j++)   
    {      
			/*一个数据位一个数据位地处理*/
      kk = charater_matrix[2*i] << j ;  //左移J位          
			if( kk & 0x80)     
			{   
				printf("*"); //如果最高位为1，输出*号，表示笔迹
			}   
			else  
			{   
				printf(" "); //如果最高位为0，输出空格，表示空白  
			}   
    }   
		/*一行像素的第二个字节*/
    for(j=0; j<8; j++)   
		{              
      kk = charater_matrix[2*i+1] << j ;  //左移J位   
       
			if( kk & 0x80)         
			{   
				printf("*"); //如果最高位为1，输出*号，表示笔迹   
			}   
			else  
			{   
				printf(" "); //如果最高位为0，输出空格，表示空白 
			}   
		}         
    printf("\n");    //输出完一行像素，换行 
	}   
	printf("\n\n"); 		//一个字输出完毕
}   



/********************************END OF FILE****************************/
