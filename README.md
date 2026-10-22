# 🌟 Smart Light  - Đèn Thông Minh Tự Cân Bằng Ánh Sáng

**Bài tập lớn môn học Hệ thống nhúng (Embedded Systems)**

Hệ thống đèn chiếu sáng thông minh sử dụng vi điều khiển **STM32F103C8T6**, tự động điều chỉnh cường độ sáng dựa trên ánh sáng môi trường, tích hợp cảm biến phát hiện chuyển động và tối ưu hóa trải nghiệm thị giác người dùng bằng các thuật toán xử lý tín hiệu số thời gian thực.

🎥 **Video Demo Hoạt Động :** [ VIDEO DEMO SẢN PHẨM ](https://drive.google.com/drive/folders/1Ui-AjM84c4pKZEXi94avBHPAIBK-4vks?usp=drive_link)

---

## 📌 1. Tính Năng Nổi Bật (Features)
*   **Chế độ Tự Động (Auto):** Kết hợp cảm biến chuyển động PIR và Quang trở (LDR). Đèn chỉ bật khi có người và tự động tăng/giảm % công suất bù trừ tuyến tính theo độ tối của môi trường. Tự động tắt sau 10 giây khi người rời đi.
*   **Chế độ Chỉnh Tay (Manual):** Cho phép người dùng ghi đè hệ thống, điều chỉnh độ sáng từ 0-100% cực kỳ phản hồi qua Biến trở.
*   **Hiển thị trực quan:** Cập nhật % độ sáng theo thời gian thực lên màn hình LED 7 đoạn (Module TM1637).


## 🛠️ 2. Phần Cứng Sử Dụng (Hardware)
1.  **MCU:** STM32F103C8T6 - ARM Cortex-M3.
2.  **Cảm biến:** Quang trở (LDR) + Điện trở 10kΩ, Cảm biến PIR HC-SR501.
3.  **Điều khiển:** Biến trở 10kΩ, Nút nhấn (Push button).
4.  **Công suất & Hiển thị:** MOSFET IRFZ44N, Màn hình LED 7 đoạn TM1637, Thanh 8 bóng LED 5V.

## 🔌 3. Sơ Đồ Chân (Pinout Mapping)

| Chức Năng | Chân STM32 | Cấu hình Ngoại vi (CubeMX) |
| :--- | :--- | :--- |
| **Quang trở (LDR)** | `PA0` | `ADC1_IN0` (DMA Circular) |
| **Biến trở (POT)** | `PA1` | `ADC1_IN1` (DMA Circular) |
| **MOSFET (LED)** | `PA2` | `TIM2_CH3` (PWM 1kHz) |
| **Nút nhấn (Mode)** | `PB0` | `GPIO_EXTI0` (Falling Edge, Pull-up) |
| **Cảm biến PIR** | `PB1` | `GPIO_EXTI1` (Rising/Falling, Pull-down) |
| **LED 7 Đoạn (CLK)** | `PB6` | `GPIO_Output` (High Speed) |
| **LED 7 Đoạn (DIO)** | `PB7` | `GPIO_Output` (High Speed) |

## 🚀 4. Hướng Dẫn Biên Dịch & Nạp Code
1.  **Môi trường:** Keil C (MDK-ARM V5) và STM32CubeMX.
2.  **Cấu trúc thư mục:** Mở file `SmartLight_STM32.ioc` để xem cấu hình phần cứng. Code thực thi nằm trong thư mục `MDK-ARM/SmartLight_STM32.uvprojx`.
3.  **Biên dịch:** Nhấn phím `F7` (Build) trên Keil C.
4.  **Nạp Code:** Kết nối mạch nạp ST-Link V2 và nhấn `F8` (Download) để nạp Firmware xuống vi điều khiển.
