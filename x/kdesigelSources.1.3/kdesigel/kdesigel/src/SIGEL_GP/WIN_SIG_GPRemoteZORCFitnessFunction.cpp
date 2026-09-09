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

#include "SIGEL_GP/WIN_SIG_GPRemoteZORCFitnessFunction.h"
#include <QInputDialog>   // widget used in this file only
#include "SIGEL_Tools/SIG_IO.h"


namespace SIGEL_GP
{

   SIG_GPRemoteZORCFitnessFunction::SIG_GPRemoteZORCFitnessFunction( SIGEL_Program::SIG_Program &program,
								  SIGEL_Robot::SIG_Robot &robot,
								  SIGEL_Environment::SIG_Environment &environment,
								  SIGEL_Simulation::SIG_SimulationParameters & simulationParameters )
    : SIG_GPFitnessFunction( program,
			     robot,
			     environment,
			     simulationParameters,
			     "RemoteZORCFitnessFunction" )
   {  // nothing
	   fWaitingOnRead = false;
	   FillMemory(&osReader, sizeof(OVERLAPPED), 0);
	   osReader.hEvent = CreateEvent(NULL, true, false, NULL);

	   if(osReader.hEvent == NULL){
			SIGEL_Tools::SIG_IO::cerr << "Creation error: Couldn't create event for overlapped communication. Program evaluation was canceled." << Qt::endl;
			fprintf(stderr, "Cannot create com-event.");
      }

   }


   SIG_GPRemoteZORCFitnessFunction::~SIG_GPRemoteZORCFitnessFunction()
   {  // nothing
	   CloseHandle(osReader.hEvent);
   }


   double SIG_GPRemoteZORCFitnessFunction::evalFitness()
   {  double   fitness  = 0,
               distance = 0;
      QString  expInstruct,
               dlogTitle,
               prgLines,
               langParmLines;
      bool     wasOk = false;
      long     BaudRate = 115200;
	  HANDLE	fd;
	   int      timeToRun,
               recvPrgLen = 0;
      char     serTxt[48] = "-";


      // make a textstream object from langParmLines to get the language parameters in the string
      QTextStream lpTs( &langParmLines, QIODeviceBase::WriteOnly );

      // fetch current program as string
      program.printToString(prgLines);

      // fetch language parameters
      rob.getLangParam()->writeToFileTransfer(lpTs);

   	  // Open the serial device (might differ on your computer !)
	  char gszPort[9] = "COM1";
	  fd = CreateFile((unsigned short*)gszPort, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
      if (fd == INVALID_HANDLE_VALUE)
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
		unsigned long cntWritten;
		WriteFile(fd, "4", 1, &cntWritten, &osReader);

      if (! timedSerialWait(fd, 2) )
      {
         // ZORC didn't respond for 2 seconds -- not connected ?!
         SIGEL_Tools::SIG_IO::cerr << "Serial Connection Failed: Connection to ZORC can't be established using interface \"/dev/modem\". The RS-232 Ping failed. Possibly the menu on ZORC wasn't reset to the toplevel ? Please check !" << Qt::endl;
         fprintf(stderr, "Can't establish connection to ZORC -- RS232-Ping failed");

         return -1.0;
      }

		// check PING response from ZORC
	  unsigned long cntRead;
      ReadFile(fd, &serTxt, 8, &cntRead, &osReader);
	  WaitForSingleObject(osReader.hEvent, INFINITE);
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
      ReadFile(fd, &serTxt, 32, &cntRead, &osReader);
	  WaitForSingleObject(osReader.hEvent, INFINITE);
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
	Sleep(100);

      // Exit d/l section and prepare to run program on ZORC
		goZORCMenu(fd, '4');
		goZORCMenu(fd, '2');
	   Sleep(100);

      // while ZORC is waiting we can transmit the time to run;
      // first get time to evaluate on ZORC
      timeToRun = QTime( 0, 0 ).secsTo( simparameter.getTimeToSimulate() );
      sprintf(serTxt, "%d\n", timeToRun);
      WriteFile(fd, serTxt, strlen(serTxt), &cntWritten, &osReader);

      // that's it -- close the serial device
      CloseHandle(fd);

      // ask for the fitness/distance
      dlogTitle = "Evaluating current Program on ZORC";
      expInstruct = "<B><BIG>Current program was successfully downloaded to ZORC.<BR> \
                     Let's start the evaluation:</B></BIG><BR> \
                     <OL><LI>Set ZORC down to start executing the current program</LI><LI>Enter the distance travelled by ZORC</LI></OL> \
                     <BR><BR><I>Execute the program as many times as you want by restarting the program manually on ZORC</I> <BR><BR><BR>\
                     Enter the distance travelled by the robot (centimeters, 1 decimal):";
      distance = QInputDialog::getDouble(dlogTitle, expInstruct, 1.0, 0, 500.0, 1, &wasOk);

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
   int SIG_GPRemoteZORCFitnessFunction::SetSerial(HANDLE fd, long baud, int handshake)
   {
      int baudcode;
	  DCB dcb = {0};

      /* read the current settings */
	  FillMemory(&dcb, sizeof(dcb), 0);
      if(!GetCommState(fd, &dcb)){
		  fprintf(stderr, "SIG_GPRemoteZORCFitnessFunction::SetSerial() -- Couldn't get CommState\n");
		  return -1;
	  }

      switch (baud)
      {
         case   2400: 	baudcode = CBR_2400; 	break;
         case   9600: 	baudcode = CBR_9600; 	break;
         case  19200: 	baudcode = CBR_19200; 	break;
         case  38400: 	baudcode = CBR_38400; 	break;
         case  57600: 	baudcode = CBR_57600; 	break;
         case 115200: 	baudcode = CBR_115200; 	break;

         default	  :	fprintf(stderr, "SIG_GPRemoteZORCFitnessFunction::SetSerial() -- Illegal baud rate\n");
				            return -1;
      }

	  // Baud rate, 8N1, ignore control lines, enable receive; 
	  // Flow control if needed
	  dcb.BaudRate = baudcode;		// baudrate
	  dcb.ByteSize = 8;				// 8Bit char size |
	  dcb.StopBits = ONESTOPBIT;	// 1 stop bit	  |-> 8N1
	  
      if (handshake){
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
		dcb.fOutxCtsFlow= true;
	  }

	  // ignore frame/parity errors
	  dcb.fParity = NOPARITY;

      // blocking read until 1 char arrives
	  COMMTIMEOUTS timeOuts;
	  FillMemory(&timeOuts, sizeof(timeOuts), 0);
	  SetCommTimeouts(fd, &timeOuts);

      // Apply the settings
	  FlushFileBuffers(fd);		// flush output buffer; ensures that all pending write
								// operations are transmitted (equals TCSADRAIN on Linux)
								// before changing the serial comm settings
	  if(!SetCommState(fd, &dcb))	// sets the new settings
      {  fprintf(stderr, "Serial initialization -- tcsetattr() -- failed");
         return -1;
      }

	  // set event mask for the serial port to notice incoming data (without polling the port)
	  DWORD dwStoredFlags;
	  dwStoredFlags = EV_RXCHAR | EV_RXFLAG;	// detect new char and new event char
	  if(!SetCommMask(fd, dwStoredFlags)){
		  fprintf(stderr, "Serial initialization failed -- Couldn't set CommEventMask");
		  return -1;
	  }

      return (int) fd;
   }


   /**
    * This method sends all data contained in the QString object over the
    * serial interface specified by the 'serIF' file descriptor.
    * The method returns when all data has been sent and the serial buffer is empty.
    */
   void SIG_GPRemoteZORCFitnessFunction::sendOverSerialLine(HANDLE serIF, const QString &txtToSend)
   {  unsigned char  c,
                     prev = '-';
      int   idx = 0;

      // send all data
      while ((const char)txtToSend[idx] != 0)
      {
         // put next char to serial interface
         c = (const char)txtToSend[idx];
         idx++;

         // prevent double newlines..
		 unsigned long cntWritten;
         if ((prev != '\n') || (c != '\n'))
         {  WriteFile(serIF, &c, 1, &cntWritten, &osReader);
         }
         prev = c;
      }

      // wait for transmission to be completed, i.e. all data left the buffer
	  FlushFileBuffers(serIF);
   }


   /**
    * Wait for input on the specified serial device for 'timeOutSecs' number of seconds.
    * Returns 'true' when data is pending, 'false' otherwise.
    */
   bool SIG_GPRemoteZORCFitnessFunction::timedSerialWait(HANDLE serIF, int timeOutSecs)
   {  
	   DWORD dwCommEvent, result;

	   WaitCommEvent(serIF, &dwCommEvent, &osReader);
	   result = WaitForSingleObject(osReader.hEvent, timeOutSecs * 1000);
	   if(result == WAIT_TIMEOUT)
		   return false;

	   return true;
   }

	/**
    * This method goes one step into the ZORC menu hierarchy.
    */
	void SIG_GPRemoteZORCFitnessFunction::goZORCMenu(HANDLE serIF, char inChoice)
	{
		unsigned long cntWritten;
		WriteFile(serIF, (LPCVOID)&inChoice, 1, &cntWritten, &osReader);
		Sleep(100);
	}
}
