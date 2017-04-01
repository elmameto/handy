use <../../lib/std.scad>

// Richieste INTERNE telecomand
min_l = 31.5;
min_p = 76;
min_h = 26.5;

// Dimensioni INTERNE telecomando
mar = 0.7;
_l = min_l + 2*mar;
_h = min_h + 2*mar;
_p = 83;

// Dimensioni ESTERNE telecomando
s = 2;
l = _l + 2*s;
p = _p + 2*s;
h = _h + 2*s;
lato_cianf = 3;


// Pos pot
x_pot = 24;
y_pot = 69;
d_pot = 25;
h_pot = 6;
s_pot = 3;

// Pos pulsante
p_puls = [-2*s, p-10-30, 5];
d_puls = 5;

// Pos interruttore
p_int = [l/2-mar-s, p-8, s];
dim_int = [15.2, 8];

// Dimensioni circuito stampato
xb = 20.32;
xa = 9.652;
ya = 64.008;
dxb = 5.08;
dyb = 3.556;

h_colpcb = 9.5;
d_colpcb = 5;
d_buc = 2.5;
h_pcb = 1.6;

// Altezza delle due metà
mar_h_scatole = 0.7;
h1 = s + h_colpcb + h_pot + h_pcb - mar_h_scatole;
h2 = h-h1;

d_col = 7.5;
diff_h = 0.5;
h_col2 = 3;
h_col1 = _h-2*diff_h-h_col2;
d_tv = 6;
h_buc_tv = h_col2 - 2;
n_buchi = 2;
buchi = [	[l/2-s-mar, 2],
			[s+3.5, p-18]];

$fn = 60;

solo_utili = true;

if(!solo_utili)
tz(s)
	rotella();


montaggio_pcb();
sotto_scatola();
sopra_scatola();
for(i=[0:n_buchi-1])
	t(buchi[i].x, buchi[i].y)
		buco_insca();

module prof_scatola(){
	r = _l-x_pot;
	p_piu = y_pot+r;
	p_meno = _p - p_piu;
	polygon([	[-mar, -mar-p_meno],
				[_l, -mar-p_meno], 
				[_l, y_pot],
				[x_pot, p_piu],
				[-mar, p_piu]	]);
	
	t(x_pot, y_pot)
		circle(r=r);
}

module cianf(){
	rap = [(l-2*lato_cianf)/l, (p-2*lato_cianf)/p];
	
	mirror([0, 0, 1])t(-s+mar/2-0.5, -s+mar/2-0.5)resize([l+1, p+1])
	d(){
		linear_extrude(lato_cianf)
			prof_scatola2();
		
		t(l/2-mar, 35)
			linear_extrude(lato_cianf, scale=rap)
				t(-l/2+mar, -35)
					prof_scatola2();
	}
}

module prof_scatola2(){
	r_min = 6;
	r = _l-x_pot;
	p_piu = y_pot+r;
	p_meno = _p - p_piu;
	polygon([	[-mar, -mar-p_meno+r_min],
				[-mar+r_min, -mar-p_meno],
				[_l-r_min, -mar-p_meno], 
				[_l, -mar-p_meno+r_min], 
				[_l, y_pot],
				[x_pot, p_piu],
				[-mar+r_min, p_piu],
				[-mar, p_piu-r_min]	]);
	
	t(x_pot, y_pot)
		circle(r=r);
	t(-mar+r_min, -mar-p_meno+r_min)
		circle(r_min);
	t(_l-r_min, -mar-p_meno+r_min)
		circle(r_min);
	t(-mar+r_min, p_piu-r_min)
		circle(r_min);
}

module sotto_scatola(){
	d(){
		t(-s+mar/2, -s+mar/2)
		resize([l, p])
			linear_extrude(h1)
				prof_scatola2();
		
		tz(s)
			linear_extrude(h1)
				prof_scatola2();
		
		for(i=[0:n_buchi-1])
			t(buchi[i].x, buchi[i].y, -1)
				cy(d_tv, s+2);
		
		tz(lato_cianf)
			cianf();
		
		t(p_int.x-dim_int.x/2, p_int.y-dim_int.y/2, p_int.z)
			cube([dim_int.x, 3*s, dim_int.y]);
	}
}

module sopra_scatola(){
	tz(h1 + 10)
	d(){
		t(-s+mar/2, -s+mar/2)
		resize([l, p])
			linear_extrude(h2)
				prof_scatola2();
		tz(-s)
			linear_extrude(h2)
				prof_scatola2();
		
		t(x_pot, y_pot, -1)
			cy(d_pot+2.5, h_pot+1+mar_h_scatole);
		
		translate(p_puls)r(0, 90)
			cy(d_puls, 2*s);
		
		tz(h2-lato_cianf)mirror([0, 0, 1])
			cianf();
	}
}

module buco_insca(){
	tz(_h+s+10-h_col1)
		d(){
			cy(d_col, h_col1);
			tz()
				cy(d_buc, h_col1+2);
		}
	
	tz(s)		
		d(){
			cy(d_col, h_col2);
			tz()
				cy(d_buc, h_col2+2);
			#tz()
				cy(d_tv, h_buc_tv+1);
		}
}

module rotella(){
	t(x_pot, y_pot, h_colpcb+h_pcb+h_pot)
		cy(d_pot, 3);
}

module montaggio_pcb(){
	
	t(dxb, dyb){
		sing();
		t(xb)
			sing();
		t(xa, ya)
			sing();
	}
	
	if(!solo_utili)
	tz(h_colpcb)
		profilo_pcb();
	
	module sing(){
		d(){
			cy(d_colpcb, h_colpcb);
			tz()
				cy(d_buc, h_colpcb+2);
		}
	}
	
}

module profilo_pcb(){
	linear_extrude(h_pcb)
		import("/Users/Mamo/MEGA/EL/controller/TX/Board/controller-Edge.Cuts.dxf");
}