# NHÓM 3 - HỆ THỐNG NHÚNG - BÁO CÁO BÀI TẬP TUẦN 3

## A. Nội dung công việc

1. Giao tiếp nối tiếp giữa STM32 với máy tính <sử dụng ngắt>

2. Hiểu và triển khai giao thức I2C

3. Hiểu và triển khai giao thức SPI

## B. Báo cáo chi tiết

### 1. Giao tiếp giữa STM32 với máy tính

> - Cấu hình UART trên stm32f1
> - Viết chương trình gửi chuỗi "Hello from STM32!" tới máy tính
> - Sử dụng phần mềm Turminal để hiển thị chuỗi này
> - Khi gửi chuỗi thì stm32 sẽ phản hồi lại ký tự đó, gửi chuỗi "ON" thì bật đèn, "OFF" thì tắt đèn

**Ý tưởng**

> Cấu hình GPIO cho LED

- LED thường nối vào PC13 trên board STM32F103C8T6 (Blue Pill).

- Cần cấu hình PC13 làm Output Push-Pull.

> Cấu hình USART

- Dùng USART1 (thường chân PA9: TX, PA10: RX trên STM32F103C8T6).

Thông số cơ bản:

- Baudrate: 9600 hoặc 115200 (tùy Terminal chọn).

- 8 bit data, 1 stop bit, no parity.

- Enable cả TX và RX.

> Gửi chuỗi ra UART

- Dùng hàm `USART_SendData(USART1, data)` trong thư viện có sẵn.

- Chờ đến khi USART_FLAG_TXE set thì gửi byte tiếp theo.

- Viết hàm `USART1_SendString("Hello...")`.

> Nhận dữ liệu UART

- Bật USART1 interrupt RXNE để nhận từng ký tự.

Trong hàm `USART1_IRQHandler`, khi có ký tự đến thì:

- Lưu vào buffer.

- Echo lại ký tự (`USART_SendData`).

- Nếu buffer chứa "ON" → bật LED.

- Nếu buffer chứa "OFF" → tắt LED.

- Khi gặp ký tự xuống dòng (\r hoặc \n) → reset buffer để nhận lệnh mới.

**Phần mềm: [UART](src/bt6.c)**

**Demo: https://drive.google.com/file/d/1faWeoPSrIksrtR9MJjU1Ee9PtZsJOY1Q/view?usp=drive_link**

### 2. Hiểu và triển khai giao thức I2C

> Ở bài tập này chúng em sử dụng module I2C RTC DS1307 

**Ý tưởng**

- Cấu hình I2C trên STM32 (Master mode, tốc độ 100kHz).

Ví dụ: Sử dụng I2C1 (PB6: SCL, PB7: SDA).

- Cấu hình UART để truyền dữ liệu đọc được từ DS1307 ra PC.

Ví dụ: USART1 (PA9: TX, PA10: RX).

`main`

Khởi động UART + I2C.

Ghi dữ liệu vào DS1307 (thiết lập giờ ban đầu).

Ghi vào thanh ghi giây (0x00), phút (0x01), giờ (0x02), …

Đọc dữ liệu từ DS1307.

Đọc tuần tự từ địa chỉ 0x00 → lấy giờ, phút, giây.

Chuyển dữ liệu từ BCD → Decimal để dễ đọc.

Gửi chuỗi thời gian qua UART dạng "HH:MM:SS\r\n".

Lặp lại định kỳ (ví dụ mỗi 1 giây đọc thời gian).

**Phần mềm: [I2C](src/bt7.c)**

**Demo: https://drive.google.com/file/d/1S7XCTI8Kz5o4LB6ehnlKlB0JPZsp4KKQ/view?usp=drive_link**

### 3. Hiểu và triển khai giao thức SPI

> Ở bài này chúng em sử dụng module SPI LoRa-02-Think (SX1278)

**Ý tưởng**

> Cấu hình UART

USART1_Init() cấu hình PA9 (TX), PA10 (RX) baudrate 9600.

Có hàm gửi ký tự và chuỗi (USART1_SendString).

Dùng để in log ra terminal (PC).

> Cấu hình SPI

SPI1_Init() cấu hình SPI1 ở chế độ Master:

SCK: PA5, MOSI: PA7, MISO: PA6, NSS: PA4.

CPOL = 0, CPHA = 0 → đúng chuẩn SX1278.

SPI1_Transfer() để gửi/nhận 1 byte (full-duplex).

> Cấu hình LoRa (SX1278)

LoRa_ReadReg() và LoRa_WriteReg() để đọc/ghi thanh ghi.

Trong LoRa_Init() có đọc thanh ghi version (0x42). Nếu trả về 0x12 → chip nhận diện OK.

LoRa_SendAndEcho() ghi dữ liệu vào FIFO TX (reg 0x00), sau đó đọc ngược lại từ FIFO để giả lập echo

**Phần mềm: [SPI](src/bt8.c)**

**Demo: https://drive.google.com/file/d/1Qbw4l8KWWi7EEwaaLQNTurwAbJLulAO5/view?usp=drive_link**