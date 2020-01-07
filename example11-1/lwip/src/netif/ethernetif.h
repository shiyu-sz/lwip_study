#define        BaseAddr         0x06000000

#define       REGISTERADDR(x)   (BaseAddr+x)

#define        rRWPORT          REGISTERADDR(10)         //读写地址

#define        rRESETPORT       REGISTERADDR(18)         //复位地址

/* Page 0 */
//Page0的可写寄存器

#define        rCR                 REGISTERADDR(0)  //CR寄存器

#define        rPSTART             REGISTERADDR(1)  //开始页面寄存器

#define        rPSTOP              REGISTERADDR(2)  //停止页面寄存器

#define        rBNRY               REGISTERADDR(3)  //边界寄存器

#define        rTPSR               REGISTERADDR(4)  //传送页开始寄存器

#define        rTBCR0              REGISTERADDR(5)  //传输字节计数计算器（低位）

#define        rTBCR1              REGISTERADDR(6)  //传输字节计数计算器（高位）

#define        rISR                REGISTERADDR(7)  //中断状态寄存器

#define        rRSAR0              REGISTERADDR(8)  //远程DMA地址寄存器（低位）

#define        rRSAR1              REGISTERADDR(9)  //远程DMA地址寄存器（高位）

#define        rRBCR0              REGISTERADDR(10)         //远程DMA字节计数寄存器（低位）

#define        rRBCR1              REGISTERADDR(11)         //远程DMA字节计数寄存器（高位）

#define        rRCR                REGISTERADDR(12)         //接收配置寄存器

#define        rTCR                REGISTERADDR(13)         //发送配置寄存器

#define        rDCR                REGISTERADDR(14)         //数据配置寄存器

#define        rIMR                REGISTERADDR(8)  //中断屏蔽寄存器

//Page0的只读寄存器，对应上述的寄存器

#define       rCLDA0             REGISTERADDR(1)  //本地DMA地址（低位）

#define        rCLDA1            REGISTERADDR(2)  //本地DMA地址（高位）

#define       rTSR               REGISTERADDR(4)  //发送状态寄存器

#define       rNCR               REGISTERADDR(5)  //碰撞数量寄存器

#define       rFIFO              REGISTERADDR(6)  //FIFO

#define       rCRDA0             REGISTERADDR(8)  //远程DMA地址（低位）

#define       rCRDA1             REGISTERADDR(9)  //远程DMA地址（高位）

#define       r8019ID0           REGISTERADDR(10)         //8019ID寄存器（低位）

#define       r8019ID1           REGISTERADDR(11)         //8019ID寄存器（低位）

#define        rRSR              REGISTERADDR(12)         //接收状态寄存器

#define        rCNTR0            REGISTERADDR(13)         //接收对齐错误寄存器

#define       rCNTR1             REGISTERADDR(14)         //接收CRC错误计数器

#define       rCNTR2             REGISTERADDR(8)  //接收丢失帧错误寄存器

/* Page 1 */

#define        rPAR0                 REGISTERADDR(1)  //物理地址寄存器

#define        rPAR1                 REGISTERADDR(2)

#define        rPAR2                 REGISTERADDR(3)

#define        rPAR3                 REGISTERADDR(4)

#define        rPAR4                 REGISTERADDR(5)

#define        rPAR5                 REGISTERADDR(6)

#define        rCURR                REGISTERADDR(7)  //当前页面寄存器

#define        rMAR0               REGISTERADDR(8)  //多播地址寄存器

#define        rMAR1               REGISTERADDR(9)

#define        rMAR2               REGISTERADDR(10)

#define        rMAR3               REGISTERADDR(11)

#define        rMAR4               REGISTERADDR(12)

#define        rMAR5               REGISTERADDR(13)

#define        rMAR6               REGISTERADDR(14)

#define        rMAR7               REGISTERADDR(8)

/*Page2，下列寄存器均只读*/

#define        rPSTART             REGISTERADDR(1)

#define        rPSTOP              REGISTERADDR(2)

#define        rTPSR               REGISTERADDR(4)

#define        rRCR                REGISTERADDR(12)

#define        rTCR                REGISTERADDR(13)

#define        rDCR                REGISTERADDR(14)

#define        rIMR                REGISTERADDR(8)

/* Page 3 */
#define        rCR9346             REGISTERADDR(1)
#define        rCONFIG0            REGISTERADDR(3)
#define        rCONFIG1            REGISTERADDR(4)
#define        rCONFIG2            REGISTERADDR(5)
#define        rCONFIG3            REGISTERADDR(6)