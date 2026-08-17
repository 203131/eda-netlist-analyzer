# EDA Netlist Analyzer

用 C++20 寫的小工具：讀 gate-level 的 Verilog 網表，建成 DAG 電路圖，再用 Kahn's 拓樸排序
＋動態規劃算出 Static Timing Analysis (STA) 的 critical path 跟 max delay。

動機是想練一遍 EDA 工具鏈裡最基礎的一段 pipeline：`文字網表 -> Parser -> Graph -> Timing Engine -> 報告`，
這其實是 PrimeTime 這類商用 STA 工具背後最核心邏輯的簡化版雛形。

## 目前支援什麼

- 解析 `module` / `input` / `output` / `wire` 宣告，以及 `AND` / `OR` / `NOT` / `NAND` / `NOR` 這幾種 gate primitive
- 把解析結果建成 DAG：每個 gate 節點以它的輸出腳位命名（細節見下面「架構」段）
- Kahn's algorithm 做拓樸排序，順便當作環路偵測——電路裡如果有 combinational loop 會直接報錯，不會裝作沒事
- 沿拓樸序做一次線性 DP 算 arrival time，取全圖最大值回溯出 critical path

## 目前不支援 / 已知限制

先講清楚邊界，免得看起來像完整的商用 STA 工具：

- 沒有 `assign`、沒有 bus/vector（`[7:0]` 那種寫法），只吃最陽春的 gate-level netlist
- Gate delay 是寫死在 `Parser.cpp` 裡的一張表（每種閘型一個固定值），不是從真實的 Liberty (`.lib`)
  或 SDF 檔案讀出來的，純粹是為了讓整條 STA pipeline 能跑起來的簡化假設
- 只算組合邏輯的 critical path，沒有 clock、沒有 setup/hold check，也沒有 multi-cycle path
- Parser 是逐行 token 掃描，不是正規的 lexer/parser，沒有處理 Verilog 語法的所有變化

## 架構

```
Verilog 檔 (.v)
     |  NetlistParser::parse()
     v
Module 名稱 / Ports / Gate 清單          <- Parser.hpp / Parser.cpp
     |  NetlistParser::build_graph()
     v
NetlistGraph (DAG)                       <- Netlist.hpp / Netlist.cpp
     |  STAEngine::run_sta()
     v
TimingResult { max_delay, critical_path } <- STA.hpp / STA.cpp
```

**Parser**　`NetlistParser::parse()` 一行一行掃描檔案，遇到關鍵字就分流處理，把 module 名稱、
`input` / `output` / `wire` 清單，以及每個 gate（type、instance name、output pin、input pins）收集起來。
這一步只負責「讀懂檔案」，不會去碰 `NetlistGraph`——轉圖是 `build_graph()` 另外做的事，兩件事故意拆開，
方便之後如果要換一個 parser 前端也不用動 graph 那層。

**Netlist（DAG）**　`NetlistGraph` 用 `unordered_map<string, Node>` 存節點，`Node` 裡有 `fanin` /
`fanout` 兩個 vector 記邊。比較值得說明的一個設計決定：**gate 節點的 key 用它的輸出腳位名稱，不是
instance name**。因為在 Verilog 裡，下游 gate 的輸入寫的是訊號名稱（例如 `w1`），不是驅動它的 gate
是誰（`g1`），這樣 `build_graph()` 建邊時可以直接拿訊號名稱去查表，不用另外維護一份「訊號到驅動 gate」
的對照表。

**STA**　`run_topological_sort()` 是標準的 Kahn's algorithm：算每個節點的入度、把入度 0 的塞進
queue，pop 出來後把它的 fanout 入度各減一，減到 0 就再塞回 queue。如果跑完 `topo_order` 的節點數
跟圖的節點數對不上，代表有環，直接回傳 false（`run_sta()` 這時回傳一個空的 `TimingResult`）。

`run_sta()` 沿拓樸序做一次線性 DP：`AT(v) = delay(v) + max(AT(u) for u in fanin(v))`，同時記一張
`prev_node` 表方便回溯 critical path。這裡踩過一個小坑：如果一個 gate 的所有 fanin 都是 primary
input（AT 剛好都是 0.0），原本用嚴格 `>` 比較會導致一個 parent 都選不到，回溯到這裡就斷掉，
漏掉最前面的 input。後來改成「先把第一個 fanin 收作候選，之後才用 `>` 篩選出真正較大的」解決。

## Build & Run

```bash
cmake -B build -S .
cmake --build build
./build/netlist_analyzer tests/simple_and_not.v
```

### 範例輸出

`tests/simple_and_not.v`——最小電路，AND 接 NOT：

```
$ ./build/netlist_analyzer tests/simple_and_not.v
========================================
 Netlist Summary: simple_circuit
========================================
[Inputs]  : a b
[Outputs] : y
[Wires]   : w1
[Gates]   : 2 instances found:
  - AND (g1) -> Out: w1 | In: a b
  - NOT (g2) -> Out: y | In: w1
========================================

========================================
 Static Timing Analysis (STA) Result
========================================
[Max Delay]      : 0.3 ns
[Critical Path]  : a -> w1 -> y
========================================
```

`tests/reconvergent_path.v`——5 種閘型 + 一條匯聚路徑（`w1` 同時餵給 `w3` 和 `z`），用來確認
STA 真的有比較過所有分支、挑出延遲最長的那條，而不是照 topological order 隨便選：

```
$ ./build/netlist_analyzer tests/reconvergent_path.v
========================================
 Netlist Summary: test_circuit
========================================
[Inputs]  : a b c
[Outputs] : y z
[Wires]   : w1 w2 w3
[Gates]   : 5 instances found:
  - AND (g1) -> Out: w1 | In: a b
  - NOT (g2) -> Out: w2 | In: c
  - OR (g3) -> Out: w3 | In: w1 w2
  - NAND (g4) -> Out: y | In: w3 a
  - NOR (g5) -> Out: z | In: w1 c
========================================

========================================
 Static Timing Analysis (STA) Result
========================================
[Max Delay]      : 0.55 ns
[Critical Path]  : a -> w1 -> w3 -> y
========================================
```

`z` 那條路徑只有 0.35 ns，比 `y` 這條的 0.55 ns 短，所以不會出現在 critical path 裡——這也是這個
測資存在的目的，如果 STA 邏輯選錯了會直接被這個測資抓出來。

## 專案結構

```
eda-netlist-analyzer/
├── CMakeLists.txt          # C++20, -Wall -Wextra, 自動 glob src/*.cpp
├── include/
│   ├── Netlist.hpp         # Node / NodeType / NetlistGraph
│   ├── Parser.hpp          # Gate / NetlistParser
│   └── STA.hpp             # TimingResult / STAEngine
├── src/
│   ├── Netlist.cpp
│   ├── Parser.cpp
│   ├── STA.cpp
│   └── main.cpp             # 串起 Parser -> NetlistGraph -> STAEngine，印出結果
└── tests/
    ├── simple_and_not.v     # 最小電路：AND + NOT
    └── reconvergent_path.v  # 5 種閘型 + 匯聚路徑，驗證 critical path 選路邏輯
```

## 之後想做的

- Gate delay table 改成真的從 Liberty (`.lib`) 檔讀
- 加 `assign` 語句支援
- 補 setup/hold timing check（目前只有純組合邏輯的 max delay）
