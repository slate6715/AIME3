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
	std::string configfile = "mud.conf";	
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



/*
   if (simdata_file.size() == 0) {
      std::cerr << "You must specify the sim_data inject database file.\n";
      displayHelp(argv[0]);
      exit(0);
   }

   DronePlotDB db;

   // Kick off the simulation thread by creating the sim management object
   // This will raise a runtime_exception if the simdata database load fails
   AntennaSim sim(db, simdata_file.c_str(), time_mult, verbosity);

   // Launch the thread
   pthread_t simthread;
   if (pthread_create(&simthread, NULL, t_simulator, (void *) &sim) != 0)
      throw std::runtime_error("Unable to create simulator thread");

   // Start the replication server
   ReplServer repl_server(db, ip_addr.c_str(), port, time_mult, verbosity); 

   pthread_t replthread;
   if (pthread_create(&replthread, NULL, t_replserver, (void *) &repl_server) != 0)
      throw std::runtime_error("Unable to create replication server thread");

   // Sleep the duration of the simulation
   sleep(sim_time / time_mult);

   // Stop the replication server
   repl_server.shutdown();

   // Stop the thread
   sim.terminate();

   // Wait until the thread has exited
   pthread_join(simthread, NULL);
   pthread_join(replthread, NULL);

   // Write the replication database to a CSV file
   std::cout << "Writing results to: " << outfile << "\n";
   db.sortByTime();
   db.writeCSVFile(outfile.c_str());
   */
   return 0;
}
