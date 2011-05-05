/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* This program sends a (bsetvel rotvel transvel) to the ViRbot program */


#include <stdio.h>
#include "msocket.h"
#include <unistd.h>
#include <signal.h>
#include <jsw.h>
#include <stdlib.h>
#include <string.h>

#define DEVICE "/dev/input/js0"
#define CALIB	"~/.joystick"

#define BUFFERSIZE 1024

static int runlevel;
static void MySignalHandler (int s);

msocketData outport;

static void
MySignalHandler (int s)
{
   switch (s) {
      case SIGINT:
         /* Set runlevel to 1, causing the main while() loop to break. */
	 runlevel = 1;
	 break;
   }
}

int
main (int argc, char *argv[])
{
   int outportnum, with_clips;
   int i, status, buttonState, error;
   
   int X, Y, pressed_Button_1 = 0;


   float axisState_X;
   float axisState_Y;
   float speed_mul;  /* to increase/decrease the speed */
   float transVel;
   float rotVel;
   char buffer[BUFFERSIZE];
   char realhost[BUFFERSIZE];
   /* Default device and calibration file paths. */
   const char *device = JSDefaultDevice;
   const char *calib = JSDefaultCalibration;

  /* Parse arguments, first argument is the joystick device name
   * and the second is the calibration file name. Did we get
   * enough arguments?
   */
   if (argc < 6) {
      /* Use default joystick device and calibration file
       * * but also print usage.
       * */
      printf ("Usage: tx8_js <DEVICE> <CONFIG> <IP> <PORT> <clips format 1/0>\n");
      exit (1);
   }

  /* Fetch device and calibration file names from input. */
  device = argv[1];
  calib = argv[2];
  sscanf (argv[4], "%d", &outportnum);
  sscanf (argv[5], "%d", &with_clips);
  
  /* This option will send the data as
   *  "(bsetvel rotvel transvel)" if 1
   *  or "rotvel transvel" if 0
   */

  if(with_clips != 1 && with_clips != 0) {
     printf("The option clips format must be 1 or 0\n");
     exit(1);
  }

  if (error = msocketGetIpAdd (argv[3], buffer, realhost)) {
     printf ("error reading ip addr: %s", msocketGetError (error));
     exit (1);
  }

  if (error = outport.msocketInitSocket (argv[3], outportnum, TX_SOCKET)) {
     printf ("error opening server: %s", msocketGetError (error));
     exit (1);
  }


  /* Joystick data structure. */
  js_data_struct jsd;

  /* Set up signal handler to catch the interrupt signal. */
  signal (SIGINT, MySignalHandler);

  /* Initialize (open) the joystick, passing it the pointer to
   * the jsd structure (it is safe to pass an uninitialized jsd
   * structure as seen here).
   */


  status = JSInit (&jsd, device, calib, JSFlagNonBlocking);

  /* Error occured opening joystick? */
  if (status != JSSuccess) {
     /* There was an error opening the joystick, print by the
      * recieved error code from JSInit().
      */
      switch (status) {
         case JSBadValue:
	    fprintf (stderr,"%s: Invalid value encountered while initializing joystick.\n",device);
	 break;

         case JSNoAccess:
	    fprintf (stderr,"%s: Cannot access resources, check permissions and file locations.\n", device);
	    break;
	 
	 case JSNoBuffers:
	    fprintf (stderr, "%s: Insufficient memory to initialize.\n",device);
	    break;
	 
	 default:/* JSError */
	    fprintf (stderr, "%s: Error occured while initializing.\n", device);
         break;
      }

      /* Print some helpful advice. */
      fprintf (stderr, "Make sure that:\n\
							1. Your joystick is connected (and turned on as needed).\n\
							2. Your joystick modules are loaded (`modprobe <driver>').\n\
							3. Your joystick needs to be calibrated (use `jscalibrator').\n\
							\n");

      /* Close joystick jsd structure (just in case). */
      JSClose (&jsd);
      return (1);

   } else {
      /* Opened joystick successfully. */
      printf ("Reading values from `%s' (press CTRL+C to interrupt).\n",device);
   }

  /* Set runlevel to 2 and begin the main while() loop. Here we
   * will print the values of each axis and button as they
   * change.
   */
   runlevel = 2;
   
   while (runlevel >= 2) {
      /* Fetch for event, JSUpdate() will return JSGotEvent
       * if we got an event.
       */
      if (JSUpdate (&jsd) == JSGotEvent) {
	 /* Got an event, now print the values to reflect the
	  * change in axises and/or buttons.
	  */
         printf ("\r");
	 /* Print value of each axis. */
	 for (i = 0; i < jsd.total_axises; i++) {
	    printf ("A%i:%.2f ", i, JSGetAxisCoeffNZ (&jsd, i));
	    
	    switch (i) {
	       /* X */
	       case 0:
		   rotVel = -1 * JSGetAxisCoeffNZ (&jsd, i);
		  break;
		  /* Y */
	       case 1:
		  axisState_Y = JSGetAxisCoeffNZ (&jsd, i);
		  transVel = axisState_Y;
		  break;
	       case 2:
		  /* the max speed is 40 cm/s */
		  speed_mul = 40 * JSGetAxisCoeffNZ (&jsd, i);
		  if(speed_mul < 0)
		     speed_mul = 0;
		  break;
		  
	    }
	 }
	 //printf(" ");
	 /* Print state of each button. */
	 for (i = 0; i < jsd.total_buttons; i++) {
	    switch (i) {
	       case 0:
		  buttonState = JSGetButtonState (&jsd, i);
		  if (buttonState != 0) {
		     printf ("Botón 0\n");
		  }
		  break;
	       case 1:
		  buttonState = JSGetButtonState (&jsd, i);
		  if (buttonState != 0) {
		     printf ("Botón 1\n");
		     pressed_Button_1 = 1; 
		  }
		  break;
	       case 2:
		  buttonState = JSGetButtonState (&jsd, i);
		  if (buttonState != 0) {
		     printf ("Botón 2\n");
		  }
		  break;
	       case 3:
		  buttonState = JSGetButtonState (&jsd, i);
		  if (buttonState != 0) {
		     printf ("Botón 3\n");
		  }
		  break;
	       case 4:
		  buttonState = JSGetButtonState (&jsd, i);
		  if (buttonState != 0) {
		     printf ("Botón 4\n");
		  }
		  break;
	       case 5:
		  buttonState = JSGetButtonState (&jsd, i);
		  if (buttonState != 0) {
		     printf ("Botón 5\n");
		  }
		  break;
	       case 6:
		  buttonState = JSGetButtonState (&jsd, i);
		  if (buttonState != 0) {
		     printf ("Botón 6\n");
		  }
		  break;
	       case 7:
		  buttonState = JSGetButtonState (&jsd, i);
		  if (buttonState != 0) {
		     printf ("Botón 7\n");
		  }
		  break;
	    }
	    //printf("B%i:%i ",i, buttonState);
	 }

         
	 if(pressed_Button_1 == 1 && with_clips == 1) {
	    //sprintf(buffer, "(bsetvel %2.2f %2.2f)", rotVel, transVel * speed_mul);
	    sprintf(buffer, "(bsetvel %2.2f %2.2f)", rotVel, 40*transVel);
	    outport.msocketTxData (buffer, BUFFERSIZE);
	    memset(buffer, 0, BUFFERSIZE);
	 }
         	 
	 if(pressed_Button_1 == 1 && with_clips == 0) {
	    //sprintf(buffer, "%2.2f %2.2f", rotVel, transVel * speed_mul);
	    sprintf(buffer, "%2.2f %2.2f", rotVel, 40*transVel);
	    outport.msocketTxData (buffer, BUFFERSIZE);
	    memset(buffer, 0, BUFFERSIZE);
	 }

	 
	 pressed_Button_1 = 0;
	 /* That's it, besure to flush output stream. */
	 fflush (stdout);
      }

      
#ifdef __MSW__
#else
      usleep (99000);			/* Don't hog the cpu. */
#endif
   }
   /* Close the joystick, now jsd may no longer be used after
    * * this call to any other JS*() calls except JSInit() which
    * * would initialize it again.*/
   printf ("\nClosing joystick...\n");
   JSClose (&jsd);
   printf ("Done.\n");
   outport.msocketKillSocket ();
   return (0);
}


