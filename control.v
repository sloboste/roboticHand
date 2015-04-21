// control.v
module control(
input PCLK, 
input PRESERN,

input PSEL,
input PENABLE,
output wire PREADY,
output wire PSLVERR,
input PWRITE, 
input [31:0] PADDR,
input wire [31:0] PWDATA,

output reg [31:0] PRDATA, 
input switch0,
input switch1,
output reg [7:0] pwm,
output reg SW0_INT,
output reg SW1_INT,
input wire jb
);  

//APB bus setup
assign PSLVERR = 0; //assumes no error generation
assign PREADY = 1; //assumes zero wait
wire write_enable = (PENABLE && PWRITE && PSEL); //decodes APB3 write cycle
wire read_enable = (!PWRITE && PSEL); //decode APB3 read cycle


//INT GENERATION
always@(posedge PCLK) begin
    if(switch0)
        SW0_INT <= 1;
    else
        SW0_INT <= 0;

    if (switch1)
        SW1_INT <= 1;
    else
        SW1_INT <= 0;
end


//Tried to make a pwm module but it was optimized out to combinational logic every time during sythesis...
//PWM  for servo 0
wire servo_write0 = write_enable & (PADDR[4:2] == 3'b000); //0x40050100 --> blah 0001 0000 0000
`define period 2000000
reg [31:0] count0;
reg [4:0] counter0 = 5'b00000;
reg [31:0] pulseWidth0;
always @(posedge PCLK) begin
    if(servo_write0)
    pulseWidth0<=PWDATA;
end
always @(posedge PCLK) begin
    if (count0 == `period)
        count0 <= 0;
    else
        count0 <= count0 + 1;
    if (count0 < pulseWidth0)
        pwm[0] <= 1;
    else
        pwm[0] <= 0;
end

//PWM  for servo 1
wire servo_write1 = write_enable & (PADDR[4:2] == 3'b001); //0x40050104 --> blah 0001 0000 0100
reg [31:0] count1;
reg [4:0] counter1 = 5'b00000;
reg [31:0] pulseWidth1;
always @(posedge PCLK) begin
    if(servo_write1)
    pulseWidth1<=PWDATA;
end
always @(posedge PCLK) begin
    if (count1 == `period)
        count1 <= 0;
    else
        count1 <= count1 + 1;
    if (count1 < pulseWidth1)
        pwm[1] <= 1;
    else
        pwm[1] <= 0;
end

//PWM  for servo 2
wire servo_write2 = write_enable & (PADDR[4:2] == 3'b010);
reg [31:0] count2;
reg [4:0] counter2 = 5'b00000;
reg [31:0] pulseWidth2;
always @(posedge PCLK) begin
    if(servo_write2)
    pulseWidth2<=PWDATA;
end
always @(posedge PCLK) begin
    if (count2 == `period)
        count2 <= 0;
    else
        count2 <= count2 + 1;
    if (count2 < pulseWidth2)
        pwm[2] <= 1;
    else
        pwm[2] <= 0;
end

//PWM  for servo 3
wire servo_write3 = write_enable & (PADDR[4:2] == 3'b011);
reg [31:0] count3;
reg [4:0] counter3 = 5'b00000;
reg [31:0] pulseWidth3;
always @(posedge PCLK) begin
    if(servo_write3)
    pulseWidth3<=PWDATA;
end
always @(posedge PCLK) begin
    if (count3 == `period)
        count3 <= 0;
    else
        count3 <= count3 + 1;
    if (count3 < pulseWidth3)
        pwm[3] <= 1;
    else
        pwm[3] <= 0;
end

//PWM  for servo 4
wire servo_write4 = write_enable & (PADDR[4:2] == 3'b100);
reg [31:0] count4;
reg [4:0] counter4 = 5'b00000;
reg [31:0] pulseWidth4;
always @(posedge PCLK) begin
    if(servo_write4)
    pulseWidth4<=PWDATA;
end
always @(posedge PCLK) begin
    if (count4 == `period)
        count4 <= 0;
    else
        count4 <= count4 + 1;
    if (count4 < pulseWidth4)
        pwm[4] <= 1;
    else
        pwm[4] <= 0;
end

//PWM  for servo 5
wire servo_write5 = write_enable & (PADDR[4:2] == 3'b101);
reg [31:0] count5;
reg [4:0] counter5 = 5'b00000;
reg [31:0] pulseWidth5;
always @(posedge PCLK) begin
    if(servo_write5)
    pulseWidth5<=PWDATA;
end
always @(posedge PCLK) begin
    if (count5 == `period)
        count5 <= 0;
    else
        count5 <= count5 + 1;
    if (count5 < pulseWidth5)
        pwm[5] <= 1;
    else
        pwm[5] <= 0;
end

//PWM  for servo 6
wire servo_write6 = write_enable & (PADDR[4:2] == 3'b110);
reg [31:0] count6;
reg [4:0] counter6 = 5'b00000;
reg [31:0] pulseWidth6;
always @(posedge PCLK) begin
    if(servo_write6)
    pulseWidth6<=PWDATA;
end
always @(posedge PCLK) begin
    if (count6 == `period)
        count6 <= 0;
    else
        count6 <= count6 + 1;
    if (count6 < pulseWidth6)
        pwm[6] <= 1;
    else
        pwm[6] <= 0;
end

// Joystick button press (not used)
wire jb_read = read_enable & (PADDR[4:2] == 3'b111);
wire jb_debounced;
Button_Debouncer bd0(PCLK, jb, jb_debounced);
always @(posedge PCLK) begin
    if (jb_read) begin
        PRDATA[31:1] <= 0;
        PRDATA[0] <= jb_debounced;
    end 
end

endmodule