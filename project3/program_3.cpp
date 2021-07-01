#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <stdbool.h>
#include<fstream>
#include<iostream>
#include<string>
#include<iomanip>


using namespace std;

unsigned int seed = 0;

omp_lock_t	Lock;
int		NumInThreadTeam;
int		NumAtBarrier;
int		NumGone;
int count_date;

int	NowYear;		// 2021 - 2026
int	NowMonth;		// 0 - 11

float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// grain height in inches
int	NowNumDeer;		// number of deer in the current population
int NowNumHunter;
const float GRAIN_GROWS_PER_MONTH =		9.0;
const float ONE_DEER_EATS_PER_MONTH =		1.0;

const float AVG_PRECIP_PER_MONTH =		7.0;	// average
const float AMP_PRECIP_PER_MONTH =		6.0;	// plus or minus
const float RANDOM_PRECIP =			2.0;	// plus or minus noise

const float AVG_TEMP =				60.0;	// average
const float AMP_TEMP =				20.0;	// plus or minus
const float RANDOM_TEMP =			10.0;	// plus or minus noise

const float MIDTEMP =				40.0;
const float MIDPRECIP =				10.0;




float
SQR( float x )
{
        return x*x;
}


void
InitBarrier( int n )
{
        NumInThreadTeam = n;
        NumAtBarrier = 0;
	omp_init_lock( &Lock );
}


void
WaitBarrier( )
{
        omp_set_lock( &Lock );
        {
                NumAtBarrier++;
                if( NumAtBarrier == NumInThreadTeam )
                {
                        NumGone = 0;
                        NumAtBarrier = 0;
                        // let all other threads get back to what they were doing
			// before this one unlocks, knowing that they might immediately
			// call WaitBarrier( ) again:
                        while( NumGone != NumInThreadTeam-1 );
                        omp_unset_lock( &Lock );
                        return;
                }
        }
        omp_unset_lock( &Lock );

        while( NumAtBarrier != 0 );	// this waits for the nth thread to arrive

        #pragma omp atomic
        NumGone++;			// this flags how many threads have returned
}


float
Ranf( unsigned int *seedp,  float low, float high )
{
        float r = (float) rand_r( seedp );              // 0 - RAND_MAX

        return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}


int
Ranf( unsigned int *seedp, int ilow, int ihigh )
{
        float low = (float)ilow;
        float high = (float)ihigh + 0.9999f;

        return (int)(  Ranf(seedp, low,high) );
}


void temperature_calculate(){

    float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

    float temp = AVG_TEMP - AMP_TEMP * cos( ang );
    NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
    NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );
    if( NowPrecip < 0. )
	    NowPrecip = 0.;


}






void Deer(){
    
    while( NowYear < 2027 )
    {
	    // compute a temporary next-value for this quantity
	    // based on the current state of the simulation:
	    //. . .
        int nextNumDeer = NowNumDeer;
        int carryingCapacity = (int)( NowHeight );
        if( nextNumDeer < carryingCapacity )
            nextNumDeer++;
        else
            if( nextNumDeer > carryingCapacity )
                nextNumDeer--;
        
        nextNumDeer = nextNumDeer - NowNumHunter/3;
        

        if( nextNumDeer < 0 )
            nextNumDeer = 1;

	    // DoneComputing barrier:
	    WaitBarrier();
        NowNumDeer = nextNumDeer;
	    //. . .

    	// DoneAssigning barrier:
    	WaitBarrier();
    	//. . .

    	// DonePrinting barrier:
    	WaitBarrier();
    	//. . .
    }
}

void Grain(){

    while( NowYear < 2027 )
    {   
        
        float tempFactor = exp( -SQR(  ( NowTemp - MIDTEMP ) / 10.  )   );
        float precipFactor = exp( -SQR(  ( NowPrecip - MIDPRECIP ) / 10.  )   );


        float nextHeight = NowHeight;
        nextHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
        nextHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;

        if( nextHeight < 0. ) nextHeight = 0.;


	    // compute a temporary next-value for this quantity
	    // based on the current state of the simulation:
	    //. . .

	    // DoneComputing barrier:
	    WaitBarrier();
        NowHeight = nextHeight;
	    //. . .

    	// DoneAssigning barrier:
    	WaitBarrier();
    	//. . .

    	// DonePrinting barrier:
    	WaitBarrier();
    	//. . .
    }

}

void Hunter(){

    while( NowYear < 2027 )
    {
	    // compute a temporary next-value for this quantity
	    // based on the current state of the simulation:
	    //. . .
        int nextNumHunter = NowNumHunter++;

        if(NowMonth == 9 || NowMonth == 10 || NowMonth == 11){

            nextNumHunter = nextNumHunter + 2;

        }
        
        if(NowMonth == 0){

            nextNumHunter = nextNumHunter - 2;

        }

        if(nextNumHunter * 2 > NowNumDeer){

            nextNumHunter = nextNumHunter - nextNumHunter * 0.2;

        }

        if (nextNumHunter < 0.){

            nextNumHunter = 0.;

        }



	    // DoneComputing barrier:
	    WaitBarrier();
        NowNumHunter = nextNumHunter;
	    //. . .

    	// DoneAssigning barrier:
    	WaitBarrier();
    	//. . .

    	// DonePrinting barrier:
    	WaitBarrier();
    	//. . .
    }


}


void Watcher(){
  
    
    ofstream data_output;
    data_output.open("data_output.csv");

    while( NowYear < 2027 )
    {
      
	    // compute a temporary next-value for this quantity
	    // based on the current state of the simulation:
	    //. . .

	    // DoneComputing barrier:
	    WaitBarrier();
	    //. . .

    	// DoneAssigning barrier:
    	WaitBarrier();
    	//. . .
        count_date++;
        if(NowMonth == 11){
            NowMonth = 0;
            NowYear++;
        }
        else{
            NowMonth++;
        }
        temperature_calculate();

        data_output << NowYear << "." << NowMonth + 1
                    //<< ","  << NowMonth + 1
                    << "," << count_date
                    << "," << ((5./9.) * (NowTemp - 32.))
                    << "," << NowPrecip * 2.54
                    << "," << NowHeight * 2.54
                    << ","  << NowNumDeer 
                    << "," << NowNumHunter << endl;


        
    	// DonePrinting barrier:
    	WaitBarrier();
    	//. . .
    }
    data_output.close();

}






int main(){

    NowMonth =    0;
    NowYear  = 2021;

    NowNumDeer = 1;
    NowHeight =  1.;
    NowNumHunter = 1;
    count_date = 0;

    InitBarrier(4);
    omp_set_num_threads( 4 );	// same as # of sections
    #pragma omp parallel sections
    {
	    #pragma omp section
	    {
		    Deer( );
	    }

	    #pragma omp section
	    {
	    	Grain( );
	    }

	    #pragma omp section
	    {
	    	Watcher( );
	    }

	    #pragma omp section
	    {
	    	Hunter( );	// your own
	    }
    }       // implied barrier -- all functions must return in order
	        // to allow any of them to get past her

}


