/*
 *  file:     board.h
 *  brief:    Board definition for the IMXRT1020-EVK
 *  created:  2023-09-16
 *  authors:  nvitya
*/

#ifndef BOARD_H_
#define BOARD_H_

#define PRINTF_SUPPORT_FLOAT

#ifdef MCU_IMXRT1052A
  #define BOARD_NAME "IMXRT1050-EVKA"
#else
  #define BOARD_NAME "IMXRT1050-EVKB"
  #define MCU_IMXRT1052
#endif

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

#define BOOTLOADER_LOAD_ADDR   0x20010000
#define BOOTLOADER_MAX_SIZE   (64 * 1024)
#define APP_CODE_LOAD_ADDR     0x00000000


/* FlexRAM Configuration

                      (o=OCRAM(01), I=ITCRAM(11), D=DTCRAM(10))
Default:
 CFG   DTCM ITCM ORAM
 0000:  128  128  256  {o o o o D D I I I I D D o o o o}
                         5   5   A   F   F   A   5   5     <---- reverse order !

Change to -->
       DTCM ITCM ORAM
        128  256  128  {o o o o D D I I I I D D I I I I}
                         5   5   A   F   F   A   F   F     <---- reverse order !
*/

#define FLEXRAM_ITC_SIZE  (256 * 1024)
#define FLEXRAM_DTC_SIZE  (128 * 1024)
#define FLEXRAM_OCR_SIZE  (128 * 1024)

#define FLEXRAM_CONFIG          0xFFAFFA55  // 256k ITC, 128k DTC, 128k OCRAM
#define FLEXRAM_CONFIG_REBOOT   0xFFAFFA55  // must contain at least 32k OCRAM
#define FLEXRAM_OCRAM_SIZE      (128 * 1024)


#endif /* BOARD_H_ */
