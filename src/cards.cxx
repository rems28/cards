// cards.cxx
//
//  Copyright (c) 2014-2016 by NuTyX team (http://nutyx.org)
// 
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
//  USA.
//

#include <iostream>
#include <cstdlib>
#include "file_download.h"
#include "cards_base.h"
#include "cards_sync.h"
#include "cards_depends.h"
#include "cards_install.h"
#include "cards_remove.h"
#include "cards_create.h"
#include "cards_info.h"
#include "cards_upgrade.h"

#include "pkgrepo.h"
#include "pkginfo.h"
#include "pkgrm.h"

using namespace std;


int main(int argc, char** argv)
{
	string command = basename(argv[1]);
	setlocale(LC_ALL,"");
	bindtextdomain(GETTEXT_PACKAGE,PACKAGE_LOCALE_DIR);
	textdomain(GETTEXT_PACKAGE);

	string configFile = "/etc/cards.conf";
	try {
		CardsArgumentParser cardsArgPars;
		cardsArgPars.parse(argc, argv);

		if (cardsArgPars.isSet(CardsArgumentParser::OPT_CONFIG_FILE))
			configFile=cardsArgPars.getOptionValue(CardsArgumentParser::OPT_CONFIG_FILE);

		if (cardsArgPars.command() == CardsArgumentParser::CMD_HELP) {
			cout << _("Usage: ") << BLUE << cardsArgPars.appName()  << _(" command ") << NORMAL << "[options]"<< endl;

			cout << "where possible" << BLUE << _(" command ") << NORMAL<< "are:" << endl;

			cout << GREEN << "\nINFORMATION" << NORMAL << endl;
			cout << BLUE << "  help                       "
				<< NORMAL << _("show this help") << endl;
			cout << BLUE << "  list [-b][-p]              "
				<< NORMAL << _("list of installed packages") << endl
				<< "                             "
				<< _("Available binaries in depot.") << endl
				<< "                             "
				<< _("Available ports.") << endl;

			cout << BLUE << _("  info") << NORMAL << " [-b][-p] <package>    "
				<< _("print info about install package") << endl
				<< "                             "
				<< _("Available binaries in depot.") << endl
				<< "                             "
				<< _("Available ports.") << endl;
			cout << BLUE << "  config" << NORMAL << "                     "
				<< _("show the configuration of your ")
				<<  cardsArgPars.appName() << endl;
			cout << BLUE << "  level [-I]" << NORMAL << "                 "
				<< _("show all the ports founds. The list is showned by order of compilation")
				<< endl
				<< "                             "
				<< _("If -I it will ignore the WARNING about NOT FOUND <dependencies> from <port>")
				<< endl;
			cout << GREEN << "\nPORTS SPECIFIC SCENARIO" << NORMAL << endl;
			cout << BLUE << "  depends" << NORMAL << " [-i] <port>        "
				<< _("show dependencies for the port in compilation order.")
				<< endl
				<< "                             "
				<< _("If -i it will shows the installed dependencies as well.")
				<< endl;
			cout << BLUE << "  deptree" << NORMAL << "   <port>           "
				<< _("show dependencies in a tree.") << endl;
			cout << BLUE << "  depcreate" << NORMAL << " <port>           "
				<< _("compile and install the port and its dependencies.")
				<< endl;
			cout << BLUE << "  create" << NORMAL << "    <port>           "
				<< _("install all the dependencies from binaries and then compile the port.")
				<< endl;

			cout << GREEN << "\nDIFFERENCES / UPGRADE / CLEANUP" << NORMAL << endl;
			cout << BLUE << "  diff" << NORMAL << " [-p]                  "
				<< _("list outdated packages.") << endl
				<< "                             list outdated ports" << endl;
			cout << BLUE << "  upgrade" << NORMAL << "                    "
				<< _("upgrade outdated packages.") << endl;
			cout << BLUE << "  purge" << NORMAL << "                      "
				<< _("cleanup downloaded binaries in cache.") << endl;
			cout << GREEN << "\nSEARCHING" << NORMAL << endl;
			cout << BLUE << "  search" << NORMAL << " <expr>              "
				<< _("show port names or description containing 'expr'")
				<< endl;
			cout << BLUE << "  query" << NORMAL << "  <file>              "
				<< _("list owner of file(s) matching the query.") << endl;
			cout << BLUE << "  files" << NORMAL << "  <package>           "
				<< _("list the file(s) owned by the <package>") << endl;
			cout << GREEN << "\nSYNCHRONISATION" << NORMAL << endl;
			cout << BLUE << "  sync" << NORMAL << "                       "
				<< _("synchronize the local and remote meta datas.") << endl;
			cout << GREEN << "\nINSTALL, UPDATE and REMOVAL" << NORMAL << endl;
			cout << BLUE << "  install" << NORMAL << " [-u][-f] <package> "
				<< _("install the binary found on the mirror.") << endl
				<< "                             "
				<< _("If -u it will upgrade the installed package.") << endl
				<< "                             "
				<< _("If -f it will force the installation in case of files conflicts.") << endl;
			cout << BLUE << "  remove" << NORMAL << " [-a] <package>      "
				<< _("remove the installed package.") << endl
				<< "                             "
				<< _("If -a it will remove the sub-package as well.") << endl;
			cout << GREEN << "\nBASE SYSTEM" << NORMAL << endl;
			cout << BLUE << "  base" << NORMAL << " -r                    "
				<< _("return to a base system.") << endl
				<< "                             "
				<< _("You need to have a valid 'base' directory") << endl
				<< "                             "
				<< _("configured in the /etc/cards.conf file otherwise the command will abort.") << endl;
			return EXIT_SUCCESS;
		}
		if (cardsArgPars.command() == CardsArgumentParser::CMD_CONFIG) {
			Config config;
			Pkgrepo::parseConfig(configFile.c_str(), config);
			unsigned int index = 0;
			string prtDir, url ;

			for ( auto i : config.dirUrl ) {
				index++;
				cout << index << _(" Directory: ") << i.Dir ;
				if ( i.Url != "" )
					cout << _(" from ")
					<< i.Url ;
				cout << endl;
			}
			for ( auto i : config.baseDir )
				cout << _("Base System list directory: ") << i << endl;
			cout <<   _("Binaries : ")
				<< config.arch << endl;
			for ( auto i : config.locale ) cout << "Locale   : " << i << endl;
			if ( config.logdir != "") {
				cout << _("log directory: ")
					<< config.logdir << endl;
			}
			return EXIT_SUCCESS;
		} else if (cardsArgPars.command() == CardsArgumentParser::CMD_DEPCREATE) {
			// get the list of the dependencies
			CardsDepends CD(cardsArgPars);
			vector<string> listOfPackages = CD.getNeededDependencies();
			if ( listOfPackages.empty() ) {
				cout << _("The package ")
				<< cardsArgPars.otherArguments()[0]
				<< _(" is already installed") << endl;
				return EXIT_SUCCESS;
			}
			// create (compile and install) the List of deps (including the final package)
			Cards_create CC(cardsArgPars,configFile.c_str(),listOfPackages);
			return EXIT_SUCCESS;
		} else if (cardsArgPars.command() == CardsArgumentParser::CMD_CREATE) {

			if ( ( ! cardsArgPars.isSet(CardsArgumentParser::OPT_REMOVE)) &&
			( ! cardsArgPars.isSet(CardsArgumentParser::OPT_DRY)) ) {
				cardsArgPars.printHelp("create");
				return EXIT_SUCCESS;
			}
			if  ( ! cardsArgPars.isSet(CardsArgumentParser::OPT_DRY)) {
				// go back to a base system
				Cards_base CB(cardsArgPars);
				CB.run(argc, argv);
			}
			// get the list of the dependencies"
			CardsDepends CD(cardsArgPars);
			vector<string> listOfDeps = CD.getDependencies();

			if (!listOfDeps.empty())
				Cards_install CI(cardsArgPars,configFile.c_str(),listOfDeps);

			// compilation of the final port"
			Cards_create CC( cardsArgPars,
				configFile.c_str(),
				cardsArgPars.otherArguments()[0]);
			return EXIT_SUCCESS;

		} else if (cardsArgPars.command() == CardsArgumentParser::CMD_BASE) {
			if ( ( ! cardsArgPars.isSet(CardsArgumentParser::OPT_REMOVE)) &&
			( ! cardsArgPars.isSet(CardsArgumentParser::OPT_DRY)) ) {
				cardsArgPars.printHelp("base");
				return EXIT_SUCCESS;
			}
			Cards_base CB(cardsArgPars);
			CB.run(argc, argv);
			return EXIT_SUCCESS;
		} else if (cardsArgPars.command() == CardsArgumentParser::CMD_SYNC) {
			Cards_sync CS(cardsArgPars);
			CS.run();
			return EXIT_SUCCESS;
		} else if (cardsArgPars.command() == CardsArgumentParser::CMD_PURGE) {
			Cards_sync CS(cardsArgPars);
			CS.purge();
			return EXIT_SUCCESS;
		} else if ( (cardsArgPars.command() == CardsArgumentParser::CMD_UPGRADE) ||
					(cardsArgPars.command() == CardsArgumentParser::CMD_DIFF) ) {
			Cards_upgrade CU(cardsArgPars,configFile.c_str());
			return EXIT_SUCCESS;
		} else if (cardsArgPars.command() == CardsArgumentParser::CMD_INSTALL) {
				Cards_install CI(cardsArgPars,configFile.c_str());
			return EXIT_SUCCESS;
		} else if (cardsArgPars.command() == CardsArgumentParser::CMD_REMOVE) {
			Cards_remove CR("cards remove",cardsArgPars, configFile.c_str());
			return EXIT_SUCCESS;
		} else if (cardsArgPars.command() == CardsArgumentParser::CMD_LEVEL) {
			CardsDepends CD(cardsArgPars);
			CD.showLevel();
			return EXIT_SUCCESS;
		} else if (cardsArgPars.command() == CardsArgumentParser::CMD_DEPENDS) {
			CardsDepends CD(cardsArgPars);
			CD.showDependencies();
			return EXIT_SUCCESS;
		} else if (cardsArgPars.command() == CardsArgumentParser::CMD_DEPTREE) {
			CardsDepends CD(cardsArgPars);
			return CD.deptree();
		} else if ( (cardsArgPars.command() == CardsArgumentParser::CMD_INFO) ||
				(cardsArgPars.command() == CardsArgumentParser::CMD_LIST)   ||
				(cardsArgPars.command() == CardsArgumentParser::CMD_SEARCH) ||
				(cardsArgPars.command() == CardsArgumentParser::CMD_FILES)  ||
				(cardsArgPars.command() == CardsArgumentParser::CMD_QUERY)) {
			Cards_info CI(cardsArgPars,configFile.c_str());
			return EXIT_SUCCESS;
		}
	} catch (runtime_error& e) {
			cerr << "cards " << VERSION << " "<< command << ": " << e.what() << endl;
			return EXIT_FAILURE;
	}
}
// vim:set ts=2 :
