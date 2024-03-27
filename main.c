#include <stdint.h>
#include <string.h>

#include "CH579SFR.h"
#include "core_cm0.h"

void write_str(const char *s) {
	for (uint32_t i = 0; i < strlen(s); i++) {
		while (!(R8_UART1_LSR & RB_LSR_TX_FIFO_EMP)) {}
		R8_UART1_THR = s[i];
	}
}

const char hexlut[16] = "0123456789abcdef";

void write_hex(uint32_t x) {
	for (int i = 0; i < 8; i++) {
		char nibble = (x >> ((7 - i) * 4)) & 0xf;
		while (!(R8_UART1_LSR & RB_LSR_TX_FIFO_EMP)) {}
		R8_UART1_THR = hexlut[nibble];
	}
}

uint32_t read_str(char *out, uint32_t sz) {
	uint32_t i;
	for (i = 0; i < sz - 1; i++) {
		while (!(R8_UART1_LSR & RB_LSR_DATA_RDY)) {}
		out[i] = R8_UART1_RBR;
		if (out[i] == '\r' || out[i] == '\n')
			break;
	}
	out[i] = 0;
	return i - 1;
}


uint32_t test_tx_buf[512];
uint32_t test_rx_buf[512];


#define RADIO_DMA_REG(offs) (*(volatile uint32_t*)(0x4000c000 + (offs)))
#define R32_DMA0_CTRL_CFG	RADIO_DMA_REG(0x08)
#define R32_DMA2_CTRL_CFG	RADIO_DMA_REG(0x0c)
#define R32_DMA0_TX_SRC		RADIO_DMA_REG(0x10)
#define R32_DMA2_TX_SRC		RADIO_DMA_REG(0x14)
#define R32_DMA0_RX_DST		RADIO_DMA_REG(0x18)
#define R32_DMA2_RX_DST		RADIO_DMA_REG(0x1c)


#define BB_REG(offs) (*(volatile uint32_t*)(0x4000c100 + (offs)))
#define R32_BB_CTRL_CFG 	BB_REG(0x00)
#define R32_BB_TXCRC_INIT 	BB_REG(0x04)
#define R32_BB_TXACCS_ADDR 	BB_REG(0x08)
#define R32_BB_RXCRC_INIT 	BB_REG(0x0C)
#define R32_BB_RXACCS_ADDR 	BB_REG(0x10)
#define R32_BB_CTRL_TX 		BB_REG(0x2c)
#define R32_BB_RSSI_ST		BB_REG(0x30)
#define R32_BB_INT_EN		BB_REG(0x34)
#define R32_BB_INT_ST		BB_REG(0x38)


#define LLE_REG(offs) (*(volatile uint32_t*)(0x4000c200 + (offs)))
#define R32_LLE_CTRL_CMD	LLE_REG(0x00)
#define R32_LLE_CTRL_CFG	LLE_REG(0x04)
#define R32_LLE_STATUS		LLE_REG(0x08)
#define R32_LLE_INT_EN		LLE_REG(0x0c)
#define R32_LLE_CTRL_MOD	LLE_REG(0x50)


#define RFEND_REG(offs) (*(volatile uint32_t*)(0x4000d000 + (offs)))


void LLE_DevInit() {
	write_str("LLE_DevInit...\r\n");
	LLE_REG(0x14) = 200;
	LLE_REG(0x1c) = 16;
	LLE_REG(0x24) = 200;
	LLE_REG(0x2c) = 16;
	LLE_REG(0x34) = 200;
	LLE_REG(0x3c) = 16;
	LLE_REG(0x44) = 200;
	LLE_REG(0x4c) = 16;
	LLE_REG(0x58) = 232;
	R32_LLE_CTRL_CFG = 0x1f;
	R32_LLE_INT_EN = 0x31f;
}

void RFEND_Reset() {
	write_str("RFEND_Reset...\r\n");

	RFEND_REG(0xc) |= (1 << 12);
	for (int i = 0; i < 20; i++) {
		asm volatile("nop");
	}
	RFEND_REG(0xc) &= ~(1 << 12);
	for (int i = 0; i < 20; i++) {
		asm volatile("nop");
	}
	RFEND_REG(0xc) |= (1 << 12);
	for (int i = 0; i < 20; i++) {
		asm volatile("nop");
	}
	
	RFEND_REG(0xc) |= (1 << 0);
	for (int i = 0; i < 20; i++) {
		asm volatile("nop");
	}
	RFEND_REG(0xc) &= ~(1 << 0);
	for (int i = 0; i < 20; i++) {
		asm volatile("nop");
	}
	RFEND_REG(0xc) |= (1 << 0);
	for (int i = 0; i < 20; i++) {
		asm volatile("nop");
	}
	
	RFEND_REG(0xc) |= (1 << 8);
	for (int i = 0; i < 20; i++) {
		asm volatile("nop");
	}
	RFEND_REG(0xc) &= ~(1 << 8);
	for (int i = 0; i < 20; i++) {
		asm volatile("nop");
	}
	RFEND_REG(0xc) |= (1 << 8);
	for (int i = 0; i < 20; i++) {
		asm volatile("nop");
	}
}

void RFEND_DevInit() {
	write_str("RFEND_DevInit...\r\n");
	RFEND_Reset();

	RFEND_REG(0x28) = 0x480;

	RFEND_REG(0x48) = (RFEND_REG(0x48) & 0x8fffffff) | 0x60000000;
	RFEND_REG(0x48) = (RFEND_REG(0x48) & 0xffff0fff) | 0x6000;
	RFEND_REG(0x48) |= 0x80000000;

	RFEND_REG(0x4c) &= 0xfffff8ff;

	RFEND_REG(0x54) |= 0x80;

	RFEND_REG(0x3c) = (RFEND_REG(0x3c) & 0xffff0fff) | 0x8000;
	RFEND_REG(0x3c) = (RFEND_REG(0x3c) & 0xf8ffffff) | 0x2000000;
	RFEND_REG(0x3c) = (RFEND_REG(0x3c) & 0x1fffffff) | 0x40000000;

	RFEND_REG(0x5c) |= 0x70000000;
	RFEND_REG(0x5c) |= 0x700000;
	RFEND_REG(0x5c) |= 0x70000;
	RFEND_REG(0x5c) &= 0xf8ffffff;

	RFEND_REG(0x2c) = (RFEND_REG(0x2c) & 0xff8fffff) | 0x200000;
	RFEND_REG(0x2c) = (RFEND_REG(0x2c) & 0xf8ffffff) | 0x5000000;
	RFEND_REG(0x2c) |= 0x3000;
	RFEND_REG(0x2c) |= 0x30000;

	RFEND_REG(0x30) &= 0xfffffff0;
	RFEND_REG(0x30) &= 0xffffff0f;
	RFEND_REG(0x30) = (RFEND_REG(0x30) & 0x8fffffff) | 0x40000000;
}

void BB_DevInit() {
	write_str("BB_DevInit...\r\n");
	R32_BB_CTRL_CFG = 0x10822a5;
	R32_BB_CTRL_TX = 0x14e78;
}

void RFEND_WaitTune() {
	write_str("RFEND_WaitTune...\r\n");
	for (int i = 0; i < 150000; i++) {
		uint32_t reg = RFEND_REG(0x90);
		write_hex(reg);
		write_str("\r\n");
		if ((reg & 0x06000000) == 0x06000000)
			break;
	}
}

void RFEND_TXCtune() {
	write_str("RFEND_TXCtune...\r\n");
	RFEND_REG(0x04) &= ~0x100;
	RFEND_REG(0x28) &= ~0x1000;
	RFEND_REG(0x2c) &= ~0x10;
	RFEND_REG(0x08) |= 0x20000;
	RFEND_REG(0x04) |= 0x10;

	RFEND_REG(0x04) &= ~1;
	RFEND_REG(0x38) = (RFEND_REG(0x38) & 0xfffe00ff) | 0xc000;
	RFEND_REG(0x04) |= 1;
	RFEND_WaitTune();
	int nCO2401_related = RFEND_REG(0x90);
	int nGA2401_related = RFEND_REG(0x94);

	RFEND_REG(0x04) &= ~1;
	RFEND_REG(0x38) = (RFEND_REG(0x38) & 0xfffe00ff) | 0xe700;
	RFEND_REG(0x04) |= 1;
	RFEND_WaitTune();
	int nCO2480_related = RFEND_REG(0x90);
	int nGA2480_related = RFEND_REG(0x94);

	RFEND_REG(0x04) &= ~1;
	RFEND_REG(0x38) = (RFEND_REG(0x38) & 0xfffe00ff) | 0xd300;
	RFEND_REG(0x04) |= 1;
	RFEND_WaitTune();
	int nCO2440_related = RFEND_REG(0x90);
	int nGA2440_related = RFEND_REG(0x94);

	write_str("tuning results:\r\n");
	write_hex(nCO2401_related);
	write_str("\r\n");
	write_hex(nGA2401_related);
	write_str("\r\n");
	write_hex(nCO2480_related);
	write_str("\r\n");
	write_hex(nGA2480_related);
	write_str("\r\n");
	write_hex(nCO2440_related);
	write_str("\r\n");
	write_hex(nGA2440_related);
	write_str("\r\n");

	int nCO2401 = nCO2401_related & 0x3f;
	int nCO2440 = nCO2440_related & 0x3f;
	int nCO2480 = nCO2480_related & 0x3f;
	int nGA2401 = (nGA2401_related >> 10) & 0x7f;
	int nGA2440 = (nGA2440_related >> 10) & 0x7f;
	int nGA2480 = (nGA2480_related >> 10) & 0x7f;

	int co_diff_2401_2440 = nCO2401 - nCO2440;
	int co_diff_2440_2480 = nCO2440 - nCO2480;

	RFEND_REG(0xa0) =
		(((((co_diff_2401_2440) * 39) / 39) & 0xf) << 0) |
		(((((co_diff_2401_2440) * 38) / 39) & 0xf) << 4) |
		(((((co_diff_2401_2440) * 37) / 39) & 0xf) << 8) |
		(((((co_diff_2401_2440) * 36) / 39) & 0xf) << 12) |
		(((((co_diff_2401_2440) * 35) / 39) & 0xf) << 16) |
		(((((co_diff_2401_2440) * 34) / 39) & 0xf) << 20) |
		(((((co_diff_2401_2440) * 33) / 39) & 0xf) << 24) |
		(((((co_diff_2401_2440) * 32) / 39) & 0xf) << 28);
	RFEND_REG(0xa4) =
		(((((co_diff_2401_2440) * 31) / 39) & 0xf) << 0) |
		(((((co_diff_2401_2440) * 30) / 39) & 0xf) << 4) |
		(((((co_diff_2401_2440) * 29) / 39) & 0xf) << 8) |
		(((((co_diff_2401_2440) * 28) / 39) & 0xf) << 12) |
		(((((co_diff_2401_2440) * 27) / 39) & 0xf) << 16) |
		(((((co_diff_2401_2440) * 26) / 39) & 0xf) << 20) |
		(((((co_diff_2401_2440) * 25) / 39) & 0xf) << 24) |
		(((((co_diff_2401_2440) * 24) / 39) & 0xf) << 28);
	RFEND_REG(0xa8) =
		(((((co_diff_2401_2440) * 23) / 39) & 0xf) << 0) |
		(((((co_diff_2401_2440) * 22) / 39) & 0xf) << 4) |
		(((((co_diff_2401_2440) * 21) / 39) & 0xf) << 8) |
		(((((co_diff_2401_2440) * 20) / 39) & 0xf) << 12) |
		(((((co_diff_2401_2440) * 19) / 39) & 0xf) << 16) |
		(((((co_diff_2401_2440) * 18) / 39) & 0xf) << 20) |
		(((((co_diff_2401_2440) * 17) / 39) & 0xf) << 24) |
		(((((co_diff_2401_2440) * 16) / 39) & 0xf) << 28);
	RFEND_REG(0xac) =
		(((((co_diff_2401_2440) * 15) / 39) & 0xf) << 0) |
		(((((co_diff_2401_2440) * 14) / 39) & 0xf) << 4) |
		(((((co_diff_2401_2440) * 13) / 39) & 0xf) << 8) |
		(((((co_diff_2401_2440) * 12) / 39) & 0xf) << 12) |
		(((((co_diff_2401_2440) * 11) / 39) & 0xf) << 16) |
		(((((co_diff_2401_2440) * 10) / 39) & 0xf) << 20) |
		(((((co_diff_2401_2440) *  9) / 39) & 0xf) << 24) |
		(((((co_diff_2401_2440) *  8) / 39) & 0xf) << 28);
	RFEND_REG(0xb0) =
		(((((co_diff_2401_2440) *  7) / 39) & 0xf) << 0) |
		(((((co_diff_2401_2440) *  6) / 39) & 0xf) << 4) |
		(((((co_diff_2401_2440) *  5) / 39) & 0xf) << 8) |
		(((((co_diff_2401_2440) *  4) / 39) & 0xf) << 12) |
		(((((co_diff_2401_2440) *  3) / 39) & 0xf) << 16) |
		(((((co_diff_2401_2440) *  2) / 39) & 0xf) << 20) |
		(((((co_diff_2401_2440) *  1) / 39) & 0xf) << 24) |
		(((((co_diff_2401_2440) *  0) / 39) & 0xf) << 28);

	RFEND_REG(0xb4) =
		(((((co_diff_2440_2480) *  1) / 40) & 0xf) << 0) |
		(((((co_diff_2440_2480) *  2) / 40) & 0xf) << 4) |
		(((((co_diff_2440_2480) *  3) / 40) & 0xf) << 8) |
		(((((co_diff_2440_2480) *  4) / 40) & 0xf) << 12) |
		(((((co_diff_2440_2480) *  5) / 40) & 0xf) << 16) |
		(((((co_diff_2440_2480) *  6) / 40) & 0xf) << 20) |
		(((((co_diff_2440_2480) *  7) / 40) & 0xf) << 24) |
		(((((co_diff_2440_2480) *  8) / 40) & 0xf) << 28);
	RFEND_REG(0xb8) =
		(((((co_diff_2440_2480) *  9) / 40) & 0xf) << 0) |
		(((((co_diff_2440_2480) * 10) / 40) & 0xf) << 4) |
		(((((co_diff_2440_2480) * 11) / 40) & 0xf) << 8) |
		(((((co_diff_2440_2480) * 12) / 40) & 0xf) << 12) |
		(((((co_diff_2440_2480) * 13) / 40) & 0xf) << 16) |
		(((((co_diff_2440_2480) * 14) / 40) & 0xf) << 20) |
		(((((co_diff_2440_2480) * 15) / 40) & 0xf) << 24) |
		(((((co_diff_2440_2480) * 16) / 40) & 0xf) << 28);
	RFEND_REG(0xbc) =
		(((((co_diff_2440_2480) * 17) / 40) & 0xf) << 0) |
		(((((co_diff_2440_2480) * 18) / 40) & 0xf) << 4) |
		(((((co_diff_2440_2480) * 19) / 40) & 0xf) << 8) |
		(((((co_diff_2440_2480) * 20) / 40) & 0xf) << 12) |
		(((((co_diff_2440_2480) * 21) / 40) & 0xf) << 16) |
		(((((co_diff_2440_2480) * 22) / 40) & 0xf) << 20) |
		(((((co_diff_2440_2480) * 23) / 40) & 0xf) << 24) |
		(((((co_diff_2440_2480) * 24) / 40) & 0xf) << 28);
	RFEND_REG(0xc0) =
		(((((co_diff_2440_2480) * 25) / 40) & 0xf) << 0) |
		(((((co_diff_2440_2480) * 26) / 40) & 0xf) << 4) |
		(((((co_diff_2440_2480) * 27) / 40) & 0xf) << 8) |
		(((((co_diff_2440_2480) * 28) / 40) & 0xf) << 12) |
		(((((co_diff_2440_2480) * 29) / 40) & 0xf) << 16) |
		(((((co_diff_2440_2480) * 30) / 40) & 0xf) << 20) |
		(((((co_diff_2440_2480) * 31) / 40) & 0xf) << 24) |
		(((((co_diff_2440_2480) * 32) / 40) & 0xf) << 28);
	RFEND_REG(0xc4) =
		(((((co_diff_2440_2480) * 33) / 40) & 0xf) << 0) |
		(((((co_diff_2440_2480) * 34) / 40) & 0xf) << 4) |
		(((((co_diff_2440_2480) * 35) / 40) & 0xf) << 8) |
		(((((co_diff_2440_2480) * 36) / 40) & 0xf) << 12) |
		(((((co_diff_2440_2480) * 37) / 40) & 0xf) << 16) |
		(((((co_diff_2440_2480) * 38) / 40) & 0xf) << 20) |
		(((((co_diff_2440_2480) * 39) / 40) & 0xf) << 24) |
		(((((co_diff_2440_2480) * 40) / 40) & 0xf) << 28);
	RFEND_REG(0xc8) =
		(((((co_diff_2440_2480) * 41) / 40) & 0xf) << 0) |
		(((((co_diff_2440_2480) * 42) / 40) & 0xf) << 4) |
		((((((nGA2440 - nGA2480) * 10) / co_diff_2440_2480) & 0xf) | 0x8) << 8) |
		((((((nGA2440 - nGA2480) * 9) / co_diff_2440_2480) & 0xf) | 0x8) << 12) |
		((((((nGA2440 - nGA2480) * 8) / co_diff_2440_2480) & 0xf) | 0x8) << 16) |
		((((((nGA2440 - nGA2480) * 7) / co_diff_2440_2480) & 0xf) | 0x8) << 20) |
		((((((nGA2440 - nGA2480) * 6) / co_diff_2440_2480) & 0xf) | 0x8) << 24) |
		((((((nGA2440 - nGA2480) * 5) / co_diff_2440_2480) & 0xf) | 0x8) << 28);
	RFEND_REG(0xcc) =
		((((((nGA2440 - nGA2480) * 4) / co_diff_2440_2480) & 0xf) | 0x8) << 0) |
		((((((nGA2440 - nGA2480) * 3) / co_diff_2440_2480) & 0xf) | 0x8) << 4) |
		((((((nGA2440 - nGA2480) * 2) / co_diff_2440_2480) & 0xf) | 0x8) << 8) |
		((((((nGA2440 - nGA2480) * 1) / co_diff_2440_2480) & 0xf) | 0x8) << 12) |
		((((((nGA2401 - nGA2440) * 0) / co_diff_2401_2440) & 0xf)) << 16) |
		((((((nGA2401 - nGA2440) * 1) / co_diff_2401_2440) & 0xf)) << 20) |
		((((((nGA2401 - nGA2440) * 2) / co_diff_2401_2440) & 0xf)) << 24) |
		((((((nGA2401 - nGA2440) * 3) / co_diff_2401_2440) & 0xf)) << 28);
	RFEND_REG(0xd0) =
		((((((nGA2401 - nGA2440) * 4) / co_diff_2401_2440) & 0xf)) << 0) |
		((((((nGA2401 - nGA2440) * 5) / co_diff_2401_2440) & 0xf)) << 4) |
		((((((nGA2401 - nGA2440) * 6) / co_diff_2401_2440) & 0xf)) << 8) |
		((((((nGA2401 - nGA2440) * 7) / co_diff_2401_2440) & 0xf)) << 12) |
		((((((nGA2401 - nGA2440) * 8) / co_diff_2401_2440) & 0xf)) << 16) |
		((((((nGA2401 - nGA2440) * 9) / co_diff_2401_2440) & 0xf)) << 20) |
		((((((nGA2401 - nGA2440) * 10) / co_diff_2401_2440) & 0xf)) << 24);

	RFEND_REG(0x04) &= ~0x10;
	RFEND_REG(0x04) &= ~1;

	RFEND_REG(0x28) |= 0x1000;
	RFEND_REG(0x2c) |= 0x10;

	RFEND_REG(0x38) = (RFEND_REG(0x38) & 0x80ffffc0) | (nCO2440 & 0x3f) | ((nGA2440 & 0x7f) << 24);
}


void RFEND_TXFtune() {
	write_str("RFEND_TXFtune...\r\n");
	RFEND_REG(0x04) |= 0x100;
}


void RFEND_RXFilter() {
	write_str("RFEND_RXFilter...\r\n");
	RFEND_REG(0x50) &= ~0x10000;
	RFEND_REG(0x08) |= 0x200000;
	RFEND_REG(0x0c) |= 0x10;
	RFEND_REG(0x0c) &= ~0x10;
	asm volatile("nop\nnop\nnop\nnop\n");
	RFEND_REG(0x0c) |= 0x10;
	RFEND_REG(0x04) |= 0x1000;
	while (!(RFEND_REG(0x9c) & 0x100)) {}
	asm volatile("nop\nnop\nnop\nnop\n");
	RFEND_REG(0x50) |= 0x10000;
	uint32_t reg_9c = RFEND_REG(0x9c);
	write_hex(reg_9c);
	write_str("\r\n");
	RFEND_REG(0x50) = (RFEND_REG(0x50) & 0xffffffe0) | (reg_9c & 0x1f);
	RFEND_REG(0x08) &= ~0x200000;
}


void RFEND_RXAdc() {
	write_str("RFEND_RXAdc (auto)...\r\n");
	RFEND_REG(0x58) &= ~0x10000;
	RFEND_REG(0x08) |= 0x10000;
	RFEND_REG(0x0c) &= ~0x100;
	RFEND_REG(0x04) &= ~0x10000;
	RFEND_REG(0x0c) |= 0x100;
	RFEND_REG(0x04) |= 0x10000;
}


int main() {
	R32_PA_DIR |= (1 << 9);
	R32_PA_PU |= (1 << 8);

	R8_UART1_IER = RB_IER_RESET;
	R8_UART1_IER = RB_IER_TXD_EN;
	R8_UART1_LCR = 0b11;
	R16_UART1_DL = 35;
	R8_UART1_DIV = 1;

	write_str("\r\nHello world!\r\n");
	R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG1;		
	R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG2;
	R16_CLK_SYS_CFG = RB_CLK_OSC32M_XT | (2<<6) | 0x08;		// 32 MHz, external crystal
	R8_SAFE_ACCESS_SIG = 0;
	write_str("external crystal ok...\r\n");

	R32_DMA0_TX_SRC = (uint32_t)(&test_tx_buf);
	R32_DMA2_TX_SRC = (uint32_t)(&test_tx_buf);
	R32_DMA0_RX_DST = (uint32_t)(&test_rx_buf);
	R32_DMA2_RX_DST = (uint32_t)(&test_rx_buf);
	R32_DMA0_CTRL_CFG |= (1 << 13);
	R32_DMA2_CTRL_CFG |= (1 << 13);

	LLE_DevInit();
	RFEND_DevInit();
	BB_DevInit();
	R32_LLE_CTRL_MOD = 0x5d;
	RFEND_TXCtune();
	RFEND_TXFtune();
	RFEND_RXFilter();
	RFEND_RXAdc();
	R32_LLE_CTRL_MOD = 0;

	NVIC_EnableIRQ(BLEL_IRQn);
	__enable_irq();

	// basic rx:
	// (gdb) set *(int*)0x4000c250 = 0x59
	// (gdb) set *(int*)0x4000c10c = 0x555555
	// (gdb) set *(int*)0x4000c204 = 0x1e
	// (gdb) set *(int*)0x4000c200 = 1

	((char*)(&test_tx_buf))[0] = 0x42;
	((char*)(&test_tx_buf))[1] = 15;

	((char*)(&test_tx_buf))[2] = 0xaa;
	((char*)(&test_tx_buf))[3] = 0xbb;
	((char*)(&test_tx_buf))[4] = 0xcc;
	((char*)(&test_tx_buf))[5] = 0xdd;
	((char*)(&test_tx_buf))[6] = 0xee;
	((char*)(&test_tx_buf))[7] = 0x3f;

	((char*)(&test_tx_buf))[8] = 0x02;
	((char*)(&test_tx_buf))[9] = 0x01;
	((char*)(&test_tx_buf))[10] = 0x04;

	((char*)(&test_tx_buf))[11] = 0x05;
	((char*)(&test_tx_buf))[12] = 0x09;
	((char*)(&test_tx_buf))[13] = 'O';
	((char*)(&test_tx_buf))[14] = 'w';
	((char*)(&test_tx_buf))[15] = 'O';
	((char*)(&test_tx_buf))[16] = '?';

	((char*)(&test_tx_buf))[20] = 0x14;		// tx power, (len + 5) & ~3

	R32_LLE_CTRL_MOD = 0x5a;
	R32_BB_TXCRC_INIT = 0x555555;
	R32_BB_RXCRC_INIT = 0x555555;
	R32_DMA0_CTRL_CFG = (R32_DMA0_CTRL_CFG & ~0x1fff) | 15;
	// R32_LLE_CTRL_CFG &= ~1;

	// while (1) {
	// 	R32_LLE_CTRL_CMD = 2;
	// 	for (int i = 0; i < 40000; i++) {
	// 		asm volatile("nop");
	// 	}
	// }

	R32_LLE_CTRL_CFG |= 1;
	R32_LLE_CTRL_CMD = 2;
	while (1) {}
}


void LLE_IRQHandler() {
	uint32_t status = R32_LLE_STATUS;
	R32_LLE_STATUS = 0;
	write_str("LLE IRQ!\r\n");
	write_hex(status);
	write_str("\r\n");
}
