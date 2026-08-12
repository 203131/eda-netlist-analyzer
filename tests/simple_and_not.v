module simple_circuit (
    input a, b,
    output y
);
    wire w1;

    AND g1 (w1, a, b);
    NOT g2 (y, w1);
endmodule