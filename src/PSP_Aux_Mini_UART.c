
#include "PSP_Aux_Mini_UART.h"
#include "PSP_REGS.h"
#include "PSP_GPIO.h"

/*------------------------------------------------------------------------------------------------
    Private PSP_Aux_Mini_UART Defines
 -------------------------------------------------------------------------------------------------*/

// AUX Register Addresses
#define PSP_AUX_BASE_ADDRESS      (PSP_REGS_AUX_BASE_ADDRESS)

#define PSP_AUX_IRQ_A             (PSP_AUX_BASE_ADDRESS | 0x00000000u) // Auxiliary Interrupt status address
#define PSP_AUX_ENABLES_A         (PSP_AUX_BASE_ADDRESS | 0x00000004u) // Auxiliary enables address

#define PSP_AUX_MU_IO_REG_A       (PSP_AUX_BASE_ADDRESS | 0x00000040u) // Mini Uart I/O Data address
#define PSP_AUX_MU_IER_REG_A      (PSP_AUX_BASE_ADDRESS | 0x00000044u) // Mini Uart Interrupt Enable address
#define PSP_AUX_MU_IIR_REG_A      (PSP_AUX_BASE_ADDRESS | 0x00000048u) // Mini Uart Interrupt Identify address
#define PSP_AUX_MU_LCR_REG_A      (PSP_AUX_BASE_ADDRESS | 0x0000004Cu) // Mini Uart Line Control address
#define PSP_AUX_MU_MCR_REG_A      (PSP_AUX_BASE_ADDRESS | 0x00000050u) // Mini Uart Modem Control address
#define PSP_AUX_MU_LSR_REG_A      (PSP_AUX_BASE_ADDRESS | 0x00000054u) // Mini Uart Line Status address
#define PSP_AUX_MU_MSR_REG_A      (PSP_AUX_BASE_ADDRESS | 0x00000058u) // Mini Uart Modem Status address
#define PSP_AUX_MU_SCRATCH_A      (PSP_AUX_BASE_ADDRESS | 0x0000005Cu) // Mini Uart Scratch address
#define PSP_AUX_MU_CNTL_REG_A     (PSP_AUX_BASE_ADDRESS | 0x00000060u) // Mini Uart Extra Control address
#define PSP_AUX_MU_STAT_REG_A     (PSP_AUX_BASE_ADDRESS | 0x00000064u) // Mini Uart Extra Status address
#define PSP_AUX_MU_BAUD_REG_A     (PSP_AUX_BASE_ADDRESS | 0x00000068u) // Mini Uart Baudrate address

// AUX Register Pointers
#define PSP_AUX_IRQ_R             (*((volatile uint32_t *)PSP_AUX_IRQ_A))             // Auxiliary Interrupt status register
#define PSP_AUX_ENABLES_R         (*((volatile uint32_t *)PSP_AUX_ENABLES_A))         // Auxiliary enables register

#define PSP_AUX_MU_IO_REG_R       (*((volatile uint32_t *)PSP_AUX_MU_IO_REG_A))       // Mini Uart I/O Data register
#define PSP_AUX_MU_IER_REG_R      (*((volatile uint32_t *)PSP_AUX_MU_IER_REG_A))      // Mini Uart Interrupt Enable register
#define PSP_AUX_MU_IIR_REG_R      (*((volatile uint32_t *)PSP_AUX_MU_IIR_REG_A))      // Mini Uart Interrupt Identify register
#define PSP_AUX_MU_LCR_REG_R      (*((volatile uint32_t *)PSP_AUX_MU_LCR_REG_A))      // Mini Uart Line Control register
#define PSP_AUX_MU_MCR_REG_R      (*((volatile uint32_t *)PSP_AUX_MU_MCR_REG_A))      // Mini Uart Modem Control register
#define PSP_AUX_MU_LSR_REG_R      (*((volatile uint32_t *)PSP_AUX_MU_LSR_REG_A))      // Mini Uart Line Status register
#define PSP_AUX_MU_MSR_REG_R      (*((volatile uint32_t *)PSP_AUX_MU_MSR_REG_A))      // Mini Uart Modem Status register
#define PSP_AUX_MU_SCRATCH_R      (*((volatile uint32_t *)PSP_AUX_MU_SCRATCH_A))      // Mini Uart Scratch register
#define PSP_AUX_MU_CNTL_REG_R     (*((volatile uint32_t *)PSP_AUX_MU_CNTL_REG_A))     // Mini Uart Extra Control register
#define PSP_AUX_MU_STAT_REG_R     (*((volatile uint32_t *)PSP_AUX_MU_STAT_REG_A))     // Mini Uart Extra Status register
#define PSP_AUX_MU_BAUD_REG_R     (*((volatile uint32_t *)PSP_AUX_MU_BAUD_REG_A))     // Mini Uart Baudrate register

// AUX IRQ Register Masks
#define AUX_MINI_UART_IRQ    0b001u // If set the mini UART has an interrupt pending
#define AUX_SPI_1_IRQ        0b010u // If set the AUX SPI 1 module has an interrupt pending
#define AUX_SPI_2_IRQ        0b100u // If set the AUX SPI 2 module has an interrupt pending

// AUX Enable Register masks
#define AUX_MINI_UART_ENABLE 0b001u // If set the mini UART is enabled
#define AUX_SPI_1_ENABLE     0b010u // If set the AUX SPI 1 module is enabled
#define AUX_SPI_2_ENABLE     0b100u // If set the AUX SPI 2 module is enabled

// Mini UART Line Control Register Masks
#define AUX_MU_LCR_DLAB_ACCESS 0x80u // give access the the Baudrate register.
#define AUX_MU_LCR_BREAK       0x40u // If set high the UART1_TX line is pulled low continuously
#define AUX_MU_LCR_7_BIT_MODE  0x00u // the UART works in 7-bit mode
#define AUX_MU_LCR_8_BIT_MODE  0x03u // the UART works in 8-bit mode

// Mini UART Modem Control Register Masks
#define AUX_MU_MCR_RTS 0x02u // If clear the UART1_RTS line is high. If set the UART1_RTS line is low

// Mini UART Line Status Register MASKS
#define AUX_MU_LSR_TRANSMITTER_IDLE  0x40u // This bit is set if the transmit FIFO is empty and the transmitter is idle
#define AUX_MU_LSR_TRANSMITTER_EMPTY 0x20u // This bit is set if the transmit FIFO can accept at least one byte
#define AUX_MU_LSR_RECIEVER_OVERRUN  0x02u // This bit is set if there was a receiver overrun
#define AUX_MU_LSR_DATA_READY        0x01u // This bit is set if the receive FIFO holds at least 1 symbol

// Mini UART Modem Status Register Masks
#define AUX_MU_MSR_CTS_STATUS 0x20u // This bit is the inverse of the UART1_CTS input

// Mini UART Extra Control Register Masks
#define AUX_MU_CNTL_CTS_ASSERT_LEVEL 0x80u        // This bit allows one to invert the CTS auto flow operation polarity
#define AUX_MU_CNTL_RTS_ASSERT_LEVEL 0x40u        // This bit allows one to invert the RTS auto flow operation polarity

#define AUX_MU_CNTL_RTS_AUTO_FLOW_LEVEL_0 0x20u   // These two bits specify at what receiver FIFO level the R/W
#define AUX_MU_CNTL_RTS_AUTO_FLOW_LEVEL_1 0x10u   // RTS line is de-asserted in auto-flow mode (pg. 16)

#define AUX_MU_CNTL_EN_TR_AUTO_FC_USING_CTS 0x08u // If this bit is set the transmitter will stop if the CTS line is de-asserted
#define AUX_MU_CNTL_EN_RC_AUTO_FC_USING_RTS 0x04u // If this bit is set the RTS line will de-assert if the receive FIFO reaches it 'auto flow' level

#define AUX_MU_CNTL_TRANSMITTER_ENABLE 0x02u      // If this bit is set the mini UART transmitter is enabled
#define AUX_MU_CNTL_RECIEVER_ENABLE    0x01u      // If this bit is set the mini UART receiver is enabled

// Mini UART Extra Status Register Masks
#define AUX_MU_STAT_TRANSMITTER_DONE 0x200u // This bit is set if the transmitter is idle and the transmit R FIFO is empty
#define AUX_MU_STAT_TR_FIFO_IS_EMPTY 0x100u // If this bit is set the transmitter FIFO is empty
#define AUX_MU_STAT_CTS_LINE         0x080u // This bit shows the status of the UART1_CTS line
#define AUX_MU_STAT_RTS_STATUS       0x040u // This bit shows the status of the UART1_RTS line
#define AUX_MU_STAT_TR_FIFO_IS_FULL  0x020u // This is the inverse of bit 1
#define AUX_MU_STAT_RECIEVER_OVERRUN 0x010u // This bit is set if there was a receiver overrun
#define AUX_MU_STAT_TR_IS_IDLE       0x008u // If this bit is set the transmitter is idle
#define AUX_MU_STAT_RC_IS_IDLE       0x004u // If this bit is set the receiver is idle
#define AUX_MU_STAT_SPACE_AVAILABLE  0x002u // If this bit is set the mini UART transmitter FIFO can accept at least one more symbol
#define AUX_MU_STAT_SYMBOL_AVAILABLE 0x001u // If this bit is set the mini UART receive FIFO contains at least 1 symbol




/*-----------------------------------------------------------------------------------------------
    PSP_Aux_Mini_UART Function Definitions
 -------------------------------------------------------------------------------------------------*/

void PSP_AUX_Mini_Uart_Init(PSP_AUX_Mini_Uart_Baud_Rate_t baud_rate_enum)
{
    // set pins 14 and 15 to alt pin mode 5 for mini uart
    PSP_GPIO_Set_Pin_Mode(PSP_AUX_MINI_UART_TX_PIN, PSP_GPIO_PINMODE_ALT5);
    PSP_GPIO_Set_Pin_Mode(PSP_AUX_MINI_UART_RX_PIN, PSP_GPIO_PINMODE_ALT5);

    // enable the mini uart
    PSP_AUX_ENABLES_R |= AUX_MINI_UART_ENABLE;

    // disable mini uart interrupts
    PSP_AUX_MU_IER_REG_R = 0u;

    // zero out the control register
    PSP_AUX_MU_CNTL_REG_R = 0u;

    // enable 8 bit mode
    PSP_AUX_MU_LCR_REG_R = AUX_MU_LCR_8_BIT_MODE;

    // set RTS line to be high
    PSP_AUX_MU_MCR_REG_R = 0;

    // set the baud rate
    PSP_AUX_Mini_Uart_Set_Baud_Rate(baud_rate_enum);

    // enable transmitter and reciever
    PSP_AUX_MU_CNTL_REG_R |= AUX_MU_CNTL_TRANSMITTER_ENABLE | AUX_MU_CNTL_RECIEVER_ENABLE;
}



void PSP_AUX_Mini_Uart_Set_Baud_Rate(PSP_AUX_Mini_Uart_Baud_Rate_t baud_rate_enum)
{
    PSP_AUX_MU_BAUD_REG_R = baud_rate_enum;
}



void PSP_AUX_Mini_Uart_Send_Byte(uint8_t value)
{
    while (!(PSP_AUX_MU_LSR_REG_R & AUX_MU_LSR_TRANSMITTER_EMPTY))
    {
        // wait until the transmitter can accept data
    }

    // write the value to the I/O register
    PSP_AUX_MU_IO_REG_R = value;
}



void PSP_AUX_Mini_Uart_Send_String(char * c_string)
{
    
    for (int i = 0u; c_string[i] != '\0'; i++)
    {
        PSP_AUX_Mini_Uart_Send_Byte(c_string[i]);
    }
}
