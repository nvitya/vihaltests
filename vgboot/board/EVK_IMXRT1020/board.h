/*
 *  file:     board.h
 *  brief:    Board definition for the IMXRT1020-EVK
 *  created:  2023-09-16
 *  authors:  nvitya
*/

#ifndef BOARD_H_
#define BOARD_H_

#define PRINTF_SUPPORT_FLOAT

#define BOARD_NAME "IMXRT1020-EVK"
#define MCU_IMXRT1021
#define EXTERNAL_XTAL_HZ   24000000

#define CONUART_UART_SPEED   115200
#define HAS_SDRAM                 1
#define SPI_SELF_FLASHING         1

#define QSPI_SPEED         30000000
#define QSPI_LINE_COUNT           4

#define FLADDR_APPLICATION  0x10000  // app starts at the position 0x10000 (app header first)

// DMA channel allocation
#define DMACH_QSPI_TX          0  // lowest priority
#define DMACH_QSPI_RX          1

//---------------------------------------
// RAM - the linker scripts must be
//       modified accordingly

#define BOOTLOADER_LOAD_ADDR   0x20008000
#define BOOTLOADER_MAX_SIZE   (32 * 1024)
#define APP_CODE_LOAD_ADDR     0x00000000


/* FlexRAM Configuration by fuses

                      (o=OCRAM(01), I=ITCRAM(11), D=DTCRAM(10))

CFG   DTCM    ITCM ORAM

0000:   64   64  128  {o o D D I I o o} = 5FA5

Default:
 CFG   DTCM ITCM ORAM
 0000:   64   64  128  {o o D D I I o o} = 5FA5
                         5   A   F   5    <---- reverse order !
Change to -->
       DTCM ITCM ORAM
         64  160   32  {o I D D I I I I}
                         7   A   F   F    <---- reverse order !
*/

#define FLEXRAM_ITC_SIZE  (160 * 1024)
#define FLEXRAM_DTC_SIZE  ( 64 * 1024)
#define FLEXRAM_OCR_SIZE  ( 32 * 1024)

#define FLEXRAM_CONFIG          0x0000FFA7  // 160k ITC, 64k DTC, 32k OCRAM
#define FLEXRAM_CONFIG_REBOOT   0x0000FFA7  // must contain at least 32k OCRAM
#define FLEXRAM_OCRAM_SIZE      (32 *1024)


#endif /* BOARD_H_ */
