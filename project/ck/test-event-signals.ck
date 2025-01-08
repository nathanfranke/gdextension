global Event BigEvent;
global Event the_nextNote;
global float the_frequency;
global float filter_freq;

// patch
Blit s => LPF filter => ADSR e => JCRev r => dac;
.5 => s.gain;
.05 => r.mix;
2000.0 => filter_freq => filter.freq;
// set adsr
e.set( 5::ms, 3::ms, .5, 5::ms );

// an array
[ 0, 2, 4, 7, 9, 11 ] @=> int hi[];

// infinite time loop
while( true )
{
    // frequency
    Std.mtof( 33 + Math.random2(0,3) * 12 +
        hi[Math.random2(0,hi.size()-1)] ) => s.freq;
    // copy to global
    s.freq() => the_frequency;
    filter_freq => filter.freq;

    // harmonics
    Math.random2( 1, 5 ) => s.harmonics;

    // key on
    e.keyOn();

    BigEvent.broadcast();

    // wait on event
    the_nextNote => now;
    // key off
    e.keyOff();
    // advance time for release
    5::ms => now;
}
