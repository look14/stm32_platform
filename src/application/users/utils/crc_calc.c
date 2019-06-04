#include "crc_calc.h"


#if !CRC_CALC_OPTIMIZE_EN

void crc16_init(void)
{

}


u16 crc16_calc(u8 buf[], u16 len)
{
	u16 i;
	u16 crc = 0x0000;
    for (; len > 0; len--)              /* Step through bytes in memory */
    {  
        crc = crc ^ (*buf++ << 8);     /* Fetch byte from memory, XOR into CRC top byte*/  
        for (i = 0; i < 8; i++)             /* Prepare to rotate 8 bits */  
        {  
            if (crc & 0x8000)            /* b15 is set... */  
                crc = (crc << 1) ^ CRC_CALC_POLY;    /* rotate and XOR with polynomic */  
            else                          /* b15 is clear... */  
                crc <<= 1;                  /* just rotate */  
        }                             /* Loop for 8 bits */  
        crc &= 0xFFFF;                  /* Ensure CRC remains 16-bit value */  
    }                               /* Loop until len=0 */  

    return (crc ^ 0xFFFF);
}

#endif


#if CRC_CALC_OPTIMIZE_EN

static u16 g_crcTable[256];

void crc16_init(void)  
{  
    u16 remainder;  
    u16 dividend;  
    u8 _bit;  		   
    /* Perform binary long division, a bit at a time. */  
    for(dividend = 0; dividend < 256; dividend++)  
    {  
        /* Initialize the remainder.  */  
        remainder = dividend << 8;
        /* Shift and XOR with the polynomial.   */  
        for(_bit = 0; _bit < 8; _bit++)  
        {  
            /* Try to divide the current data bit.  */  
            if(remainder & 0x8000)  
            {  
                remainder = (remainder << 1) ^ CRC_CALC_POLY;  
            }  
            else  
            {  
                remainder = remainder << 1;  
            }  
        }  
        /* Save the result in the table. */  
        g_crcTable[dividend] = remainder;  
    }  
}


u16 crc16_calc(u8 buf[], u16 len)
{  
	u16 crc = 0x0000;
    u16 offset;  
    u8 byte;

    /* Divide the message by the polynomial, a byte at a time. */  
    for( offset = 0; offset < len; offset++)  
    {  
        byte = (crc >> 8) ^ buf[offset];  
        crc = g_crcTable[byte] ^ (crc << 8);  
    }  
    /* The final remainder is the CRC result. */  
    return (crc ^ 0xFFFF);  
}

#endif

u16 crc16_ibm_calc(const u8 buf[], u16 len)
{
	u16 i;
	u16 crc = 0x0000;

    for (; len > 0; len--)              /* Step through bytes in memory */
    {  
        crc = crc ^ (*buf++ << 8);     /* Fetch byte from memory, XOR into CRC top byte*/  
        for (i = 0; i < 8; i++)             /* Prepare to rotate 8 bits */  
        {  
            if (crc & 0x8000)            /* b15 is set... */  
                crc = (crc << 1) ^ 0x8005;    /* rotate and XOR with polynomic */  
            else                          /* b15 is clear... */  
                crc <<= 1;                  /* just rotate */  
        }                             /* Loop for 8 bits */  
        crc &= 0xFFFF;                  /* Ensure CRC remains 16-bit value */  
    }                               /* Loop until len=0 */  

    return crc;//(crc ^ 0xFFFF);
}

u16 crc16_ccitt_calc(const u8 buf[], u16 len)
{
	u16 i;
	u16 crc = 0x0000;

    for (; len > 0; len--)              /* Step through bytes in memory */
    {  
        crc = crc ^ (*buf++ << 8);     /* Fetch byte from memory, XOR into CRC top byte*/  
        for (i = 0; i < 8; i++)             /* Prepare to rotate 8 bits */  
        {  
            if (crc & 0x8000)            /* b15 is set... */  
                crc = (crc << 1) ^ 0x1021;    /* rotate and XOR with polynomic */  
            else                          /* b15 is clear... */  
                crc <<= 1;                  /* just rotate */  
        }                             /* Loop for 8 bits */  
        crc &= 0xFFFF;                  /* Ensure CRC remains 16-bit value */  
    }                               /* Loop until len=0 */  

    return crc;//(crc ^ 0xFFFF);
}

