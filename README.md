# EDA Netlist Analyzer

A high-performance C++ tool for parsing Verilog netlists and performing Static Timing Analysis (STA).

## Key Features
- **Verilog Parsing**: Extract modules, ports, and gate primitives.
- **DAG Graph**: Construct in-memory timing logic graphs.
- **Static Timing Analysis**: Calculate propagation delays and critical path.

## Build & Run
```bash
mkdir -p build && cd build
cmake ..
make
./netlist_analyzer ../tests/simple_and_not.v
```

eda-netlist-analyzer/
├── CMakeLists.txt        # [建構核心] 告訴編譯器如何連結與編譯 C++ 程式碼
├── README.md             # [專案說明] 
├── include/              # [介面定義] 標頭檔 (.hpp / .h)，宣告資料結構與類別
│   ├── Netlist.hpp       #   - 定義電路圖 (DAG)、Gate (Logic Pin) 與 Wire 結構
│   ├── Parser.hpp        #   - 定義 Verilog Netlist 語法解析器介面
│   └── STA.hpp           #   - 定義靜態時序分析 (STA) 引擎介面
├── src/                  # [程式邏輯] 實作檔 (.cpp)，撰寫具體演算法
│   ├── Parser.cpp        #   - 實作 Lexer 與 Verilog 語法解析
│   ├── STA.cpp           #   - 實作 Topological Sort 與 Critical Path 計算
│   └── main.cpp          #   - 主程式進入點 (分析 CLI 參數與串接流程)
└── tests/                # [測試用例] 存放測試用的 Verilog 電路檔
    └── simple_and_not.v  #   - 範例電路檔 (測試 AND / NOT Gate)