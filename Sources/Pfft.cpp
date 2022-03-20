#include <math.h>
#include "Pfft.h"

#ifdef PV
/* bitreverse places double array x containing N/2 complex values
   into bit-reversed order */

bitreverse( double x[], int N )
{
  double 	rtemp,itemp;
  int 		i,j,
		m;

    for ( i = j = 0; i < N; i += 2, j += m ) {
	if ( j > i ) {
	    rtemp = x[j]; itemp = x[j+1]; /* complex exchange */
	    x[j] = x[i]; x[j+1] = x[i+1];
	    x[i] = rtemp; x[i+1] = itemp;
	}
	for ( m = N>>1; m >= 2 && j >= m; m >>= 1 )
	    j -= m;
    }
}

/* cfft replaces double array x containing NC complex values
   (2*NC double values alternating real, imagininary, etc.)
   by its Fourier transform if forward is true, or by its
   inverse Fourier transform if forward is false, using a
   recursive Fast Fourier transform method due to Danielson
   and Lanczos.  NC MUST be a power of 2. */

cfft( double x[], int NC, int forward )
{
	double 	wr,wi,
			wpr,wpi,
			theta,
			scale;
	int 		mmax,
			ND,
			m,
			i,j,
			delta;
	
	ND = NC<<1;
	bitreverse( x, ND );
	for ( mmax = 2; mmax < ND; mmax = delta ) {
		delta = mmax<<1;
		theta = 2*PI/( forward? mmax : -mmax );
		wpr = -2.*pow( sin( 0.5*theta ), 2. );
		wpi = sin( theta );
		wr = 1.;
		wi = 0.;
		for ( m = 0; m < mmax; m += 2 ) {
			register double rtemp, itemp;
			for ( i = m; i < ND; i += delta ) {
				j = i + mmax;
				rtemp = wr*x[j] - wi*x[j+1];
				itemp = wr*x[j+1] + wi*x[j];
				x[j] = x[i] - rtemp;
				x[j+1] = x[i+1] - itemp;
				x[i] += rtemp;
				x[i+1] += itemp;
			}
			wr = (rtemp = wr)*wpr - wi*wpi + wr;
			wi = wi*wpr + rtemp*wpi + wi;
		}
	}
	
	/* scale output */
	
	scale = forward ? 1./ND : 2.;
	{ register double *xi=x, *xe=x+ND;
	while ( xi < xe )
		*xi++ *= scale;
	}
}

/* If forward is true, rfft replaces 2*N real data points in x with
   N complex values representing the positive frequency half of their
   Fourier spectrum, with x[1] replaced with the real part of the Nyquist
   frequency value.  If forward is false, rfft expects x to contain a
   positive frequency spectrum arranged as before, and replaces it with
   2*N real values.  N MUST be a power of 2. */


rfft( double x[], int N, short forward )
{
	double 	c1,c2,
			h1r,h1i,
			h2r,h2i,
			wr,wi,
			wpr,wpi,
			temp,
			theta;
	double 	xr,xi;
	int		i, i1,i2,i3,i4,	N2p1;
	
	theta = PI/N;
	wr = 1.;
	wi = 0.;
	c1 = 0.5;
	if ( forward ) {
		c2 = -0.5;
		cfft( x, N, forward );
		xr = x[0];
		xi = x[1];
	} else {
		c2 = 0.5;
		theta = -theta;
		xr = x[1];
		xi = 0.;
		x[1] = 0.;
	}
	wpr = -2.*pow( sin( 0.5*theta ), 2. );
	wpi = sin( theta );
	N2p1 = (N<<1) + 1;
	for ( i = 0; i <= N>>1; i++ ) {
		i1 = i<<1;
		i2 = i1 + 1;
		i3 = N2p1 - i2;
		i4 = i3 + 1;
		if ( i == 0 ) {
			h1r =  c1*(x[i1] + xr );
			h1i =  c1*(x[i2] - xi );
			h2r = -c2*(x[i2] + xi );
			h2i =  c2*(x[i1] - xr );
			x[i1] =  h1r + wr*h2r - wi*h2i;
			x[i2] =  h1i + wr*h2i + wi*h2r;
			xr =  h1r - wr*h2r + wi*h2i;
			xi = -h1i + wr*h2i + wi*h2r;
		} else {
			h1r =  c1*(x[i1] + x[i3] );
			h1i =  c1*(x[i2] - x[i4] );
			h2r = -c2*(x[i2] + x[i4] );
			h2i =  c2*(x[i1] - x[i3] );
			x[i1] =  h1r + wr*h2r - wi*h2i;
			x[i2] =  h1i + wr*h2i + wi*h2r;
			x[i3] =  h1r - wr*h2r + wi*h2i;
			x[i4] = -h1i + wr*h2i + wi*h2r;
		}
		wr = (temp = wr)*wpr - wi*wpi + wr;
		wi = wi*wpr + temp*wpi + wi;
	}
	if ( forward )
		x[1] = xr;
	else
		cfft( x, N, forward );
}
#endif

Pfft::Pfft(int bits, double SR)
{
	int    i,y;
	double v;

	N = 1 << bits;
	P = bits;
	pertable = new short[N];
	ctable = new double[N];
	stablet = new double[N];
	stablei = new double[N];
	lastphase = new double[N];
	SampleRate = SR;
	
	for(i=0;i<N;i++)
		pertable[i]=permute(i);
	v = 2.0*PI/(double)N;
	for(i=0;i<N/2;i++)
	{
		y=pertable[i*2];
		ctable[i]=cos(v*y);
		stablet[i]=-sin(v*y);
		stablei[i]=sin(v*y);
	}
}

Pfft::~Pfft(void)
{
	delete [] pertable;
	delete [] ctable;
	delete [] stablet;
	delete [] stablei;
}

int Pfft::permute(int n)
{
	int 	ac,
			i;

	ac = 0;
	for(i = 0; i < P; i++)
	{
		ac <<= 1;
		if((n & 1) == 1)
			ac |= 1;
		n >>= 1;
	}

	return ac;
}

void Pfft::forward(double *data, double *real, double *imag)
{
#ifdef PV
	double	x[1024];
	for (short i=0; i<N; i++)
		x[i] = data[i];
	rfft(x, N/2, TRUE);
	for (short i=0; i<N/2; i++) {
		real[i] = x[2*i];
		imag[i] = x[2*i+1];
//		fprintf(stderr, "tran %g %g %g\n", data[i], real[i], imag[i]);
	}
#else		
	unsigned i1,i2,i3,i4;
	int loop,loop1,loop2;
	double a1,a2,b1,b2,z1,z2,v,tr,ti;

	for(loop = 0; loop < N; loop++)
	{
		real[loop] = data[loop]/20;
		imag[loop] = 0;
	}

    i1 = N >> 1;
	i2 = 1;
	v = 2.0*PI/(double)N;
	for(loop = 0; loop < P; loop++)
	{
		i3 = 0;
		i4 = i1;
		for(loop1 = 0; loop1 < i2; loop1++)
		{
			z1 =  ctable[loop1];
			z2 =  stablet[loop1];
			for(loop2 = i3; loop2 < i4; loop2++)
			{
				a1 = real[loop2];
				a2 = imag[loop2];
				b1 = z1*real[loop2+i1] - z2*imag[loop2+i1];
				b2 = z2*real[loop2+i1] + z1*imag[loop2+i1];
				real[loop2] 	 = a1 + b1;
				imag[loop2] 	 = a2 + b2;
				real[loop2 + i1] = a1 - b1;
				imag[loop2 + i1] = a2 - b2;
			}
			i3 += (i1<<1);
			i4 += (i1<<1);
		}
		i1 >>= 1;
		i2 <<= 1;
	}
	for(loop=0;loop<N;loop++)
	{
		loop2=pertable[loop];
		if (loop<loop2)
		{
			tr=real[loop];
			ti=imag[loop];
			real[loop]=real[loop2];
			imag[loop]=imag[loop2];
			real[loop2]=tr;
			imag[loop2]=ti;
		}
	}

#endif
}

void Pfft::reverse(double *data, double *real, double *imag)
{
#ifdef PV
	double	x[1024];
	for (short i=0; i<N/2; i++) {
		x[2*i] = real[i];
		x[2*i+1] = imag[i];
	}
	rfft(x, N/2, FALSE);
	for (short i=0; i<N; i++)
		x[i] = data[i];
#else		
	unsigned i1,i2,i3,i4;
	int loop,loop1,loop2;
	double a1,a2,b1,b2,z1,z2,v;

	i1 = N >> 1;
	i2 = 1;
	v  = 2*PI/(double)N;
	for(loop = 0; loop < P; loop++)
	{
		i3 = 0;
		i4 = i1;
		for(loop1 = 0; loop1 < i2; loop1++)
		{
			z1 = ctable[loop1];
			z2 = stablei[loop1];
			for(loop2 = i3; loop2 < i4; loop2++)
			{
				a1 = real[loop2];
				a2 = imag[loop2];
				b1 = z1*real[loop2+i1] - z2*imag[loop2+i1];
				b2 = z2*real[loop2+i1] + z1*imag[loop2+i1];
				real[loop2] 	 = a1 + b1;
				imag[loop2] 	 = a2 + b2;
				real[loop2 + i1] = a1 - b1;
				imag[loop2 + i1] = a2 - b2;
			}
			i3 += (i1<<1);
			i4 += (i1<<1);
		}
		i1 >>= 1;
		i2 <<= 1;
	}
  
	for(loop = 0; loop < N; loop++)
		data[loop] = real[pertable[loop]]*2;
#endif
}
              
void Pfft::tospec(double *pow, double *freq,
			 double *real, double *imag)
{
	short		i;
	double		phasediff, phase;
	double		fundamental = (double) SampleRate/N;
	double		factor = SampleRate/(N*2*PI);
	
	for ( i = 0; i < N; i++ ) {
		/* compute magnitude value from real and imaginary parts */
		
		pow[i] = sqrt( real[i]*real[i] + imag[i]*imag[i] );
		
		/* compute phase value from real and imaginary parts and take
		difference between this and previous value for each channel */
		
//		if ( pow[i] == 0. )
//			phasediff = 0.;
//		else {
//			phasediff = ( phase = -atan2( imag[i], real[i] ) ) - lastphase[i];
//		lastphase[i] = phase;

		
		/* unwrap phase differences */
		
//		while ( phasediff > PI )
//			phasediff -= 2.0*PI;
//		while ( phasediff < -PI )
//			phasediff += 2.0*PI;
//		}

		phasediff = -atan2( imag[i], real[i] );		
		
		/* convert each phase difference to Hz */
		
		freq[i] = phasediff*factor + i*fundamental;
	}

}

void Pfft::fromspec(double *pow, double *freq,
			 double *real, double *imag)
{					
	double	mag;
	double	fundamental = SampleRate/N;
	double	factor = 2.0*PI*N/SampleRate;
	double	phase;
	short	i;
	
	for ( i = 0; i <N; i++ ) {
		mag = pow[i];
//		lastphase[i] += freq[i] - i * fundamental;
//		phase = lastphase[i]*factor;
		phase = (freq[i] - i*fundamental)*factor;
		real[i] = mag*cos( phase );
		imag[i] = -mag * sin( phase );
//	fprintf(stderr, "fr %g %g %g %g %g %g\n", pow[i], freq[i], mag, phase, real[i], mag[i]);
	}
}
