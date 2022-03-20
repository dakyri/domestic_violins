#include "SpecDisplay.h"

SpecDisplay::SpecDisplay(BRect where, sem_id buf_sem,
							int n1, double *xp, int n2, double *yp,
							float off, float sc)
	: BView(where, "spec", B_FOLLOW_ALL, B_WILL_DRAW)
{
	reg = Bounds();

	nx = n1,
	x = xp;
	ny = n2;
	y = yp;
	sem = buf_sem;
	scale = sc;
	offset = off;
	snap = new float [nx];
	StrokeRect(reg);
}

void SpecDisplay::Draw(BRect region)
{
	BPoint	start_point(0,0),
			end_point(0,0);
	float	xdiff = 2*(reg.right - reg.left) / nx,
			fact = (reg.bottom - reg.top)/scale;
	rgb_color	l_color,
				w_color;
	l_color.red = l_color.green = l_color.blue = 0;
	w_color.red = w_color.green = w_color.blue = 255;

	BeginLineArray(nx);
	start_point.Set(0, (snap[0])*fact+offset);
	end_point.Set(0,0);
	for (short i=1; i<nx; i+=2) {
		end_point.y = (snap[i])*fact+offset;
		AddLine(start_point, end_point, w_color);
		start_point = end_point;
		end_point.x+=xdiff;
	}
	EndLineArray();
	
	acquire_sem(sem);
	for (short i=0; i<nx; i++) {
		snap[i] = x[i];
	}
	release_sem(sem);	
	BeginLineArray(nx);
	start_point.Set(0, (snap[0])*fact+offset);
	end_point.Set(0, 0);
	for (short i=1; i<nx; i+=2) {
		end_point.y = (snap[i])*fact+offset;
		AddLine(start_point, end_point, l_color);
		start_point = end_point;
		end_point.x+=xdiff;
	}
	EndLineArray();
	StrokeRect(reg, B_SOLID_HIGH);
}

void SpecDisplay::Draw()
{
	Draw(reg);
}

