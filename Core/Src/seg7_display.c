#include "seg7_display.h"

// B?ng mã HEX hi?n th? các s? t? 0 d?n 9 (Dùng cho LED 7 do?n Anode/Cathode chung c?a TM1637)
static const uint8_t DIGIT_MAP[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

// Hàm t?o tr? siêu nh? (Microsecond) b?ng vòng l?p tiêu t?n xung nh?p CPU (Dành cho Clock 72MHz)
static void Seg7_DelayUs(uint32_t us) {
    volatile uint32_t count = us * 18; 
    while (count--) {
        __NOP(); // No Operation: L?nh tr?ng d? d?t th?i gian
    }
}

// B?t d?u khung truy?n d? li?u
static void TM1637_Start(void) {
    HAL_GPIO_WritePin(SEG7_CLK_PORT, SEG7_CLK_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SEG7_DIO_PORT, SEG7_DIO_PIN, GPIO_PIN_SET);
    Seg7_DelayUs(5);
    HAL_GPIO_WritePin(SEG7_DIO_PORT, SEG7_DIO_PIN, GPIO_PIN_RESET);
}

// K?t thúc khung truy?n d? li?u
static void TM1637_Stop(void) {
    HAL_GPIO_WritePin(SEG7_CLK_PORT, SEG7_CLK_PIN, GPIO_PIN_RESET);
    Seg7_DelayUs(5);
    HAL_GPIO_WritePin(SEG7_DIO_PORT, SEG7_DIO_PIN, GPIO_PIN_RESET);
    Seg7_DelayUs(5);
    HAL_GPIO_WritePin(SEG7_CLK_PORT, SEG7_CLK_PIN, GPIO_PIN_SET);
    Seg7_DelayUs(5);
    HAL_GPIO_WritePin(SEG7_DIO_PORT, SEG7_DIO_PIN, GPIO_PIN_SET);
}

// Ghi 1 Byte d? li?u xu?ng chip TM1637
static void TM1637_WriteByte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(SEG7_CLK_PORT, SEG7_CLK_PIN, GPIO_PIN_RESET);
        if (byte & 0x01) {
            HAL_GPIO_WritePin(SEG7_DIO_PORT, SEG7_DIO_PIN, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(SEG7_DIO_PORT, SEG7_DIO_PIN, GPIO_PIN_RESET);
        }
        Seg7_DelayUs(5);
        byte >>= 1;
        HAL_GPIO_WritePin(SEG7_CLK_PORT, SEG7_CLK_PIN, GPIO_PIN_SET);
        Seg7_DelayUs(5);
    }
    // Ch? xung ACK t? chip TM1637
    HAL_GPIO_WritePin(SEG7_CLK_PORT, SEG7_CLK_PIN, GPIO_PIN_RESET);
    Seg7_DelayUs(5);
    HAL_GPIO_WritePin(SEG7_CLK_PORT, SEG7_CLK_PIN, GPIO_PIN_SET);
    Seg7_DelayUs(5);
    HAL_GPIO_WritePin(SEG7_CLK_PORT, SEG7_CLK_PIN, GPIO_PIN_RESET);
}

void Seg7_Init(void) {
    TM1637_Start();
    // G?i mã 0x8A d? b?t màn hình LED 7 do?n và d?t d? sáng ? m?c trung bình
    // (D?i mã ch?nh d? sáng c?a TM1637: 0x88 là t?i nh?t, 0x8F là sáng nh?t)
    TM1637_WriteByte(0x8A); 
    TM1637_Stop();
    
    // M?c d?nh lúc v?a c?p ngu?n hi?n s? 0
    Seg7_DisplayPercent(0); 
}

void Seg7_DisplayPercent(uint8_t percent) {
    if (percent > 100) percent = 100;

    // B? nh? t?m cho 4 ô LED trên màn hình (0x00 là t?t ô dó)
    uint8_t digits[4] = {0x00, 0x00, 0x00, 0x00}; 

    // Thu?t toán tách các ch? s? (Hàng tram, hàng ch?c, hàng don v?)
    if (percent == 100) {
        digits[1] = DIGIT_MAP[1]; // Ô s? 2: hi?n th? s? 1
        digits[2] = DIGIT_MAP[0]; // Ô s? 3: hi?n th? s? 0
        digits[3] = DIGIT_MAP[0]; // Ô s? 4: hi?n th? s? 0
    } else if (percent >= 10) {
        digits[2] = DIGIT_MAP[percent / 10]; // C?t l?y hàng ch?c
        digits[3] = DIGIT_MAP[percent % 10]; // C?t l?y hàng don v?
    } else {
        digits[3] = DIGIT_MAP[percent];      // Ch? hi?n hàng don v?
    }

    // L?nh 0x40: C?u hình IC t? d?ng d?ch d?a ch? ô nh?
    TM1637_Start();
    TM1637_WriteByte(0x40);
    TM1637_Stop();

    // L?nh 0xC0: Ð?nh v? con tr? vào ô LED ngoài cùng bên trái (Ô s? 1)
    TM1637_Start();
    TM1637_WriteByte(0xC0);
    
    // Ð? m?ng 4 ch? s? vào 4 ô LED liên ti?p
    for (uint8_t i = 0; i < 4; i++) {
        TM1637_WriteByte(digits[i]);
    }
    TM1637_Stop();
}