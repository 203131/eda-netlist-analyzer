# ==============================================================================
# CMake Build Configuration for EDA Netlist Analyzer
# ==============================================================================

# 1. 指定最低要求的 CMake 版本（確保相容性）
cmake_minimum_required(VERSION 3.15)

# 2. 定義專案名稱、版本與程式語言 (CXX 代表 C++)
project(NetlistAnalyzer VERSION 1.0 LANGUAGES CXX)

# 3. 指定 C++ 標準為 C++20（提供高效率現代圖論容器與語法支援）
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 強制編譯器必須支援此標準
set(CMAKE_CXX_EXTENSIONS OFF)         # 關閉編譯器特定擴充，確保程式碼跨平台

# 4. 指定標頭檔 (.hpp / .h) 搜尋目錄
# 包含 include/（專門放標頭檔）與 src/（放實作檔）
include_directories(include src)

# 5. 自動搜尋並收集 src/ 目錄底下所有的 C++ 原始碼檔 (.cpp)
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 6. 定義最終編譯產出的可執行檔名稱 (netlist_analyzer)
add_executable(netlist_analyzer ${SOURCES})

# 7. 開啟編譯器嚴格警告與 O2 效能優化 (幫助在編譯階段提前抓出潛在 Bug)
target_compile_options(netlist_analyzer PRIVATE -Wall -Wextra)