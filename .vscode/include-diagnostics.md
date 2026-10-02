# ESP32 项目 include 诊断与修复

## 原因

这是 Arduino 草图工程。安装 C/C++、CMake、PlatformIO 插件不会自动给普通草图配置 Arduino SDK。项目原先没有 c_cpp_properties.json、编译数据库或 platformio.ini，因此 VS Code 不知道 ESP32 SDK、Arduino 库、编译器和开发板宏定义的位置。

## 已修复

- 使用已有 Arduino IDE 和 ESP32 Arduino Core 3.3.11，无需再安装插件。
- 沿用该草图原有 ESP32-S3、16 MB Flash、OPI PSRAM、USB CDC 开发板参数。
- 生成真实编译数据库，展开 GCC 响应文件和 SDK 相对 include 路径，并把 Arduino 构建目录中的源码路径映射回项目源码。
- 配置 ESP32 编译器、头文件路径、宏定义，以及 .ino 文件的 C++ 语言关联。
- 生成路径配置保留在本机，不提交编译产物。

## 验证结果

配置生效后曾在 camera.h 中确认 VS Code 显示“没有问题”和“IntelliSense: 就绪”，使用“ESP32-S3 Arduino”配置。

完整编译未通过。此次编译报告了以下源码错误：

- microTask.cpp:22：microTaskHandle 未声明。
- microTask.cpp:38（以及第 8 行宏的使用处）：SAMPLE_RATE 未声明。
- microTask.cpp:66 及后续位置：decodedLength 未声明。
- httpTask.cpp:243：mbedtls_base64_decode 未声明，缺少相应函数声明；函数声明所在头文件是 mbedtls/base64.h。

这些是实际源码问题。按用户要求未修改源码。用户正进行中的修改尚未重新验证，此处记录的是本次完整编译时的结果。

## 后续使用

- VS Code 菜单“终端 → 运行任务 → Arduino: Refresh IntelliSense”：新增库或切换开发板配置后刷新。
- Ctrl+Shift+B：运行 Arduino: Build ESP32-S3，仅编译。
- 开发板参数和 Arduino CLI 路径在 .vscode/arduino-board.json 中维护。
- 首次克隆到其他电脑时先调整上述路径，再运行刷新任务生成本机配置。
