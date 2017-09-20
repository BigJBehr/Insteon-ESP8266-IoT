// Insteon Switch Box
X = 92;
Y = 52;

Floor = 3;          // thickness of the case top and bottom

D = 6;              // diameter of the rounded corners
R = D / 2;

PostD  = 7;         // diameter of board mounting bosses
PostZ  = 7.5;       // height of the board mounting bosses. provides a 0.5mm gap

                    // between the top of a switch and the bottom of a button
// uncomment one and only one of these. this is the size of the hole in the
// board mounting bosses. pick the correct size for the hardware you are using.
//SHD = 1.75;         // correct size for #2 pan tapping screw
//SHD = 2.25;         // correct size for M2.5 pan tapping screw or tap
//SHD = 2.25;         // correct size for 4-40 screw or tap
SHD = 2.75;         // correct size for M3 screw or tap
//SHD = 3;            // correct size for 6-32 screw

JackD = 8;          // barrel jack clearence hole is 8mm

// Floor + JackD is the height of the top of the power jack
// PostZ + 2 is how much the PC board hangs down from the top
// add in 5mm for clearance and jack offset from floor
Z = Floor + JackD + 5 + PostZ;  // minimum height so that board clears power jack

//*****************************************************************************
// if you are using an ESP8266 board that is socketed then un-comment this
//Z = 26;          // extra height to accomodate socket strips
//*****************************************************************************

//*****************************************************************************
// This is the start of the main code that 3D prints all of the parts.
//*****************************************************************************
$fn = 50;

// ***** Here is where all the parts are printed at the same time *****
// Comment out what you do not want to print by placing two slashes ("//") in
// front of the line.

// Prints the case Bottom
Bottom();

// Moves the case top away from the bottom and prints it. Translate uses
// [X, Y, Z] coordinates to move the Top.
translate([0, -70, 0]) Top();

// You will need eight buttons. You can make any combianation of flat top
// or bullet buttons. The default is four flat top and four bullet buttons.
// Translate moves the buttons away from the Bottom, Top and other buttons.
// You can have one letter printed on top of FlatButtons. If you do not want
// a letter then use "" to specifiy no text.

bx = -25;
translate([bx, -15, 0]) FlatButton("");
translate([bx, -45, 0]) FlatButton("");
translate([bx,  15, 0]) FlatButton("");
translate([bx,  45, 0]) FlatButton("");

bbx = 120;
translate([bbx, -15, 0]) BulletButton();
translate([bbx, -45, 0]) BulletButton();
translate([bbx,  15, 0]) BulletButton();
translate([bbx,  45, 0]) BulletButton();

//***** End of the main code *************************************************

// ***** case bottom *****
module Bottom()
{
    translate ([X, 0, 0]) cylinder(d = 20, h = .25);  //  removel tab
    difference()
    {
        Base(Z);
        translate([0, 0, Floor]) cube([X, Y, Z]);
        PowerJack();
    }   // bottom difference
}   //  Bottom


// ***** case top *****
module Top()
{
    translate([X, 0 , 0]) cylinder(d = 20, h = .25);  //  removel tab

    difference()
    {
        union()
        {
            Base(Floor);
            translate([0, 0, Floor]) cube([X, Y, Floor]);
        }
        
        translate([2, 2, Floor]) cube([X -4 , Y - 4, Floor]);
//        translate([0, Y2, 0]) Switches(9);
        Switches(9);
    }   // top difference
    
    // add board hounting bosses
    byh2h = 44;
    bxh2h = 84;

    bxoffset = (X - bxh2h) / 2;
    byoffset = (Y - byh2h) / 2;
    
    translate([bxoffset, byoffset, Floor]) Bosses();
}   //  Top    


module Base(z)
{
    hull()
    {
        cylinder(d = D, h = z);
        translate([X, 0, 0]) cylinder(d = D, h = z);
    }

    translate([0, Y, 0]) hull()
    {
        cylinder(d = D, h = z);
        translate([X, 0, 0]) cylinder(d = D, h = z);
    }
    
    translate([-R, 0, 0]) cube([X + D, Y, z]);
}   //  Base

module Bosses()
{
    difference()
    {
        Posts(PostD);
        Posts(SHD);
    }
}   //  Bosses

module Posts(d)
{
    yh2h = 44;
    xh2h = 84;

    cylinder(d = d, h = PostZ);
    translate([xh2h, 0, 0]) cylinder(d = d, h = PostZ);
    translate([0, yh2h, 0]) cylinder(d = d, h = PostZ);
    translate([xh2h, yh2h, 0]) cylinder(d = d, h = PostZ);
}   //  Posts

module Switches(tc)
{
    xh2h = 22.5;
    yh2h = 40;
    xoffset =  (X - (3 * xh2h)) / 2;
    yoffest =  (Y - yh2h) / 2;
    d = ButtonD + 0.5;
    d1 = d + 3 ;

    translate([xoffset, yoffest, 0])
    {
        cylinder(d = d, h = Floor);
        translate([xh2h, 0, 0]) cylinder(d = d, h = Floor);
        translate([xh2h * 2, 0, 0]) cylinder(d = d, h = Floor);
        translate([xh2h * 3, 0, 0]) cylinder(d = d, h = Floor);

        translate([0, yh2h, 0]) cylinder(d = d, h = Floor);
        translate([xh2h, yh2h, 0]) cylinder(d = d, h = Floor);
        translate([xh2h * 2, yh2h, 0]) cylinder(d = d, h = Floor);
        translate([xh2h * 3, yh2h, 0]) cylinder(d = d, h = Floor);
    }

    translate([xoffset, yoffest, Floor])
    {
        cylinder(d = d1, h = Floor);
        translate([xh2h, 0, 0]) cylinder(d = d1, h = Floor);
        translate([xh2h * 2, 0, 0]) cylinder(d = d1, h = Floor);
        translate([xh2h * 3, 0, 0]) cylinder(d = d1, h = Floor);

        translate([0, yh2h, 0]) cylinder(d = d1, h = Floor);
        translate([xh2h, yh2h, 0]) cylinder(d = d1, h = Floor);
        translate([xh2h * 2, yh2h, 0]) cylinder(d = d1, h = Floor);
        translate([xh2h * 3, yh2h, 0]) cylinder(d = d1, h = Floor);
    }

}   //  Switches

// The default position of the power jack is in the middle of one of the long sides.
// If the height of the box walls exceeds 25mm then the power jack is centered on one
// of the short sides. This prevents interference with the ESP8266 board.
module PowerJack()
{
    d = JackD;      // barrel clearence is 8mm
    d1 = d + 2;
    
    if (Z < 25)
    {
        translate([X / 2, 0, Floor + (d1 / 2)]) rotate([90, 0, 0]) cylinder(d = d, h = 5);
        translate([X / 2, 0, Floor + (d1 / 2)]) rotate([90, 0, 0]) cylinder(d = d1, h = 1);
    }
    else
    {
        translate([0, Y / 2, Floor + (d1 / 2)]) rotate([0, -90, 0]) cylinder(d = d, h = 5);
        translate([0, Y / 2, Floor + (d1 / 2)]) rotate([0, -90, 0]) cylinder(d = d1, h = 1);
    }
}   //  PowerJack

ButtonHeight = Floor + 2;
ButtonD = 8;

module FlatButton(text)
{
    Button();
    translate([0, 0, ButtonHeight]) cylinder(d = ButtonD, h = 3);
    
    translate([0, -2.5, ButtonHeight + 3])
    {
        linear_extrude(height = 1) text(text, font = "Liberation Sans", size = 5, , halign = "center");
    }
}   //  BulletButton

module BulletButton()
{
    d = 8;
    
    Button();
    translate([0, 0, ButtonHeight]) sphere(d = d);
}   //  BulletButton

module Button()
{
    d1 = ButtonD + 2;
    
    cylinder(d = d1, h = 2);
    cylinder(d = ButtonD, h = ButtonHeight);
    
    cylinder(d = 20, h = .25);  //  removel tab
}   //  Button
