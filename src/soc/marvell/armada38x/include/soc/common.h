#ifndef __SOC_MARVELL_ARMADA38X_COMMON_H_
#define __SOC_MARVELL_ARMADA38X_COMMON_H_

#include <types.h>
#include <arch/io.h>

#define INTER_REGS_BASE         0xF1000000
#define MV_REG_READ(offset)             \
        (read32((void *)(INTER_REGS_BASE | (offset))))
#define MV_REG_WRITE(offset, val)    \
        (write32((void *)(INTER_REGS_BASE | (offset)), (val)))
#define MV_REG_BIT_SET(offset, bitMask)                 \
        (write32((void *)(INTER_REGS_BASE | (offset)), \
         (read32((void *)(INTER_REGS_BASE | (offset))) | bitMask)))
#define MV_REG_BIT_RESET(offset,bitMask)                \
        (write32((void *)(INTER_REGS_BASE | (offset)), \
         (read32((void *)(INTER_REGS_BASE | (offset))) & (~bitMask))))


#define NO_BIT      0x00000000
#define BIT0        0x00000001
#define BIT1        0x00000002
#define BIT2        0x00000004
#define BIT3        0x00000008
#define BIT4        0x00000010
#define BIT5        0x00000020
#define BIT6        0x00000040
#define BIT7        0x00000080
#define BIT8        0x00000100
#define BIT9        0x00000200
#define BIT10       0x00000400
#define BIT11       0x00000800
#define BIT12       0x00001000
#define BIT13       0x00002000
#define BIT14       0x00004000
#define BIT15       0x00008000

#define MV_TRUE             (1)
#define MV_FALSE            (0)

/* The following is a list of Marvell status    */
#define MV_ERROR            (-1)
#define MV_OK               (0) /* Operation succeeded                   */
#define MV_FAIL             (1) /* Operation failed                      */
#define MV_BAD_VALUE        (2) /* Illegal value (general)               */
#define MV_OUT_OF_RANGE     (3) /* The value is out of range             */
#define MV_BAD_PARAM        (4) /* Illegal parameter in function called  */
#define MV_BAD_PTR          (5) /* Illegal pointer value                 */
#define MV_BAD_SIZE         (6) /* Illegal size                          */
#define MV_BAD_STATE        (7) /* Illegal state of state machine        */
#define MV_SET_ERROR        (8) /* Set operation failed                  */
#define MV_GET_ERROR        (9) /* Get operation failed                  */
#define MV_CREATE_ERROR     (10)        /* Fail while creating an item           */
#define MV_NOT_FOUND        (11)        /* Item not found                        */
#define MV_NO_MORE          (12)        /* No more items found                   */
#define MV_NO_SUCH          (13)        /* No such item                          */
#define MV_TIMEOUT          (14)        /* Time Out                              */
#define MV_NO_CHANGE        (15)        /* Parameter(s) is already in this value */
#define MV_NOT_SUPPORTED    (16)        /* This request is not support           */
#define MV_NOT_IMPLEMENTED  (17)        /* Request supported but not implemented */
#define MV_NOT_INITIALIZED  (18)        /* The item is not initialized           */
#define MV_NO_RESOURCE      (19)        /* Resource not available (memory ...)   */
#define MV_FULL             (20)        /* Item is full (Queue or table etc...)  */
#define MV_EMPTY            (21)        /* Item is empty (Queue or table etc...) */
#define MV_INIT_ERROR       (22)        /* Error occured while INIT process      */
#define MV_HW_ERROR         (23)        /* Hardware error                        */
#define MV_TX_ERROR         (24)        /* Transmit operation not succeeded      */
#define MV_RX_ERROR         (25)        /* Recieve operation not succeeded       */
#define MV_NOT_READY        (26)        /* The other side is not ready yet       */
#define MV_ALREADY_EXIST    (27)        /* Tried to create existing item         */
#define MV_OUT_OF_CPU_MEM   (28)        /* Cpu memory allocation failed.         */
#define MV_NOT_STARTED      (29)        /* Not started yet                       */
#define MV_BUSY             (30)        /* Item is busy.                         */
#define MV_TERMINATE        (31)        /* Item terminates it's work.            */
#define MV_NOT_ALIGNED      (32)        /* Wrong alignment                       */
#define MV_NOT_ALLOWED      (33)        /* Operation NOT allowed                 */
#define MV_WRITE_PROTECT    (34)        /* Write protected                       */
#define MV_DROPPED          (35)        /* Packet dropped                        */
#define MV_STOLEN           (36)        /* Packet stolen */
#define MV_CONTINUE         (37)        /* Continue */
#define MV_RETRY            (38)        /* Operation failed need retry           */

#define MV_INVALID          (int)(-1)

#define MV_BOARD_TCLK_200MHZ    200000000
#define MV_BOARD_TCLK_250MHZ    250000000

#define MPP_SAMPLE_AT_RESET     (0x18600)

#define MV_6810_DEV_ID          0x6810
#endif // __SOC_MARVELL_ARMADA38X_COMMON_H__
