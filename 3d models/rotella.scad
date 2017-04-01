h_rot=3;
d_rot=25;
d_buc = 2.5;

d_tv = 6;
h_tv = h_rot - 1.5;

$fn=100;

difference(){
	cylinder(d=d_rot, h=h_rot);

	translate([0, 0, -1])
		cylinder(d=d_buc, h=h_rot+2);
	translate([0, 0, h_rot-h_tv])
		cylinder(d=d_tv, h=h_tv+1);
}