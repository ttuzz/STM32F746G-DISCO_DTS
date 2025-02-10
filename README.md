# STM32F746G-DISCO Data Transfer Speed (DTS) Benchmark

## ✅ QSPI Performance Test (BSP QSPI N25Q128A)

- **Flash Size:** 16,777,216 bytes (16MB)
- **Erase Sector Size:** 4,096 bytes (4KB)
- **Erase Sectors Number:** 4,096
- **Program Page Size:** 256 bytes
- **Program Pages Number:** 65,536

### Erase Test
- **Total Erase Time:** 123,251 ms (~123.25 sec)

### 64KB Block Test
- **Erase Time:** 0.27 sec
- **Write Time:** 0.193 sec
  - **Speed:** 331.61 KB/s
- **Read Time:** 0.027 sec
  - **Speed:** 2370.37 KB/s
- ✅ **Test Completed Successfully**: Data written and verified correctly!

---
## ✅ SDRAM Speed Test

### **6MB Write Test**
| Data Type  | Time (ms) | Speed (MB/s) |
|------------|----------|--------------|
| `uint8_t`  | 314 ms   | 19.11 MB/s   |
| `uint16_t` | 158 ms   | 37.97 MB/s   |
| `uint32_t` | 86 ms    | 69.77 MB/s   |
| `uint64_t` | 52 ms    | 115.38 MB/s  |

### **6MB Read Test**
| Data Type  | Time (ms) | Speed (MB/s) |
|------------|----------|--------------|
| `uint8_t`  | 460 ms   | 13.04 MB/s   |
| `uint16_t` | 240 ms   | 25.00 MB/s   |
| `uint32_t` | 117 ms   | 51.28 MB/s   |
| `uint64_t` | 74 ms    | 81.08 MB/s   |

---
## ✅ SD Card Speed Test (BSP FatFs SDMMC WideBus DMA)

### **4-bit WideBus, Fat32, DMA, 50MHz**
| Buffer Size | Write Speed (KB/s) |
|------------|--------------------|
| 512B       | 77.00 KB/s         |
| 1KB        | 951.26 KB/s        |
| 2KB        | 1882.82 KB/s       |
| 4KB        | 3494.82 KB/s       |
| 8KB        | 5688.95 KB/s       |
| 16KB       | 6243.90 KB/s       |
| 32KB       | 6249.35 KB/s       |
| 64KB       | ❌ **Error: Failed to open/create file!** |
| 56KB       | 9524.16 KB/s       |


### **4-bit WideBus (50MHz), 20MB File Test**
- **Format Time:** 1284 ms
- **Write Time:** 2995 ms
  - **Speed:** 6838.06 KB/s
- **Read Time:** 2079 ms
  - **Speed:** 9850.89 KB/s

### **1-bit WideBus (50MHz), 20MB File Test**
- **Format Time:** 2029 ms
- **Write Time:** 8009 ms
  - **Speed:** 2557.12 KB/s
- **Read Time:** 7178 ms
  - **Speed:** 2853.16 KB/s

### **4-bit WideBus, Fat32, DMA, 100MHz**
- **Format Time:** 1249 ms
- **Write Time:** 2292 ms
  - **Speed:** 8935.43 KB/s
- **Read Time:** 1283 ms
  - **Speed:** 15962.59 KB/s

### **4-bit WideBus, Fat32, DMA, 150MHz**
- **Format Time:** 1248 ms
- **Write Time:** 2044 ms
  - **Speed:** 10019.57 KB/s
- **Read Time:** 1168 ms
  - **Speed:** 17534.25 KB/s

---
## ✅ USB HS & Additional Notes
- **QSPI sectors require erasing before writing, causing slow performance.**
- **USB media buffer is crucial for speed optimization.**
- **SDRAM operates at near drag-and-drop speed.**
- **Encountered issues with SDMMC in 4-bit WideBus mode, but achieved stable operation in 1-bit mode.**
- **Further optimizations and testing are required.** 🚀
