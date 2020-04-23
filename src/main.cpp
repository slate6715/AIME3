/****************************************************************************************
 * main (AIME3) - Primary reads in the command line and launches the MUD class
 *
 *
 ****************************************************************************************/

#include <iostream>
#include <getopt.h>
#include "MUD.h"

/*****************************************************************************************
 * displayHelp - Shows command line parameters to the user.
 *****************************************************************************************/

void displayHelp(const char *execname) {
   std::cout << execname << " [OPTIONS]\n";
	std::cout << "Start the AIME3 MUD server.\n\n";
   std::cout << "  -a, --address   IP address to bind the server to (default: 127.0.0.1 or defined in the config file)\n";
   std::cout << "  -p, --port      Port to bind the server to (default: 6715 or defined in the config file)\n";
	std::cout << "  -c, --config    Override the default config file location\n";
	std::cout << "  -h, --help      Display this message\n";
}


int main(int argc, char *argv[]) {

   // ****** Set up command line parameter defaults ******
	std::string configfile = "data/mud.conf";
	unsigned short cl_port = 0;
	long portval = 0;
	std::string cl_ip_addr;

	static struct option long_options[] = {
		{"address", required_argument, 0, 'a'},
		{"port", required_argument, 0, 'p'},
		{"config", required_argument, 0, 'c'},
		{"help", required_argument, 0, 'h'},
		{0,0,0,0}
		};

   // Get the command line arguments and set params appropriately
   int c = 0;
	int option_index = 0;
   while ((c = getopt_long(argc, argv, "c:a:p:", long_options, &option_index)) != -1) {
      switch (c) {

      // Override the default or config port number via command line argument
      case 'p':
         portval = strtol(optarg, NULL, 10);
         if ((portval < 1) || (portval > 65535)) {
            std::cout << "Invalid port on command line. Value must be between 1 and 65535";
            std::cout << "Format: " << argv[0] << " [<max_range>] [<max_threads>]\n";
            exit(0);
         }
         cl_port = (unsigned short) portval;
         break;

      // IP address to attempt to bind to
      case 'a':
         cl_ip_addr = optarg;
         break;

		// Point to another config file besides default
		case 'c':
			configfile = optarg;
			break;

      case 'h':
              displayHelp(argv[0]);
              break;

      default:
              std::cout << "Unknown command line option '" << c << "'\n";
              displayHelp(argv[0]);
              break;
      }

   }

	// Create the MUD object and start configuring it
	MUD engine;


	// Initialize the config file defaults, then read in the config file
	engine.initConfig();

	try {
		engine.loadConfig(configfile.c_str());
	}

	catch (const libconfig::FileIOException &fioex)
	{
		std::cerr << "I/O error reading config file: " << configfile << std::endl;
		return(EXIT_FAILURE);
	}
	catch (const libconfig::ParseException &pex)
	{
		std::cerr << "Parse error in " << pex.getFile() << ", line: " << pex.getLine()
								<< " - " << pex.getError() << std::endl;
		return(EXIT_FAILURE);
	}

	engine.initialize();

	try {
		engine.bootServer();

	}
	catch (const socket_error &e) {
		std::cerr << "Error bringing listening socket online: " << e.what() << std::endl;
		return(EXIT_FAILURE);
	}

	// Primary MUD loop
	engine.startListeningThread();

	// Primary MUD loop
	engine.runMUD();

	// If we have gotten to this point, then the MUD is shutting down. Clean up
	engine.cleanup();

   return 0;
}
