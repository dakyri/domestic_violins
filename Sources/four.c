#include <stdlib.h>
#include <stdio.h>
#include "crack.h"
#include "pv.h"

complex zero = { 0., 0. };
complex one = { 1., 0. };
float PI;
float TWOPI;
float synt = 0.;

main(argc, argv)
    int argc; char *argv[];
{
    int 	R,
		N,
		N2,
		Nw = 0,
		Nw2, 
		D = 0, 
		I = 0,
		i,
		hoop,			/* looping variable */
		in,
		on,
		eof = 0,
		obank = 0,
		aflag = 0,
		sflag = 0,
		Np = 0;
    float 	P = 0.,
		len,
		*Hwin,
		*Wanal,
		*Wsyn,
		*input,
		*winput,
		*buffer,
		*channel,
		*output;
    char	ch,
		*dbuf;

    if (isatty(0))
	usage(1);

    while( (ch= crack( argc, argv, "R|N|M|D|I|P|p|g|ash", 0  )) != NULL ) {
	switch(ch) {
	    case 'R':	R = atoi(arg_option);
			break;
	    case 'N':	N = atoi(arg_option);
			break;
	    case 'M':	Nw = atoi(arg_option);
			break;
	    case 'D':	D = atoi(arg_option);
			break;
	    case 'I':	I = atoi(arg_option);
			break;
	    case 'P':	P = atof(arg_option);
			break;
	    case 'p':	Np = atoi(arg_option);
			break;
	    case 'g':	synt = atof(arg_option);
			break;
	    case 'a':	aflag = 1;
			break;
	    case 's':	sflag = 1;
			break;
	    case 'h':	usage(1);
	}
    }

    if (Nw == 0)
	Nw = N;

    if (aflag && sflag) {
	fprintf(stderr,"specify either -a or -s not both\n");
	exit(1);
    }

    if (aflag)
	I = 0;
    else
	if (I == 0)
	    I = D;

    if (sflag)
	D = 0;

    PI = 4.*atan(1.);
    TWOPI = 8.*atan(1.);
    obank = P != 0.;
    N2 = N>>1;
    Nw2 = Nw>>1;

    Wanal = (float *) space( Nw, sizeof(float) );	/* analysis window */
    Wsyn = (float *) space( Nw, sizeof(float) );	/* synthesis window */
    input = (float *) space( Nw, sizeof(float) );	/* input buffer */
    Hwin = (float *) space( Nw, sizeof(float) );	/* plain Hamming window */
    winput = (float *) space( Nw, sizeof(float) );	/* windowed input buffer */
    buffer = (float *) space( N, sizeof(float) );	/* FFT buffer */
    channel = (float *) space( N+2, sizeof(float) );	/* analysis channels */
    output = (float *) space( Nw, sizeof(float) );	/* output buffer */

/* create windows */

    makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank );

/* initialize input and output time values (in samples) */
    in = -Nw;
    if ( D )
	on = (in*I)/D;
    else
	on = in;

/* main loop--perform phase vocoder analysis-resynthesis */

    while ( !eof ) {

/* increment times */
	in += D;
	on += I;

/* analysis: input D samples; window, fold and rotate input
   samples into FFT buffer; take FFT; and convert to
   amplitude-frequency (phase vocoder) form */
	if ( D == 0 ) {
	  for ( hoop=0; hoop < N+2; hoop++ ) {
	    if ( fread(channel+hoop,sizeof(float),1,stdin) <= 0 )
	      eof = 1;
	  }
	  
	} 
	else {
	    eof = shiftin( input, Nw, D );
	    fold( input, Wanal, Nw, buffer, N, in );
	    rfft( buffer, N2, FORWARD );
	    convert( buffer, channel, N2, D, R );
	}

	if ( aflag ) {  /* analysis output */
	  fwrite( channel, sizeof(float), N+2, stdout );
	    fflush( stdout );
	  continue;
	}
	
/* at this point channel[2*i] contains amplitude data and
   channel[2*i+1] contains frequency data (in Hz) for phase
   vocoder channels i = 0, 1, ... N/2; the center frequency
   associated with each channel is i*f, where f is the
   fundamental frequency of analysis R/N; any desired spectral
   modifications can be made at this point: pitch modifications
   are generally well suited to oscillator bank resynthesis,
   while time modifications are generally well (and more
   efficiently) suited to overlap-add resynthesis */

/* oscillator bank resynthesis */

	unconvert( channel, buffer, N2, I, R );
	rfft( buffer, N2, INVERSE );
	overlapadd( buffer, N, Wsyn, output, Nw, on );
	shiftout( output, Nw, I, on );
    }
    exit(0);
}

usage(woof)
{
    fprintf(stderr, "%s%s%s%s%s%s%s%s%s%s%s",
	"four:  short-time fourier analyzer\n",
	"four   [flags] < floatsams > floatsams\n",
	"	N:	fft length [2^n]\n",
	"	R:	sampling rate\n",
	"	M:	window size in samples\n",
	"	D:	decimation factor in samples\n",
	"	I:	interpolation factor in samples\n",
	"	a:	analysis data output\n",
	"	s:	synthesize analysis input\n",
	"	g:	oscillator resynthesis gate threshold\n",
	"	h:	this joyous place\n");
    exit(woof);
}
