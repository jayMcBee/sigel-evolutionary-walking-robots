/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler

  This file is part of Sigel.

  Sigel is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Sigel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Sigel; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "SIGEL_GP/SIG_GPRemoteZORCFitnessFunction.h"
#include <QInputDialog>   // widget used in this file only
#include "SIGEL_Tools/SIG_DialogParent.h"
#include "SIGEL_Tools/SIG_IO.h"


namespace SIGEL_GP
{

   SIG_GPRemoteZORCFitnessFunction::~SIG_GPRemoteZORCFitnessFunction()
   {  // nothing
   }


   double SIG_GPRemoteZORCFitnessFunction::evalFitness( SIGEL_Program::SIG_Program &program,
                                                        SIGEL_Robot::SIG_Robot &rob,
                                                        SIGEL_Environment::SIG_Environment &,
                                                        SIGEL_Simulation::SIG_SimulationParameters &simparameter )
   {  double   fitness  = 0,
               distance = 0;
      QString  expInstruct,
               dlogTitle,
               prgLines,
               langParmLines;
      bool     wasOk = false;
      long     BaudRate = 115200;
	   int      fd,
               timeToRun,
               recvPrgLen = 0;
      char     serTxt[48] = "-";


      // make a textstream object from langParmLines to get the language parameters in the string
      QTextStream lpTs( &langParmLines, QIODeviceBase::WriteOnly );

      // fetch current program as string
      program.printToString(prgLines);

      // fetch language parameters
      rob.getLangParam()->writeToFileTransfer(lpTs);

   	// Open the serial device (might differ on your computer !)
      fd = open("/dev/modem", O_RDWR | O_NOCTTY);
      if (fd < 0)
      {  SIGEL_Tools::SIG_IO::cerr << "Serial Device can't be opened: The serial device \"/dev/modem\" couldn't be opened to transmit the program to ZORC. Program evaluation was canceled." << Qt::endl;
         fprintf(stderr, "Serial interface can't be opened");
         return -1.0;
      }

      // set parameters for serial device and catch errors
	   if (SetSerial(fd, BaudRate, 1) < 0)
      {  SIGEL_Tools::SIG_IO::cerr << "Error setting Parameters of Serial Device: Failed to set the parameters of serial device \"/dev/modem\". Program evaluation was canceled." << Qt::endl;
         fprintf(stderr, "Failed to set parameters of serial device");
         return -1.0;
      }

		// now check connection to ZORC using the RS232-Ping !
		goZORCMenu(fd, '3');
		goZORCMenu(fd, '1');
		goZORCMenu(fd, '3');
   	write(fd, "4", 1);

      if (! timedSerialWait(fd, 2) )
      {
         // ZORC didn't respond for 2 seconds -- not connected ?!
         SIGEL_Tools::SIG_IO::cerr << "Serial Connection Failed: Connection to ZORC can't be established using interface \"/dev/modem\". The RS-232 Ping failed. Possibly the menu on ZORC wasn't reset to the toplevel ? Please check !" << Qt::endl;
         fprintf(stderr, "Can't establish connection to ZORC -- RS232-Ping failed");

         return -1.0;
      }

		// check PING response from ZORC
      read(fd, &serTxt, 8);
      if ( sscanf(serTxt, "%d", &recvPrgLen) == 1)
		{
			if (recvPrgLen != 8)
			{	// bad response from ZORC..
         	SIGEL_Tools::SIG_IO::cerr << "Problem with Serial Connection: The connection to ZORC could be established but the response was incorrect. Please check !" << Qt::endl;
	         fprintf(stderr, "The connection to ZORC could be established but the response was incorrect.");
   	      return -1.0;
			}

			// go back to main menu
			goZORCMenu(fd, '4');
			goZORCMenu(fd, '4');
		}

      else
      {  SIGEL_Tools::SIG_IO::cerr << "Bad Response from ZORC: Got bad response from ZORC. Evaluation canceled, Please retry.." << Qt::endl;
         fprintf(stderr, "Bad response from ZORC");
         return -1.0;
      }

      // remote control ZORC and download program + parameters
   	// Go to program-download menu
		goZORCMenu(fd, '1');
		goZORCMenu(fd, '1');

      // download program and language parameters;
      sendOverSerialLine(fd, prgLines);

      // wait for response from ZORC
      if (! timedSerialWait(fd, 2) )
      {
         // ZORC didn't respond for 2 seconds -- not connected ?!
         SIGEL_Tools::SIG_IO::cerr << "Serial Connection Failed: Connection to ZORC can't be established using interface \"/dev/modem\". Evaluation canceled, Please check !" << Qt::endl;
         fprintf(stderr, "Can't establish connection to ZORC");
         return -1.0;
      }

		// response, should contain program length !
      read(fd, &serTxt, 32);
      if ( sscanf(serTxt, "%d", &recvPrgLen) == 1)
      {
         // compare original length and length reported by ZORC
         if (recvPrgLen != program.getProgramLength())
         {  char  errMsg[256];

            sprintf(errMsg, "<BIG><B>Program length received by ZORC [%d] doesn't match the original SIGEL-Program length [%d].</B></BIG><BR><BR>Please retry..", recvPrgLen, program.getProgramLength());
            SIGEL_Tools::SIG_IO::cerr << "Error transmitting program: " << errMsg << Qt::endl;
            fprintf(stderr, errMsg);
            return -1.0;
         }
      }

      else
      {  SIGEL_Tools::SIG_IO::cerr << "Bad Response from ZORC: Got bad response from ZORC. Bad response after program transmission. Evaluation was canceled, Please retry.." << Qt::endl;
         fprintf(stderr, "Bad response from ZORC");
         return -1.0;
      }

   	// Go to LangParm-download menu
		goZORCMenu(fd, '2');
   	sendOverSerialLine(fd, langParmLines);
	   usleep(100 * 1000);

      // Exit d/l section and prepare to run program on ZORC
		goZORCMenu(fd, '4');
		goZORCMenu(fd, '2');
	   usleep(100 * 1000);

      // while ZORC is waiting we can transmit the time to run;
      // first get time to evaluate on ZORC
      timeToRun = QTime( 0, 0 ).secsTo( simparameter.getTimeToSimulate() );
      sprintf(serTxt, "%d\n", timeToRun);
      write(fd, serTxt, strlen(serTxt));

      // that's it -- close the serial device
      close(fd);

      // ask for the fitness/distance
      dlogTitle = "Evaluating the Current Program on ZORC";
      expInstruct = "<B><BIG>The current program was successfully downloaded to ZORC.<BR> \
                     Let's start the evaluation:</BIG></B><BR> \
                     <OL><LI>Set ZORC down to start executing the current program</LI><LI>Enter the distance travelled by ZORC</LI></OL> \
                     <BR><BR><I>Execute the program as many times as you want by restarting the program manually on ZORC</I> <BR><BR><BR>\
                     Enter the distance travelled by the robot (centimeters, 1 decimal):";
      distance = QInputDialog::getDouble(SIGEL_Tools::dialogParent(), dlogTitle, expInstruct, 1.0, 0, 500.0, 1, &wasOk);

      // something went wrong, set fitness to -1 -> will be re-evaluated by SIGEL
      if (!wasOk)
      {  return -1.0;
      }

      // calculate fitness using fitness and time to run
      fitness = distance / timeToRun;

      fprintf(stderr, "distance: %4.3f | fitness: %5.4f\n", distance, fitness);
      return fitness;
   }


   /**
   * Set some parameters of the serial device for proper operation with ZORC.
   * Returns -1 in case any error occurred.
   */
   int SIG_GPRemoteZORCFitnessFunction::SetSerial(int fd, long baud, int handshake)
   {
      int baudcode;
      struct termios tio;

      /* read the current settings */
      tcgetattr(fd, &tio);

      switch (baud)
      {
         case   2400: 	baudcode = B2400; 	break;
         case   9600: 	baudcode = B9600; 	break;
         case  19200: 	baudcode = B19200; 	break;
         case  38400: 	baudcode = B38400; 	break;
         case  57600: 	baudcode = B57600; 	break;
         case 115200: 	baudcode = B115200; 	break;

         default	  :	fprintf(stderr, "SIG_GPRemoteZORCFitnessFunction::SetSerial() -- Illegal baud rate\n");
				            return -1;
      }

		// Baud rate, 8N1, ignore control lines, enable receive; flow control if needed
      tio.c_cflag = baudcode | CS8 | CLOCAL | CREAD;
      if (handshake) tio.c_cflag |= CRTSCTS;

		// ignore frame/parity errors
      tio.c_iflag = IGNPAR;

      // Raw output
      tio.c_oflag = 0;

      // No echo
      tio.c_lflag = 0;

      // blocking read until 1 char arrives
      tio.c_cc[VMIN] = 1;
      tio.c_cc[VTIME] = 0;

      // Apply the settings
      if (tcsetattr(fd, TCSADRAIN, &tio) < 0)
      {  fprintf(stderr, "Serial initialization -- tcsetattr() -- failed");
         return -1;
      }

      return fd;
   }


   /**
    * This method sends all data contained in the QString object over the
    * serial interface specified by the 'serIF' file descriptor.
    * The method returns when all data has been sent and the serial buffer is empty.
    */
   void SIG_GPRemoteZORCFitnessFunction::sendOverSerialLine(int serIF, const QString &txtToSend)
   {	unsigned char  c,
                     prev = '-';
      int   idx = 0;

      // send all data
      while (txtToSend[idx].toLatin1() != 0)
      {
         // put next char to serial interface
         c = txtToSend[idx].toLatin1();
         idx++;

         // prevent double newlines..
         if ((prev != '\n') || (c != '\n'))
         {  write(serIF, &c, 1);
         }
         prev = c;
      }

      // wait for transmission to be completed, i.e. all data left the buffer
      tcdrain(serIF);
   }


   /**
    * Wait for input on the specified serial device for 'timeOutSecs' number of seconds.
    * Returns 'true' when data is pending, 'false' otherwise.
    */
   bool SIG_GPRemoteZORCFitnessFunction::timedSerialWait(int serIF, int timeOutSecs)
   {  fd_set rfds;
      struct timeval tv;
      int retval;

      // Watch the device file associated with the serial interface
      FD_ZERO(&rfds);
      FD_SET(serIF, &rfds);

      // Wait up to <timeOutSecs> seconds.
      tv.tv_sec  = timeOutSecs;
      tv.tv_usec = 0;

      retval = select(serIF+1, &rfds, NULL, NULL, &tv);

      // retval tells us if data is available !
      if (retval)
      {  return true;
      }
      else
      {  return false;
      }
   }

	/**
    * This method goes one step into the ZORC menu hierarchy.
    */
	void SIG_GPRemoteZORCFitnessFunction::goZORCMenu(int serIF, char inChoice)
	{
   	write(serIF, (unsigned char *)&inChoice, 1);
	   usleep(100 * 1000);
	}
}
