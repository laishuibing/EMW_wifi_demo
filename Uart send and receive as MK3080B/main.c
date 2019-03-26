/**
 ******************************************************************************
 * @file    main.c
 * @author  
 * @version V1.0.0
 * @date    
 * @brief   
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2016 MXCHIP Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ******************************************************************************
 */

/*
    the demo is about how to use UART send and receive,
    if apply to EMW3080B wifi module.
*/
#include "mico.h"

//#define os_helloworld_log(format, ...)  custom_log("helloworld", format, ##__VA_ARGS__)

mico_thread_t aa = NULL;

mico_queue_t queueVer = NULL;

typedef struct                                                      //struct define 
{
   uint8_t cmd;
}notify;


/********************User thread callback function ***************/
void test_thread(mico_thread_arg_t arg)
{
  notify received;
 // MicoSystemReboot();
  while(1)
  {
    if(!mico_rtos_is_queue_empty(&queueVer))                        //if the queque is not empty
    {
        mico_rtos_pop_from_queue(&queueVer,&received,MICO_WAIT_FOREVER);
        if(received.cmd==9)                                         //read the data from queque
            platform_log("hello world"); 
    }

    mico_rtos_delay_milliseconds(100);                              //thread delay 100ms to other thread running 
  }
 
}


void GPIO_IRQ_Call( void* arg )                                     //GPIO interrput callback function
{

    MicoGpioOutputTrigger(MICO_GPIO_12);
}

int application_start( void )
{
  	// Add your code here
    
    notify ddd = {0};
  /* Output on debug serial port */ 
    platform_log( "lai qing guan!%s",MicoGetVer());                 //Debug Uart print message                                             
    mico_rtos_create_thread(&aa,MICO_APPLICATION_PRIORITY,"test_thread",test_thread,0x1000,0);      //create a user thread 
    mico_rtos_init_queue(&queueVer,"queue",sizeof(notify),3);       //initialization a queque
    MicoGpioInitialize(MICO_GPIO_12,OUTPUT_PUSH_PULL);              //initialization GPIO as output 
    MicoGpioOutputHigh(MICO_GPIO_12);                               //The GPIO station set high

    MicoGpioInitialize(MICO_GPIO_19,INPUT_PULL_UP);                 //Set the GPIO as input 
    MicoGpioEnableIRQ(MICO_GPIO_19,IRQ_TRIGGER_FALLING_EDGE,GPIO_IRQ_Call,(void*) NULL);            //initialization GPIO interrupt

  /* Trigger MiCO system led available on most MiCOKit */
    mico_uart_config_t uart_config;
    ring_buffer_t  rx_buffer;
    uint8_t c =  0;
//    uint8_t      rx_data[100];                                    //Static memory

    uint8_t *      rx_data;
    rx_data = malloc (1024);                                        //dynamic Memory distribution

    uart_config.baud_rate    = 115200;
    uart_config.data_width   = DATA_WIDTH_8BIT;
    uart_config.parity       = NO_PARITY;
    uart_config.stop_bits    = STOP_BITS_1;
    uart_config.flow_control = FLOW_CONTROL_DISABLED;
    uart_config.flags        = UART_WAKEUP_DISABLE;
  
    ring_buffer_init ((ring_buffer_t *)&rx_buffer, (uint8_t *)rx_data, 1024);   //initialization  Receive buffer 
    MicoUartInitialize (MICO_UART_1, &uart_config, (ring_buffer_t *)&rx_buffer); //initialization user Customer serial port


  while(1)
  {
    ddd.cmd = 9;
    platform_log("application Start");
   
    //mico_thread_sleep(2);
    mico_rtos_delay_milliseconds(500);                          //delay 500ms
    mico_rtos_push_to_queue(&queueVer,&ddd,0);                  //message push queque 

    if (kNoErr != MicoUartRecv( MICO_UART_1, &c, 1, 20))       //check UART if receive data 
         continue;
    if (c == 5)
    {
        MicoUartSend( MICO_UART_1, &c, 1 );                     //send data use by UART 
        MicoGpioOutputTrigger(MICO_GPIO_12);                    //and trigger the GPIO LED
        c = 0;                                                  //clean the variable
    }
  }
	return 0;
}


