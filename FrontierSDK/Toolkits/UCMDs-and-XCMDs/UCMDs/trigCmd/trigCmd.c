
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <math.h>
#include <ucmd.h>


#define sintoken	'sine'
#define costoken	'cos '
#define tantoken	'tan '
#define sinhtoken	'sinh'
#define coshtoken	'cosh'
#define tanhtoken	'tanh'
#define asintoken	'asin'
#define acostoken	'acos'
#define atantoken	'atan'
#define logtoken	'log '
#define log10token	'lg10'
#define exptoken	'exp '
#define abstoken	'abs '
#define sqrttoken	'sqrt'
#define ceiltoken	'ceil'
#define floortoken	'floo'
#define modtoken	'mod '


int errno; /*see math.c*/




static void sinverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
	
	x = sin (x);

	IACreturndouble (x);
	} /*sinverb*/
	

static void cosverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = cos (x);

	IACreturndouble (x);
	} /*cosverb*/
	

static void tanverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = tan (x);

	IACreturndouble (x);
	} /*tanverb*/
	

static void sinhverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = sinh (x);

	IACreturndouble (x);
	} /*sinhverb*/
	

static void coshverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = cosh (x);

	IACreturndouble (x);
	} /*coshverb*/
	

static void tanhverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = tanh (x);

	IACreturndouble (x);
	} /*tanhverb*/
	

static void asinverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = asin (x);

	IACreturndouble (x);
	} /*asinverb*/
	

static void acosverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = acos (x);

	IACreturndouble (x);
	} /*acosverb*/
	

static void atanverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = atan (x);

	IACreturndouble (x);
	} /*atanverb*/
	

static void logverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = log (x);

	IACreturndouble (x);
	} /*logverb*/
	

static void log10verb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = log10 (x);

	IACreturndouble (x);
	} /*log10verb*/
	

static void modverb (void) {
	
	double x, y;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	if (!IACgetdoubleparam ((OSType) 'dvsr', &y))
		return;
		
	x = fmod (x, y);

	IACreturndouble (x);
	} /*modverb*/
	

static void floorverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = floor (x);

	IACreturndouble (x);
	} /*floorverb*/
	

static void ceilverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = ceil (x);

	IACreturndouble (x);
	} /*ceilverb*/
	

static void sqrtverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = sqrt (x);

	IACreturndouble (x);
	} /*sqrtverb*/
	

static void absverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = fabs (x);

	IACreturndouble (x);
	} /*absverb*/
	

static void expverb (void) {
	
	double x;
	
	if (!IACgetdoubleparam ((OSType) keyDirectObject, &x))
		return;
		
	x = exp (x);

	IACreturndouble (x);
	} /*expverb*/
	

void UCMDmain (void) {
		
	switch (IACgetverbtoken ()) {
	
		case sintoken:
			sinverb (); break;
			
		case costoken:
			cosverb (); break;
			
		case tantoken:
			tanverb (); break;
		
		case sinhtoken:
			sinhverb (); break;
			
		case coshtoken:
			coshverb (); break;
			
		case tanhtoken:
			tanhverb (); break;
		
		case asintoken:
			asinverb (); break;
			
		case acostoken:
			acosverb (); break;
			
		case atantoken:
			atanverb (); break;
		
		case logtoken:
			logverb (); break;
			
		case log10token:
			log10verb (); break;
		
		case exptoken:
			expverb (); break;
			
		case abstoken:
			absverb (); break;
			
		case sqrttoken:
			sqrtverb (); break;
			
		case ceiltoken:
			ceilverb (); break;
			
		case floortoken:
			floorverb (); break;
			
		case modtoken:
			modverb (); break;
			
		default:
			IACnothandlederror (); break;
			
		} /*switch*/
	} /*UCMDmain*/


