/*
 * STM32 fa103c8 (Blue Pill) Development Board
 *
 * Copyright (C) 2018 Basel Alsayeh
 * Copyright (C) 2020 Luis CLaudio G Lopes
 *
 * Implementation based on
 * Olimex "STM-P103 Development Board Users Manual Rev. A, April 2008"
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "hw/arm/stm32.h"
#include "hw/sysbus.h"
#include "hw/arm/armv7m.h"
// #include "hw/devices.h"
#include "ui/console.h"
#include "sysemu/sysemu.h"
#include "hw/boards.h"
#include "hw/irq.h"
#include "hw/arm/boot.h"
#include "hw/i2c/i2c.h"
#include "hw/ssi/ssi.h"

#include "../system/simuliface.h"

//#define STM32_GPIOS_DIR "stm32_gpios_dir"
//#define STM32_GPIOS_SYNC "stm32_gpios_sync"
//#define STM32_TIM_SYNC "stm32_tim_sync"

typedef struct
{
   Stm32 *stm32;
   qemu_irq pin_irq[100];
   qemu_irq *pout_irq;
   qemu_irq *pdir_irq;
   qemu_irq *psync_irq;
   DeviceState *gpio_a;
   DeviceState *gpio_b;
   DeviceState *gpio_c;
   DeviceState *gpio_d;
   DeviceState *uart1;
   DeviceState *uart2;
   DeviceState *uart3;
   DeviceState *i2c1;
   DeviceState *i2c2;
   DeviceState *spi1;
   DeviceState *spi2;
   DeviceState *afio;
   DeviceState *tim1;
   DeviceState *tim2;
   DeviceState *tim3;
   DeviceState *tim4;
   DeviceState *tim5; 
   DeviceState *adc1;
   DeviceState *adc2;
} Stm32_F103c8_Mcu;

//#define QEMU_INTERNAL_UART0_BAUD 7
//#define QEMU_INTERNAL_UART1_BAUD 8
//#define QEMU_INTERNAL_UART2_BAUD 9

uint32_t stm32_uart_baud_rate(void *opaque);

/// TODO: flash and ram sizes configurable ???
#define FLASH_SIZE 0x00020000
#define RAM_SIZE 0x00005000
/* Main SYSCLK frequency in Hz (24MHz) */
#define SYSCLK_FRQ 24000000ULL

static void stm32_f103c8_init( MachineState *machine )
{
   Clock *sysclk;

   //uint32_t freq = getFrequency();

   Stm32_F103c8_Mcu* s = (Stm32_F103c8_Mcu*)g_malloc0( sizeof(Stm32_F103c8_Mcu) );

   sysclk = clock_new( OBJECT(machine), "SYSCLK");
   clock_set_hz( sysclk, SYSCLK_FRQ);
   stm32_init( FLASH_SIZE, RAM_SIZE, machine->kernel_filename, 8000000, 32768, sysclk );

   s->gpio_a = DEVICE( object_resolve_path("/machine/stm32/gpio[a]" , NULL) );
   s->gpio_b = DEVICE( object_resolve_path("/machine/stm32/gpio[b]" , NULL) );
   s->gpio_c = DEVICE( object_resolve_path("/machine/stm32/gpio[c]" , NULL) );
   s->gpio_d = DEVICE( object_resolve_path("/machine/stm32/gpio[d]" , NULL) );
   s->uart1  = DEVICE( object_resolve_path("/machine/stm32/uart[1]" , NULL) );
   s->uart2  = DEVICE( object_resolve_path("/machine/stm32/uart[2]" , NULL) );
   s->uart3  = DEVICE( object_resolve_path("/machine/stm32/uart[3]" , NULL) );
   s->i2c1   = DEVICE( object_resolve_path("/machine/stm32/i2c[1]"  , NULL) );
   s->i2c2   = DEVICE( object_resolve_path("/machine/stm32/i2c[2]"  , NULL) );
   s->spi1   = DEVICE( object_resolve_path("/machine/stm32/spi[1]"  , NULL) );
   s->spi2   = DEVICE( object_resolve_path("/machine/stm32/spi[2]"  , NULL) );
   s->afio   = DEVICE( object_resolve_path("/machine/stm32/afio"    , NULL) );
   s->tim1   = DEVICE( object_resolve_path("/machine/stm32/timer[1]", NULL) );
   s->tim2   = DEVICE( object_resolve_path("/machine/stm32/timer[2]", NULL) );
   s->tim3   = DEVICE( object_resolve_path("/machine/stm32/timer[3]", NULL) );
   s->tim4   = DEVICE( object_resolve_path("/machine/stm32/timer[4]", NULL) );
   s->tim5   = DEVICE( object_resolve_path("/machine/stm32/timer[5]", NULL) );
   s->adc1   = DEVICE( object_resolve_path("/machine/stm32/adc[1]"  , NULL) );
   s->adc2   = DEVICE( object_resolve_path("/machine/stm32/adc[2]"  , NULL) );

   assert( s->gpio_a );
   assert( s->gpio_b );
   assert( s->gpio_c );
   assert( s->gpio_d );
   assert( s->uart2 );
   assert( s->uart1 );
   assert( s->uart3 );
   assert( s->i2c1 );
   assert( s->i2c2 );
   assert( s->spi1 );
   assert( s->spi2 );
   assert( s->afio );
   assert( s->tim1 );
   assert( s->tim2 );
   assert( s->tim3 );
   assert( s->tim4 );
   assert( s->tim5 );
   assert( s->adc1 );
   assert( s->adc2 );

   //s->psync_irq = qemu_allocate_irqs(psync_irq_handler, NULL, 5);
   //qdev_connect_gpio_out_named(s->gpio_a, STM32_GPIOS_SYNC, 0, s->psync_irq[0]);
   //qdev_connect_gpio_out_named(s->gpio_b, STM32_GPIOS_SYNC, 0, s->psync_irq[1]);
   //qdev_connect_gpio_out_named(s->gpio_c, STM32_GPIOS_SYNC, 0, s->psync_irq[2]);
   //qdev_connect_gpio_out_named(s->gpio_d, STM32_GPIOS_SYNC, 0, s->psync_irq[3]);

   //qdev_connect_gpio_out_named(s->tim1, STM32_TIM_SYNC, 0, s->psync_irq[0]);
   //qdev_connect_gpio_out_named(s->tim2, STM32_TIM_SYNC, 0, s->psync_irq[1]);
   //qdev_connect_gpio_out_named(s->tim3, STM32_TIM_SYNC, 0, s->psync_irq[2]);
   //qdev_connect_gpio_out_named(s->tim4, STM32_TIM_SYNC, 0, s->psync_irq[3]);
   //qdev_connect_gpio_out_named(s->tim5, STM32_TIM_SYNC, 0, s->psync_irq[4]);

   //if (pinmap)
   //{
   //   s->pdir_irq = qemu_allocate_irqs(pdir_irq_handler, NULL, pinmap[0] + 1);
   //   s->pout_irq = qemu_allocate_irqs(pout_irq_handler, NULL, pinmap[0] + 1);
   //   for (int pin = 1; pin < (pinmap[0] + 1); pin++)
   //   {
   //      if (pinmap[pin] >= 0)
   //      {
   //         DeviceState *gport = NULL;

   //         switch ((pinmap[pin] & 0xF000) >> 12)
   //         {
   //         case 1:
   //            gport = s->gpio_a;
   //            break;
   //         case 2:
   //            gport = s->gpio_b;
   //            break;
   //         case 3:
   //            gport = s->gpio_c;
   //            break;
   //         case 4:
   //            gport = s->gpio_d;
   //            break;
   //         }

   //         if (gport)
   //         {
   //            qdev_connect_gpio_out(gport, pinmap[pin] & 0x0FFF, s->pout_irq[pin]);
   //            qdev_connect_gpio_out_named(gport, STM32_GPIOS_DIR, pinmap[pin] & 0x0FFF, s->pdir_irq[pin]);
   //            s->pin_irq[pin] = qdev_get_gpio_in(gport, pinmap[pin] & 0x0FFF);
   //         }
   //      }
   //   }
   //}

   /* Connect RS232 to UART 1 */
   stm32_uart_connect( (Stm32Uart *)s->uart1, serial_hd(0), STM32_USART1_NO_REMAP );

   /* These additional UARTs have not been tested yet... */
   stm32_uart_connect( (Stm32Uart *)s->uart2, serial_hd(1), STM32_USART2_NO_REMAP );
   stm32_uart_connect( (Stm32Uart *)s->uart3, serial_hd(2), STM32_USART3_NO_REMAP );

   /// FIXME simulide: DeviceState *i2c_master1 = DEVICE(s->i2c1);
   /// FIXME simulide: I2CBus *i2c_bus1 = I2C_BUS(qdev_get_child_bus(i2c_master1, "i2c"));
   /// FIXME simulide: i2c_slave_create_simple(i2c_bus1, "simul_i2c", 0x00);

   /// FIXME simulide: DeviceState *i2c_master2 = DEVICE(s->i2c2);
   /// FIXME simulide: I2CBus *i2c_bus2 = I2C_BUS(qdev_get_child_bus(i2c_master2, "i2c"));
   /// FIXME simulide: i2c_slave_create_simple(i2c_bus2, "simul_i2c", 0x01);

   /// FIXME simulide: DeviceState *spi_master1 = DEVICE(s->spi1);
   /// FIXME simulide: SSIBus *spi_bus1 = (SSIBus *)qdev_get_child_bus(spi_master1, "ssi");
   /// FIXME simulide: ssi_create_peripheral(spi_bus1, "simul_spi");

   /// FIXME simulide: DeviceState *spi_master2 = DEVICE(s->spi2);
   /// FIXME simulide: SSIBus *spi_bus2 = (SSIBus *)qdev_get_child_bus(spi_master2, "ssi");
   /// FIXME simulide: ssi_create_peripheral(spi_bus2, "simul_spi");

   armv7m_load_kernel( ARM_CPU(first_cpu), machine->kernel_filename, 0, FLASH_SIZE );
}

static void stm32_f103c8_machine_init(MachineClass *mc)
{
   mc->desc = "STM32F103C8 MCU";
   mc->init = stm32_f103c8_init;
   mc->block_default_type = IF_IDE;
   mc->ignore_memory_transaction_failures = true;
}

DEFINE_MACHINE("stm32-f103c8-simul", stm32_f103c8_machine_init)
