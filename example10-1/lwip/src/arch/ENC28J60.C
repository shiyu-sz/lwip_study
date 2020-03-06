#include "enc28j60.h"
 
#include <stdio.h>

static unsigned char Enc28j60Bank;
static unsigned int NextPacketPtr;

extern unsigned char	SPI1_ReadWrite(unsigned char writedat);
extern void USART_OUT(USART_TypeDef* USARTx, uint8_t *Data,...);
/****************************************************************************
* ��    �ƣ�unsigned char enc28j60ReadOp(unsigned char op, unsigned char address)
* ��    �ܣ�ENC28J60���Ĵ���
* ��ڲ�����
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/ 
unsigned char enc28j60ReadOp(unsigned char op, unsigned char address)
{
	unsigned char dat = 0;
	
	ENC28J60_CSL();
	
	dat = op | (address & ADDR_MASK);
	SPI1_ReadWrite(dat);
	dat = SPI1_ReadWrite(0xFF);
	// do dummy read if needed (for mac and mii, see datasheet page 29)
	if(address & 0x80)
	{
		dat = SPI1_ReadWrite(0xFF);
	}
	// release CS
	ENC28J60_CSH();
	return dat;
}
/****************************************************************************
* ��    �ƣ�void enc28j60WriteOp(unsigned char op, unsigned char address, unsigned char data)
* ��    �ܣ�ENC28J60 �Ĵ�����������
* ��ڲ�����op 	 address  data
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/ 
void enc28j60WriteOp(unsigned char op, unsigned char address, unsigned char data)
{
	unsigned char dat = 0;								  	  
	ENC28J60_CSL();	                      //ʹ��ENC28J60 SPIƬѡ  		
	dat = op | (address & ADDR_MASK);	  //OP--3λ������ (address & ADDR_MASK)--5λ����
	SPI1_ReadWrite(dat);				  //SPI1 д
	dat = data;
	SPI1_ReadWrite(dat);				  //SPI1 д��������
	ENC28J60_CSH();						  //��ֹENC28J60 SPIƬѡ  ��ɲ���
}
/****************************************************************************
* ��    �ƣ�void enc28j60ReadBuffer(unsigned int len, unsigned char* data)
* ��    �ܣ�ENC28J60 �����ջ�������
* ��ڲ�����op 	 address  data
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/ 
void enc28j60ReadBuffer(unsigned int len, unsigned char* data)
	{
	ENC28J60_CSL();
	// ������
	SPI1_ReadWrite(ENC28J60_READ_BUF_MEM);
	while(len)
	{
        len--;
        // read data
        *data = (unsigned char)SPI1_ReadWrite(0);
        data++;
	}
	*data='\0';
	ENC28J60_CSH();
}
/****************************************************************************
* ��    �ƣ�void enc28j60WriteBuffer(unsigned int len, unsigned char* data)
* ��    �ܣ�ENC28J60 д���ͻ�������
* ��ڲ�����op 	 address  data
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/ 
void enc28j60WriteBuffer(unsigned int len, unsigned char* data)
{
	ENC28J60_CSL();
	// issue write command
	SPI1_ReadWrite(ENC28J60_WRITE_BUF_MEM);
	
	while(len)
	{
		len--;
		SPI1_ReadWrite(*data);
		data++;
	}
	ENC28J60_CSH();
}
/****************************************************************************
* ��    �ƣ�void enc28j60SetBank(unsigned char address)
* ��    �ܣ�ENC28J60 ���üĴ���BANK
* ��ڲ�����
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/ 
void enc28j60SetBank(unsigned char address)
{
	// set the bank (if needed)
	if((address & BANK_MASK) != Enc28j60Bank)
	{
        // set the bank
        enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
        enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
        Enc28j60Bank = (address & BANK_MASK);
	}
}
/****************************************************************************
* ��    �ƣ�unsigned char enc28j60Read(unsigned char address)
* ��    �ܣ���ȡָ���Ĵ�������ֵ
* ��ڲ�����
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/ 
unsigned char enc28j60Read(unsigned char address)
{
	// set the bank
	enc28j60SetBank(address);
	// do the read
	return enc28j60ReadOp(ENC28J60_READ_CTRL_REG, address);
}
/****************************************************************************
* ��    �ƣ�void enc28j60Write(unsigned char address, unsigned char data)
* ��    �ܣ���ָ���Ĵ���д����ֵ
* ��ڲ�����
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/ 
void enc28j60Write(unsigned char address, unsigned char data)
	{
	// set the bank
	enc28j60SetBank(address);
	// do the write
	enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);
	}
/****************************************************************************
* ��    �ƣ�void enc28j60PhyWrite(unsigned char address, unsigned int data)
* ��    �ܣ���ָ��PHY�Ĵ���д����ֵ
* ��ڲ�����
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/ 
void enc28j60PhyWrite(unsigned char address, unsigned int data)
{
	// set the PHY register address
	enc28j60Write(MIREGADR, address);
	// write the PHY data
	enc28j60Write(MIWRL, data);
	enc28j60Write(MIWRH, data>>8);
	// �ȴ�PHY�Ĵ���д�����
	while(enc28j60Read(MISTAT) & MISTAT_BUSY);
}
/****************************************************************************
* ��    �ƣ�void enc28j60clkout(unsigned char clk)
* ��    �ܣ�����ENC28J60ʱ�����Ƶ��
* ��ڲ�����
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/ 
void enc28j60clkout(unsigned char clk)
{
    //setup clkout: 2 is 12.5MHz:
	enc28j60Write(ECOCON, clk & 0x7);
}
/****************************************************************************
* ��    �ƣ�void enc28j60Init(unsigned char* macaddr)
* ��    �ܣ�ENC28J60��ʼ�� 
* ��ڲ�����*macaddr--6���ֽڵ�MAC��ַ
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/ 
void enc28j60Init(unsigned char* macaddr)
{
	ENC28J60_CSH();	                              //SPI1 ENC28J60Ƭѡ��ֹ  
	/* ENC28J60�����λ 
	   ϵͳ��������λ����SC�� 1 1 1 | 1 1 1 1 1    N/A */
	enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET); 
    /*���ϵ縴λ��ENC28J60 �ӵ��ģʽ�ָ�
	  ���ڷ������ݰ���ʹ�ܽ������ݰ�����
	  ������κ�MAC��MII ��PHY �Ĵ���֮
	  ǰ�������ѯCLKRDY λ��*/
	while(!(enc28j60Read(ESTAT) & ESTAT_CLKRDY)); //    
	// do bank 0 stuff
	// initialize receive buffer
	// 16-bit transfers, must write low byte first
	// ���ý��ջ�������ʼ��ַ
	NextPacketPtr = RXSTART_INIT;
        // Rx start
	//���ջ�������һ��Ӳ�������ѭ��FIFO ���������ɡ�
//�Ĵ�����ERXSTH:ERXSTL ��ERXNDH:ERXNDL ��
//Ϊָ�룬���建���������������ڴ洢���е�λ�á�
//ERXST��ERXNDָ����ֽھ�������FIFO�������ڡ�
//������̫���ӿڽ��������ֽ�ʱ����Щ�ֽڱ�˳��д��
//���ջ������� ���ǵ�д����ERXND ָ��Ĵ洢��Ԫ
//��Ӳ�����Զ������յ���һ�ֽ�д����ERXST ָ��
//�Ĵ洢��Ԫ�� ��˽���Ӳ��������д��FIFO ����ĵ�
//Ԫ��
	enc28j60Write(ERXSTL, RXSTART_INIT&0xFF);	 //
	enc28j60Write(ERXSTH, RXSTART_INIT>>8);
	// set receive pointer address
	//ERXWRPTH:ERXWRPTL �Ĵ�������Ӳ����FIFO ��
    //���ĸ�λ��д������յ����ֽڡ� ָ����ֻ���ģ��ڳ�
    //�����յ�һ�����ݰ���Ӳ�����Զ�����ָ�롣 ָ���
    //�����ж�FIFO ��ʣ��ռ�Ĵ�С  8K-1500�� 
	enc28j60Write(ERXRDPTL, RXSTART_INIT&0xFF);
	enc28j60Write(ERXRDPTH, RXSTART_INIT>>8);
	// RX end  �涨�˽�����λ��0---��0x1fff-0x600-1)
	enc28j60Write(ERXNDL, RXSTOP_INIT&0xFF);
	enc28j60Write(ERXNDH, RXSTOP_INIT>>8);
	// TX start	  0x1fff-0x600
	enc28j60Write(ETXSTL, TXSTART_INIT&0xFF);
	enc28j60Write(ETXSTH, TXSTART_INIT>>8);
	// TX end	  �涨�˽�����λ��0x1fff-0x600---0x1fff	 
	enc28j60Write(ETXNDL, TXSTOP_INIT&0xFF);
	enc28j60Write(ETXNDH, TXSTOP_INIT>>8);
	// do bank 1 stuff, packet filter:
        // For broadcast packets we allow only ARP packtets
        // All other packets should be unicast only for our mac (MAADR)
        //
        // The pattern to match on is therefore
        // Type     ETH.DST
        // ARP      BROADCAST
        // 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
        // in binary these poitions are:11 0000 0011 1111
        // This is hex 303F->EPMM0=0x3f,EPMM1=0x30
    //���չ�����
	//UCEN������������ʹ��λ
    //��ANDOR = 1 ʱ��
	//1 = Ŀ���ַ�뱾��MAC ��ַ��ƥ������ݰ���������
	//0 = ��ֹ������
	//��ANDOR = 0 ʱ��
	//1 = Ŀ���ַ�뱾��MAC ��ַƥ������ݰ��ᱻ����
	//0 = ��ֹ������

    //CRCEN���������CRC У��ʹ��λ
	//1 = ����CRC ��Ч�����ݰ�����������
	//0 = ������CRC �Ƿ���Ч
	
	//PMEN����ʽƥ�������ʹ��λ
	//��ANDOR = 1 ʱ��
	//1 = ���ݰ�������ϸ�ʽƥ�����������򽫱�����
	//0 = ��ֹ������
	//��ANDOR = 0 ʱ��
	//1 = ���ϸ�ʽƥ�����������ݰ���������
	//0 = ��ֹ������
	enc28j60Write(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);
	enc28j60Write(EPMM0, 0x3f);
	enc28j60Write(EPMM1, 0x30);
	enc28j60Write(EPMCSL, 0xf9);
	enc28j60Write(EPMCSH, 0xf7);
        //
        //
	// do bank 2 stuff
	// enable MAC receive
	//bit 0 MARXEN��MAC ����ʹ��λ
		//1 = ����MAC �������ݰ�
		//0 = ��ֹ���ݰ�����
	//bit 3 TXPAUS����ͣ����֡����ʹ��λ
		//1 = ����MAC ������ͣ����֡������ȫ˫��ģʽ�µ��������ƣ�
		//0 = ��ֹ��ͣ֡����
	//bit 2 RXPAUS����ͣ����֡����ʹ��λ
		//1 = �����յ���ͣ����֡ʱ����ֹ���ͣ�����������
		//0 = ���Խ��յ�����ͣ����֡
	enc28j60Write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	// bring MAC out of reset
	//��MACON2 �е�MARST λ���㣬ʹMAC �˳���λ״̬��
	enc28j60Write(MACON2, 0x00);
	// enable automatic padding to 60bytes and CRC operations
	//bit 7-5 PADCFG2:PACDFG0���Զ�����CRC ����λ
		//111 = ��0 ������ж�֡��64 �ֽڳ�����׷��һ����Ч��CRC
		//110 = ���Զ�����֡
		//101 = MAC �Զ�������8100h �����ֶε�VLAN Э��֡�����Զ���䵽64 �ֽڳ��������
		//��VLAN ֡���������60 �ֽڳ�������Ҫ׷��һ����Ч��CRC
		//100 = ���Զ�����֡
		//011 = ��0 ������ж�֡��64 �ֽڳ�����׷��һ����Ч��CRC
		//010 = ���Զ�����֡
		//001 = ��0 ������ж�֡��60 �ֽڳ�����׷��һ����Ч��CRC
		//000 = ���Զ�����֡
	//bit 4 TXCRCEN������CRC ʹ��λ
		//1 = ����PADCFG��Σ�MAC�����ڷ���֡��ĩβ׷��һ����Ч��CRC�� ���PADCFG�涨Ҫ
		//׷����Ч��CRC������뽫TXCRCEN ��1��
		//0 = MAC����׷��CRC�� ������4 ���ֽڣ����������Ч��CRC �򱨸������״̬������
	//bit 0 FULDPX��MAC ȫ˫��ʹ��λ
		//1 = MAC������ȫ˫��ģʽ�¡� PHCON1.PDPXMD λ������1��
		//0 = MAC�����ڰ�˫��ģʽ�¡� PHCON1.PDPXMD λ�������㡣
	/* ��ʾ ����ENC28J60��֧��802.3���Զ�Э�̻��ƣ� ���ԶԶ˵����翨��Ҫǿ������Ϊȫ˫�� */
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);
	// set inter-frame gap (non-back-to-back)
	//���÷Ǳ��Ա�������Ĵ����ĵ��ֽ�
	//MAIPGL�� �����Ӧ��ʹ��12h ��̸üĴ�����
	//���ʹ�ð�˫��ģʽ��Ӧ��̷Ǳ��Ա�������
	//�Ĵ����ĸ��ֽ�MAIPGH�� �����Ӧ��ʹ��0Ch
	//��̸üĴ�����
	enc28j60Write(MAIPGL, 0x12);
	enc28j60Write(MAIPGH, 0x0C);
	// set inter-frame gap (back-to-back)
	//���ñ��Ա�������Ĵ���MABBIPG����ʹ��
	//ȫ˫��ģʽʱ�������Ӧ��ʹ��15h ��̸üĴ�
	//������ʹ�ð�˫��ģʽʱ��ʹ��12h ���б�̡�
	enc28j60Write(MABBIPG, 0x15);
	// Set the maximum packet size which the controller will accept
    // Do not send packets longer than MAX_FRAMELEN:
	// ���֡����  1500
	enc28j60Write(MAMXFLL, MAX_FRAMELEN&0xFF);	
	enc28j60Write(MAMXFLH, MAX_FRAMELEN>>8);
	// do bank 3 stuff
	// write MAC address
	// NOTE: MAC address in ENC28J60 is byte-backward
	enc28j60Write(MAADR5, macaddr[0]);	
	enc28j60Write(MAADR4, macaddr[1]);
	enc28j60Write(MAADR3, macaddr[2]);
	enc28j60Write(MAADR2, macaddr[3]);
	enc28j60Write(MAADR1, macaddr[4]);
	enc28j60Write(MAADR0, macaddr[5]);
	if(enc28j60Read(MAADR5)== macaddr[0]){
		printf("MAADR5 = %x\r\n", enc28j60Read(MAADR5));
		printf("MAADR4 = %x\r\n", enc28j60Read(MAADR4));
		printf("MAADR3 = %x\r\n", enc28j60Read(MAADR3));
		printf("MAADR2 = %x\r\n", enc28j60Read(MAADR2));
		printf("MAADR1 = %x\r\n", enc28j60Read(MAADR1));
		printf("MAADR0 = %x\r\n", enc28j60Read(MAADR0));
	}
	//����PHYΪȫ˫��  LEDBΪ������
	enc28j60PhyWrite(PHCON1, PHCON1_PDPXMD);


	// no loopback of transmitted frames	 ��ֹ����
    //HDLDIS��PHY ��˫�����ؽ�ֹλ
		//��PHCON1.PDPXMD = 1 ��PHCON1.PLOOPBK = 1 ʱ��
		//��λ�ɱ����ԡ�
		//��PHCON1.PDPXMD = 0 ��PHCON1.PLOOPBK = 0 ʱ��
		//1 = Ҫ���͵����ݽ�ͨ��˫���߽ӿڷ���
		//0 = Ҫ���͵����ݻỷ�ص�MAC ��ͨ��˫���߽ӿڷ���
	enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);
	// switch to bank 0
	//ECON1 �Ĵ���
		//�Ĵ���3-1 ��ʾΪECON1 �Ĵ����������ڿ���
		//ENC28J60 ����Ҫ���ܡ� ECON1 �а�������ʹ�ܡ���
		//������DMA ���ƺʹ洢��ѡ��λ��
	
	enc28j60SetBank(ECON1);
	// enable interrutps
	//EIE�� ��̫���ж�����Ĵ���
	//bit 7 INTIE�� ȫ��INT �ж�����λ
		//1 = �����ж��¼�����INT ����
		//0 = ��ֹ����INT ���ŵĻ������ʼ�ձ�����Ϊ�ߵ�ƽ��
	//bit 6 PKTIE�� �������ݰ��������ж�����λ
		//1 = ����������ݰ��������ж�
		//0 = ��ֹ�������ݰ��������ж�
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE|EIE_RXERIE);
	// enable packet reception
	//bit 2 RXEN������ʹ��λ
		//1 = ͨ����ǰ�����������ݰ�����д����ջ�����
		//0 = �������н��յ����ݰ�
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
	}

/****************************************************************************
* ��    �ƣ�unsigned char enc28j60getrev(void)
* ��    �ܣ���ȡENC28J60�İ汾��Ϣ
* ��ڲ�����
* ���ڲ�������
* ˵    ����
* ���÷�����
****************************************************************************/ 
unsigned char enc28j60getrev(void)
{
	//��EREVID ��Ҳ�洢�˰汾��Ϣ�� EREVID ��һ��ֻ����
	//�ƼĴ���������һ��5 λ��ʶ����������ʶ�����ض���Ƭ
	//�İ汾��
	return(enc28j60Read(EREVID));
}

/****************************************************************************
* ��    �ƣ�void enc28j60PacketSend(unsigned int len, unsigned char* packet)
* ��    �ܣ�ͨ��ENC28J60��������
* ��ڲ�����
* ���ڲ�����
* ˵    ����
* ���÷�����
****************************************************************************/ 
void enc28j60PacketSend(unsigned int len, unsigned char* packet)
	{
	// ���÷��ͻ�������ַдָ����� Set the write pointer to start of transmit buffer area
	while((enc28j60Read(ECON1) & ECON1_TXRTS)!=0); //    
	enc28j60Write(EWRPTL, TXSTART_INIT&0xFF);
	enc28j60Write(EWRPTH, TXSTART_INIT>>8);

	//����TXNDָ�룬�Զ�Ӧ���������ݰ���С	   
	enc28j60Write(ETXNDL, (TXSTART_INIT+len)&0xFF);
	enc28j60Write(ETXNDH, (TXSTART_INIT+len)>>8);

	// дÿ�������ֽڣ�0x00��ʾʹ��macon3�����ã� 
	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

	// �����ݰ����Ƶ����ͻ�����	
	enc28j60WriteBuffer(len, packet);

	// �������Ϸ��ͷ��ͻ�����������  
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);

    // ��λ�����߼������⡣�μ� Rev. B4 Silicon Errata point 12.
	if( (enc28j60Read(EIR) & EIR_TXERIF) )
	{
		enc28j60SetBank(ECON1);
        enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
		//enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
    }
}
/****************************************************************************
* ��    �ƣ�unsigned int enc28j60PacketReceive(unsigned int maxlen, unsigned char* )
* ��    �ܣ�																		 
* ��ڲ�������������ջ�������ȡһ��
			maxlen�� �����������ݰ������ɽ��ܵĳ���
			packet:  ���ݰ���ָ��													
* ���ڲ���: ���һ�����ݰ��յ��������ݰ����ȣ����ֽ�Ϊ��λ������Ϊ�㡣
* ˵    ����
* ���÷�����
****************************************************************************/ 
unsigned int enc28j60PacketReceive(unsigned int maxlen, unsigned char* packet)
{
	unsigned int rxstat;
	unsigned int len;

	//����Ƿ��յ�һ����
	if( enc28j60Read(EPKTCNT) ==0 )  			//�յ�����̫�����ݰ�����   EPKCNT�м�¼�˽��յ�����̫���������ݳ�����Ϣ��
	{
		return(0);
    }

	// ���ý��ջ�������ָ��
	enc28j60Write(ERDPTL, (NextPacketPtr));
	enc28j60Write(ERDPTH, (NextPacketPtr)>>8);

	// ����һ������ָ��
	NextPacketPtr  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	NextPacketPtr |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;

	// �����ĳ���
	len  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	len |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;

    len-=4; 				//ɾ��CRC����
	// ������״̬
	rxstat  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	rxstat |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;

	// ���Ƽ����ĳ���	  
    if (len>maxlen-1)
	{
        len=maxlen-1;
   	}

    // ���CRC�ͷ��Ŵ���
    //  ERXFCON.CRCEN��Ĭ�����á�ͨ�����ǲ���Ҫ���
    if ((rxstat & 0x80)==0)
	{
	   //��Ч��
	   len=0;
	}
	else
	{
       // �ӽ��ջ������и������ݰ�
        enc28j60ReadBuffer(len, packet);
    }

	//RX��ָ���ƶ�����һ�����յ������ݰ��Ŀ�ʼλ�� 
	//�ͷ����ǸղŶ��������ڴ�
	enc28j60Write(ERXRDPTL, (NextPacketPtr));
	enc28j60Write(ERXRDPTH, (NextPacketPtr)>>8);

	//�ݼ����ݰ���������־�����Ѿ��õ�������� 
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	return(len);
}

void mymacinit(unsigned char *mymac)
{
    enc28j60Init(mymac);
  	enc28j60PhyWrite(PHLCON,0x0476);	
	enc28j60clkout(2);                 // change clkout from 6.25MHz to 12.5MHz
}


