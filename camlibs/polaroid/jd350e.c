/* jd350e.c
 *
 * White balancing and brightness correction for the Jenoptik 
 * JD350 entrance camera
 *
 * Copyright (C) 2001 Michael Trawny <trawny99@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gphoto2-library.h>
#include <gphoto2-debug.h>
#include <stdlib.h>
#include <string.h>
#include "jd350e.h"
#include "jd350e_red.h"
//#include "jd350e_blue.h"

#define THRESHOLD 0xf8

#define RED(p,x,y,w) *((p)+3*((y)*(w)+(x))  )
#define GREEN(p,x,y,w) *((p)+3*((y)*(w)+(x))+1)
#define BLUE(p,x,y,w) *((p)+3*((y)*(w)+(x))+2)

#define SWAP(a,b) {unsigned char t=(a); (a)=(b); (b)=t;}

#define MINMAX(a,min,max) { (min)=MIN(min,a); (max)=MAX(max,a); }


int jd350e_postprocessing(int width, int height, unsigned char* rgb){
	int
		x,y,
		red_min=255, red_max=0, 
		blue_min=255, blue_max=0, 
		green_min=255, green_max=0;
	double
		min, max, amplify;

	/* reverse image row by row... */

	for( y=0; y<height; y++){
		for( x=0; x<width/2; x++ ){
			SWAP( RED(rgb,x,y,width), RED(rgb,width-x-1,y,width));
			SWAP( GREEN(rgb,x,y,width), GREEN(rgb,width-x-1,y,width));
			SWAP( BLUE(rgb,x,y,width), BLUE(rgb,width-x-1,y,width));
		}
	}

	/* determine min and max per color... */

	for( y=0; y<height; y++){
		for( x=0; x<width; x++ ){
			MINMAX( RED(rgb,x,y,width), red_min,   red_max  );
			MINMAX( GREEN(rgb,x,y,width), green_min, green_max);
			MINMAX( BLUE(rgb,x,y,width), blue_min,  blue_max );
		}
	}

	/* white balancing ...                               */
	/* here is still some work to do: either blue or red */
	/* pixles need to be emphasized.                     */
	/* but how can the driver decide?                    */

#if 0
	if( (green_max+blue_max)/2 > red_max ){
#endif
		/* outdoor daylight : red color correction curve*/
		gp_debug_printf(GP_DEBUG_LOW, "jd350e", "daylight mode");
		for( y=0; y<height; y++){
			for( x=0; x<width; x++ ){
				RED(rgb,x,y,width) = jd350e_red_curve[ RED(rgb,x,y,width) ];
			/* RED(rgb,x,y,width) = MIN(2*(unsigned)RED(rgb,x,y,width),255); */
			}
		}
		red_min = jd350e_red_curve[ red_min ];
		red_max = jd350e_red_curve[ red_max ];
		/* red_min = MIN(2*(unsigned)red_min,255); */
		/* red_max = MIN(2*(unsigned)red_max,255); */
#if 0
	}
	else if( (green_max+red_max)/2 > blue_max ){
		/* indoor electric light */
		gp_debug_printf(GP_DEBUG_LOW, "jd350e", "electric light mode");
		for( y=0; y<height; y++){
			for( x=0; x<width; x++ ){
				BLUE(rgb,x,y,width) = MIN(2*(unsigned)BLUE(rgb,x,y,width),255);
			}
		}
		blue_min = MIN(2*(unsigned)blue_min,255);
		blue_max = MIN(2*(unsigned)blue_max,255);
	}
#endif

	/* Normalize brightness ... */

	max = MAX( MAX( red_max, green_max ), blue_max);
	min = MIN( MIN( red_min, green_min ), blue_min);
	amplify = 255.0/(max-min);

	for( y=0; y<height; y++){
		for( x=0; x<width; x++ ){
			RED(rgb,x,y,width)= MIN(amplify*(double)(RED(rgb,x,y,width)-min),255);
			GREEN(rgb,x,y,width)= MIN(amplify*(double)(GREEN(rgb,x,y,width)-min),255);
			BLUE(rgb,x,y,width)= MIN(amplify*(double)(BLUE(rgb,x,y,width)-min),255);
		}
	}

	return GP_OK;
}
