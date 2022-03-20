#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define FORWARD 1
#define INVERSE 0
#define SIZE 512

float		TWOPI=2*PI;

/* If forward is true, rfft replaces 2*N real data points in x with
   N complex values representing the positive frequency half of their
   Fourier spectrum, with x[1] replaced with the real part of the Nyquist
   frequency value.  If forward is false, rfft expects x to contain a
   positive frequency spectrum arranged as before, and replaces it with
   2*N real values.  N MUST be a power of 2. */

rfft( float x[], int N, short forward )
{
	float 	c1,c2,
			h1r,h1i,
			h2r,h2i,
			wr,wi,
			wpr,wpi,
			temp,
			theta;
	float 	xr,xi;
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

/* cfft replaces float array x containing NC complex values
   (2*NC float values alternating real, imagininary, etc.)
   by its Fourier transform if forward is true, or by its
   inverse Fourier transform if forward is false, using a
   recursive Fast Fourier transform method due to Danielson
   and Lanczos.  NC MUST be a power of 2. */

cfft( x, NC, forward )
float x[]; int NC, forward;
{
	float 	wr,wi,
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
		theta = TWOPI/( forward? mmax : -mmax );
		wpr = -2.*pow( sin( 0.5*theta ), 2. );
		wpi = sin( theta );
		wr = 1.;
		wi = 0.;
		for ( m = 0; m < mmax; m += 2 ) {
			register float rtemp, itemp;
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
	{ register float *xi=x, *xe=x+ND;
	while ( xi < xe )
		*xi++ *= scale;
	}
}

/* bitreverse places float array x containing N/2 complex values
   into bit-reversed order */

bitreverse( x, N )
float x[]; int N;
{
  float 	rtemp,itemp;
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

/* S is a spectrum in rfft format, i.e., it contains N real values
   arranged as real followed by imaginary values, except for first
   two values, which are real parts of 0 and Nyquist frequencies;
   convert first changes these into N/2+1 PAIRS of magnitude and
   phase values to be stored in output array C; the phases are then
   unwrapped and successive phase differences are used to compute
   estimates of the instantaneous frequencies for each phase vocoder
   analysis channel; decimation rate D and sampling rate R are used
   to render these frequency values directly in Hz. */

convert( S, C, N2, D, R )
float S[], C[]; int N2, D, R;
{
	static int 		first = 1;
	static float 	*lastphase,
					fundamental,
					factor;
	float 			phase,
					phasediff;
	int 			real, imag,	amp, freq;
	float 			a,b;
	int 			i;
	
	/* first pass: allocate zeroed space for previous phase
	values for each channel and compute constants */
	
	if ( first ) {
		first = 0;
		lastphase = (float *) space( N2+1, sizeof(float) );
		fundamental = (float) R/(N2<<1);
		factor = R/(D*TWOPI);
	} 
	
	/* unravel rfft-format spectrum: note that N2+1 pairs of
	values are produced */
	
	for ( i = 0; i <= N2; i++ ) {
		imag = freq = ( real = amp = i<<1 ) + 1;
		a = ( i == N2 ? S[1] : S[real] );
		b = ( i == 0 || i == N2 ? 0. : S[imag] );
		
		/* compute magnitude value from real and imaginary parts */
		
		C[amp] = hypot( a, b );
		
		/* compute phase value from real and imaginary parts and take
		difference between this and previous value for each channel */
		
		if ( C[amp] == 0. )
			phasediff = 0.;
		else {
			phasediff = ( phase = -atan2( b, a ) ) - lastphase[i];
		lastphase[i] = phase;
		
		/* unwrap phase differences */
		
		while ( phasediff > PI )
			phasediff -= TWOPI;
		while ( phasediff < -PI )
			phasediff += TWOPI;
		}
		
		/* convert each phase difference to Hz */
		
		C[freq] = phasediff*factor + i*fundamental;
	}
}

/* unconvert essentially undoes what convert does, i.e., it
  turns N2+1 PAIRS of amplitude and frequency values in
  C into N2 PAIR of complex spectrum data (in rfft format)
  in output array S; sampling rate R and interpolation factor
  I are used to recompute phase values from frequencies */

unconvert( C, S, N2, I, R )
float C[], S[]; int N2, I, R;
{
	static int 	first = 1;
	static float 	*lastphase,
	fundamental,
	factor;
	int 	i,
	real,
	imag,
	amp,
	freq;
	float mag,
	phase;
	char	*space();
	
	/* first pass: allocate memory and compute constants */
	
	if ( first ) {
		first = 0;
		lastphase = (float *) space( N2+1, sizeof(float) );
		fundamental = (float) R/(N2<<1);
		    factor = TWOPI*I/R;
	} 
	
	/* subtract out frequencies associated with each channel,
	compute phases in terms of radians per I samples, and
	convert to complex form */
	
	for ( i = 0; i <= N2; i++ ) {
		imag = freq = ( real = amp = i<<1 ) + 1;
		if ( i == N2 )
			real = 1;
		mag = C[amp];
		lastphase[i] += C[freq] - i * fundamental;
		phase = lastphase[i]*factor;
		S[real] = mag*cos( phase );
		if ( i != N2 )
			S[imag] = -mag * sin( phase );
	}
}
