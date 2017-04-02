dM4 = 4.1;
dtM4 = 8.1;
ddM4 = 8;

dM3 = 3.1;

dL3 = 2.1;
dtL3 = 6.3;

knob();

module griglia(passo, duty, Mm, h, rot=0, nega=false){

	/* Genera una grigli, tagliandola ai bordi della superficie bidimensionale passata come children
	Esempio:
	griglia(5, 3, 22, 2)
		circle(r=10);

	Genera una griglia circolare di raggio 10mm. Le sbarre saranno spesse 3mm e i buchi 2mm.

	Impostando nega = true si ottiene la figura complementare, piena dove ci sono i buchi e vuota
	nelle sbarre.
	*/

	s = passo-duty;
	rip = Mm/passo;

	if(nega){
		difference(){
			linear_extrude(h)
				children(0);

			translate(0, 0, -1)
				griglia(passo, duty, Mm, h+2, rot, false)
					children(0);
		}
	}else{
		intersection(){

			//translate([(Mm)/2, Mm/2])
				linear_extrude(h, twist=rot, convexity=10){
					//translate([-(Mm)/2, -Mm/2])
						union(){
							for(i = [0:rip]){
								translate([0, i*passo, 0])
									square([Mm+s, s]);
							}
							for(i = [0:rip]){
								translate([i*passo, 0, 0])
									square([s, Mm]);
							}
						}
				}

			translate(0, 0, -1)
				linear_extrude(h+2)
					children(0);
		}
	}
}

module knob(	D = 35,
							d = 15,
							h = 15,
							n_aletz = 5,
							r_aletz = 7,
							r_min = 1.5,
							fn = 5){
	// MASSIMO fn CONSIGLIATO = 10!!
	$fn = fn;
	a = 360/n_aletz;
	D = D-2*r_min;
	d = d-2*r_min;
	h = h-2*r_min;

	tr(0, 0, h+r_min, 180)
	minkowski() {
		// Genera la forma dello knob
		d(){
			cone(D, d, h);
			for(i=[1:n_aletz])
				r(0, 0, i*a)t(D/2)
					sphere(r_aletz);
		}

		// Lo arrotonda
		if(r_min > 0)
			sphere(r_min);
	}
}

module cone(d1, d2, h){
	cylinder(d1=d1, d2=d2, h=h);
}

module anello(d_in, d_out, h){
	d(){
		cy(d_out, h);
		tz()
			cy(d_in, h+2);
	}
}

module cy(d, h){
	cylinder(d=d, h=h);
}

module c(x, y, z=1, center = false){
	cube([x, y, z], center=center);
}

module d(){
	difference(){
		children(0);
		children([1:$children-1]);
	}
}

module tz(z=-1){
	t(0, 0, z)
		children([0:$children-1]);
}

module t(x=0, y=0, z=0){
	translate([x, y, z])
		children([0:$children-1]);
}

module r(x=0, y=0, z=0){
	rotate([x, y, z])
		children([0:$children-1]);
}

module tr(x=0, y=0, z=0, a=0, b=0, c=0){
	t(x, y, z) r(a, b, c)
		children([0:$children-1]);
}
