// pwm.v
module pwm(
    input wire servo_write,
    input wire PCLK,
    input wire [31:0] PWDATA,
    output reg out,
    output reg [31:0]pulseWidth
);
//PWM  for servo
`define period 2000000
reg [31:0] count;
reg [4:0] counter = 5'b00000;
//reg [31:0] pulseWidth = 0;

always @(posedge PCLK) begin
    if(servo_write)
    	pulseWidth<=PWDATA;
    if (count == `period)
        count <= 0;
    else
        count <= count + 1;
        
    if (count < pulseWidth)
        out <= 1;
    else
        out <= 0;
end

endmodule