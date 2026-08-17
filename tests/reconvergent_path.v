// 這顆電路故意做了一個匯聚路徑 (reconvergent path)：w1 同時餵給 w3 和 z 兩條線，
// y 那邊經過 4 級閘、z 那邊只經過 2 級，用來檢查 STAEngine 是不是真的比較過
// 所有路徑才選出最長的那條，而不是照 topological order 隨便選一條就當 critical path。
//
// 手算比對（delay: NOT=0.10, NAND=0.15, NOR=0.15, AND=0.20, OR=0.20 ns）：
//   w1 = AND(a, b)     -> AT 0.20
//   w2 = NOT(c)        -> AT 0.10
//   w3 = OR(w1, w2)    -> AT max(0.20, 0.10) + 0.20 = 0.40
//   y  = NAND(w3, a)   -> AT max(0.40, 0.00) + 0.15 = 0.55  <- 全電路最大值
//   z  = NOR(w1, c)    -> AT max(0.20, 0.00) + 0.15 = 0.35
//
// 預期：Max Delay = 0.55 ns，Critical Path = a -> w1 -> w3 -> y（z 這條比較短，不會被選到）
module test_circuit (
    input a, b, c,
    output y, z
);
    wire w1, w2, w3;

    AND  g1 (w1, a, b);
    NOT  g2 (w2, c);
    OR   g3 (w3, w1, w2);
    NAND g4 (y, w3, a);
    NOR  g5 (z, w1, c);
endmodule
