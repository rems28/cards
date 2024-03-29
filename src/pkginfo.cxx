//
//  pkginfo.cxx
// 
//  Copyright (c) 2000 - 2005 Per Liden
//  Copyright (c) 2006 - 2013 by CRUX team (http://crux.nu)
//  Copyright (c) 2013 - 2022 by NuTyX team (http://nutyx.org)
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

#include "pkginfo.h"

using namespace std;

Pkginfo::Pkginfo(const std::string& commandName)
	: Pkgdbh(commandName),
	m_runtimedependencies_mode(0),
	m_footprint_mode(0),
	m_archiveinfo(0),
	m_installed_mode(0),
	m_list_mode(0),
	m_owner_mode(0),
	m_details_mode(0),
	m_libraries_mode(0),
	m_runtime_mode(0),
	m_epoc(0),
	m_fulllist_mode(false)
{
}
Pkginfo::Pkginfo()
	:Pkgdbh("pkginfo"),
	m_runtimedependencies_mode(0),
	m_footprint_mode(0),
	m_archiveinfo(0),
	m_installed_mode(0),
	m_list_mode(0),
	m_owner_mode(0),
	m_details_mode(0),
	m_libraries_mode(0),
	m_runtime_mode(0),
	m_epoc(0),
	m_fulllist_mode(true)
{
}
void Pkginfo::run(int argc, char** argv)
{
	parseArguments(argc,argv);
	run();
}
void Pkginfo::parseArguments(int argc, char** argv)
{
	for (int i = 1; i < argc; ++i) {
		string option(argv[i]);
		if (option == "-r" || option == "--root") {
			assertArgument(argv, argc, i);
			m_root = argv[i + 1];
			i++;
		} else if (option == "-i" || option == "--installed") {
			m_installed_mode += 1;
		} else if (option == "-d" || option == "--details") {
			assertArgument(argv, argc, i);
			m_details_mode +=1;
			m_arg = argv[i + 1];
			i++;
		} else if (option == "-l" || option == "--list") {
			assertArgument(argv, argc, i);
			m_list_mode += 1;
			m_arg = argv[i + 1];
			i++;
		} else if (option == "-R" || option == "--runtimedep") {
			assertArgument(argv, argc, i);
			m_runtime_mode += 1;
			m_arg = argv[i + 1];
			i++;
		} else if (option == "-o" || option == "--owner") {
			assertArgument(argv, argc, i);
			m_owner_mode += 1;
			m_arg = argv[i + 1];
			i++;
		} else if (option == "-a" || option == "--archive") {
			assertArgument(argv, argc, i);
			m_archiveinfo += 1;
			m_arg = argv[i + 1];
			i++;
		} else if (option == "-f" || option == "--footprint") {
			assertArgument(argv, argc, i);
			m_footprint_mode += 1;
			m_arg = argv[i + 1];
			i++;
		} else if (option == "-L" || option == "--libraries") {
			assertArgument(argv, argc, i);
			m_libraries_mode +=1;
			m_arg =  argv[i + 1];
			i++;
		} else if (option == "--runtimedepfiles") {
			assertArgument(argv, argc, i);
			m_runtimedependencies_mode +=1;
			m_arg = argv[i + 1];
			i++;
		} else if (option == "-b" || option == "--buildtime") {
			assertArgument(argv, argc, i);
			m_epoc +=1;
			m_arg = argv[i + 1];
			i++; 
		} else {
			m_actualError = INVALID_OPTION;
			treatErrors(option);
		}
		if (m_root.empty())
			m_root="/";
		else
			m_root=m_root+"/";

	}

	m_packageArchiveName = m_arg;

	if (m_runtimedependencies_mode + m_footprint_mode + m_details_mode +
	m_installed_mode + m_list_mode + m_owner_mode + m_epoc + m_archiveinfo +
	m_footprint_mode + m_libraries_mode + m_runtime_mode == 0)
	{
		m_actualError = OPTION_MISSING;
		treatErrors(m_arg);
	}
	if (m_runtimedependencies_mode + m_footprint_mode + m_installed_mode + m_archiveinfo + m_list_mode + m_owner_mode > 1)
	{
		m_actualError = TOO_MANY_OPTIONS;
		treatErrors(m_arg);
	}
}
void Pkginfo::run()
{
	if (m_archiveinfo) {
		pair<string, pkginfo_t> packageArchive = openArchivePackage(m_packageArchiveName) ;
		string name = packageArchive.first + " : ";
		cout
			<< name << _("Description    : ") << packageArchive.second.description << endl
			<< name << _("Group          : ") << packageArchive.second.group << endl
			<< name << _("URL            : ") << packageArchive.second.url << endl
			<< name << _("Contributor(s) : ") << packageArchive.second.contributors << endl
			<< name << _("Packager(s)    : ") << packageArchive.second.packager << endl
			<< name << _("Version        : ") << packageArchive.second.version << endl
			<< name << _("Release        : ") << packageArchive.second.release << endl
			<< name << _("Architecture   : ") << packageArchive.second.arch  << endl
			<< name << _("Build date     : ") << packageArchive.second.build << endl;
		if (packageArchive.second.dependencies.size() > 0 ) {
			cout << name << _("Dependencies   : ");
			for ( auto i : packageArchive.second.dependencies) cout << i.first << i.second << " ";
			cout << endl;
		}
	}
	/*
	 *  Make footprint
	 */
	if (m_footprint_mode) {
		getFootprintPackage(m_arg);
	} else {
		/*
		 *  Modes that require the database to be opened
		 *
		 */
		Db_lock lock(m_root, false);
		getListOfPackageNames(m_root);
		if (m_installed_mode) {
			/*
			 *  List installed packages
			 */
			buildSimpleDatabase();
			for (auto i : m_listOfInstPackages) {
				if (m_fulllist_mode) {
					if ( i.second.dependency == true)
						cout << "AUTO: ";
					else
						cout << " MAN: ";

					cout << "(" << i.second.collection << ")"
					<< " " << i.first << " "
					<< i.second.version << "-" << i.second.release << endl;
				} else  {
					if ( i.second.dependency == true)
						continue;
					cout << "(" << i.second.collection << ")"
					<< " " << i.first << " "
					<< i.second.version << "-" << i.second.release << endl;
				}
			}
		} else if (m_list_mode) {
			/*
			 *  List package or file contents
			 */
			buildDatabase(true,false,false,false,"");
			if ( (! checkPackageNameExist(m_arg)) &&
					(! checkPackageNameExist(m_arg)) ) {
				m_actualError = NOT_INSTALL_PACKAGE_NEITHER_PACKAGE_FILE;
				treatErrors(m_arg);
			}
			buildDatabase(true,false,false,false,m_arg);
			if (checkPackageNameExist(m_arg)) {
				string arg = m_listOfAlias[m_arg];
				copy(m_listOfInstPackages[arg].files.begin(), m_listOfInstPackages[arg].files.end(), ostream_iterator<string>(cout, "\n"));
			} else if (checkFileExist(m_arg)) {
				pair<string, pkginfo_t> package = openArchivePackage(m_arg);
				copy(package.second.files.begin(), package.second.files.end(), ostream_iterator<string>(cout, "\n"));
			}
		} else if (m_runtimedependencies_mode) {
			/* 	Get runtimedependencies of the file found in the directory path
				get the list of installed packages silently */
			buildCompleteDatabase(true);
			int Result;
			set<string>filenameList;
			Result = findRecursiveFile (filenameList, m_arg.c_str(), WS_DEFAULT);
			/*
			 * get the list of libraries for all the possible files
			 */
			set<string> librariesList;
			for (auto i : filenameList) Result = getRuntimeLibrariesList(librariesList,i);
			/*
			 * Get the own package for all the elf files dependencies libraries
			 */
#ifndef NDEBUG
			for (auto i : librariesList) cerr << i <<endl;
#endif
			if ( (librariesList.size() > 0 ) && (Result > -1) ) {
				set<string> runtimeList;
				for (auto i : librariesList) {
					for (auto j : m_listOfInstPackages) {
						bool found = false;
						for (auto k : j.second.files) {
							if ( k.find('/' + i) != string::npos) {
								string dependency = j.first + ultos(j.second.build);
								runtimeList.insert(dependency);
								break;
								found = true;
							}
						}
						if ( found == true) {
							found = false;
							break;
						}
					}
				}
				if (runtimeList.size()>0) {
#ifndef NDEBUG
					cerr << "Number of libraries found: " << runtimeList.size() << endl;
#endif
					unsigned int s = 1;
					for ( auto i : runtimeList ) {
						cout << i << endl;
						s++;
					}
					cout << endl;
				}
			}	
		} else if (m_libraries_mode + m_runtime_mode > 0) {
			/*
			 * Get the list of installed packages silently
			 *
			 */
			buildCompleteDatabase(true);
			set<string> librariesList;
			int Result = -1;
			if (checkPackageNameExist(m_arg)) {
				for (set<string>::const_iterator i = m_listOfInstPackages[m_arg].files.begin();
					i != m_listOfInstPackages[m_arg].files.end();
					++i){
					string filename('/' + *i);
					Result = getRuntimeLibrariesList(librariesList,filename);
				}
				if ( (librariesList.size() > 0 ) && (Result > -1) ) {
					if (m_runtime_mode) {
						set<string> runtimeList;
						for (set<string>::const_iterator i = librariesList.begin();
						i != librariesList.end();
						++i) {
							for (packages_t::const_iterator j = m_listOfInstPackages.begin();
								j != m_listOfInstPackages.end();
								++j){
								bool found = false;
								for (set<string>::const_iterator k = j->second.files.begin();
									k != j->second.files.end();
									++k) {
									if ( k->find('/' + *i) != string::npos) {
										runtimeList.insert(j->first);
										break;
										found = true;
									}
								}
								if (found == true) {
									found = false;
									break;
								}
							}
						}
						if (runtimeList.size()>0) {
							unsigned int s = 1;
							for (set<string>::const_iterator i = runtimeList.begin();
								i!=runtimeList.end();
								++i){
								cout << *i;
								s++;
								if (s <= runtimeList.size())
									cout << ",";
							}
							cout << endl;
						}
					} else {
						for (set<string>::const_iterator i = librariesList.begin();i != librariesList.end();++i)
							cout << *i << endl;
					}
				}
			}	
		} else if (m_epoc) {
			/*
			 *  get the buildtime of the package: return 0 if not found
			 */
			buildCompleteDatabase(true);
			if (checkPackageNameExist(m_arg)) {
				cout << m_listOfInstPackages[m_arg].build << endl;
			} else {
				cout << "0" << endl;
			}
		} else if (m_details_mode) {
			/*
			 *  get all details of a package
			 */
			buildCompleteDatabase(false);
			if (checkPackageNameExist(m_arg)) {
				string arg = m_listOfAlias[m_arg];
				cout << _("Name           : ") << arg << endl
					<< _("Description    : ") << m_listOfInstPackages[arg].description << endl;
				if (m_listOfInstPackages[arg].alias.size() > 0 ) {
					cout << _("Alias          : ");
					for ( auto i : m_listOfInstPackages[arg].alias) cout << i << " ";
						cout << endl;
				}
				if (m_listOfInstPackages[arg].categories.size() > 0 ) {
					cout << _("Categories     : ");
					for ( auto i : m_listOfInstPackages[arg].categories) cout << i << " ";
						cout << endl;
				}
				if (m_listOfInstPackages[arg].set.size() > 0 ) {
					cout << _("Set            : ");
					for ( auto i : m_listOfInstPackages[arg].set) cout << i << " ";
						cout << endl;
				}
				cout << _("Group          : ") << m_listOfInstPackages[arg].group << endl
					<< _("Collection     : ") << m_listOfInstPackages[arg].collection << endl
					<< _("URL            : ") << m_listOfInstPackages[arg].url << endl
					<< _("Contributor(s) : ") << m_listOfInstPackages[arg].contributors << endl
					<< _("Packager       : ") << m_listOfInstPackages[arg].packager << endl
					<< _("Version        : ") << m_listOfInstPackages[arg].version << endl
					<< _("Release        : ") << m_listOfInstPackages[arg].release << endl
					<< _("Build date     : ") << getDateFromEpoch(m_listOfInstPackages[arg].build) << endl
					<< _("Installed date : ") << getDateFromEpoch(m_listOfInstPackages[arg].install) << endl
					<< _("Installed Size : ") << sizeHumanRead(m_listOfInstPackages[arg].size)
					<< _("bytes") << endl
					<< _("Number of Files: ") << m_listOfInstPackages[arg].files.size()
					<< _(" file(s)") << endl
					<< _("Arch           : ") << m_listOfInstPackages[arg].arch << endl;
				if ( m_listOfInstPackages[m_arg].dependency == false )
					cout << _("Man. installed : Yes");
				else
					cout << _("Man. installed : No");
				cout << endl;
				if ( m_listOfInstPackages[m_arg].dependencies.size() > 0 ) {
					cout << _("Dependencies   : ");
					for ( auto i : m_listOfInstPackages[arg].dependencies) cout << i.first << " ";
					cout << endl;
				}
			}
		} else if (m_owner_mode) {
			/*
			 * List owner(s) of file or directory
			 */
			buildCompleteDatabase(false);
			regex_t preg;
			if (regcomp(&preg, m_arg.c_str(), REG_EXTENDED | REG_NOSUB)) {
				m_actualError = CANNOT_COMPILE_REGULAR_EXPRESSION;
				treatErrors(m_arg);
			}
			vector<pair<string, string> > result;
			result.push_back(pair<string, string>(_("Package"), _("File")));
			unsigned int width = result.begin()->first.length(); // Width of "Package"
#ifndef NDEBUG
			cerr << m_arg << endl;
#endif
			for (auto i : m_listOfInstPackages) {
				for (auto j : i.second.files) {
					const string file('/' + j);
					if (!regexec(&preg, file.c_str(), 0, 0, 0)) {
						result.push_back(pair<string, string>(i.first, j));
						if (i.first.length() > width) {
							width = i.first.length();
						}
					}
				}
			}

			regfree(&preg);
			if (result.size() > 1) {
				for (auto i : result ) {
					cout << left << setw(width + 2) << i.first << i.second << endl;
				}
			} else {
				cout << m_utilName << _(": no owner(s) found") << endl;
			}
		}
	}
}
void Pkginfo::finish()
{
}
void Pkginfo::printHelp() const
{
	cout << USAGE << m_utilName << " [options]" << endl
		<< OPTIONS << endl
		<< "  -i, --installed             "
		<< _("list of installed packages") << endl
		<< "  -d, --details               "
		<< _("list details about the <package>") << endl
		<< "  -L, --libraries             "
		<< _("list all the runtime libraries for the <package>") << endl
		<< "  -l, --list <package|file>   "
		<< _("list files in <package> or <file>") << endl
		<< "  -o, --owner <pattern>       "
		<< _("list owner(s) of file(s) matching <pattern>") << endl
		<< "  -f, --footprint <file>      "
		<< _("print footprint for <file>") << endl
		<< "  -a, --archive <file>        "
		<< _("print Name, Version, Release, BuildDate and Deps of the <file>") << endl
		<< "  -b, --buildtime <package>   "
		<< _("return the build time of the package") << endl
		<< "  -R, --runtimedep <package>  "
		<< _("return on a single line all the runtime dependencies") << endl
		<< "  --runtimedepfiles <path>    "
		<< _("return on a single line all the runtime dependencies for the files found in the <path>") << endl
		<< "  -r, --root <path>           "        
		<< _("specify alternative installation root") << endl
		<< "  -v, --version               "
		<< _("print version and exit") << endl
		<< "  -h, --help                  "
		<< _("print help and exit") << endl;
}
// vim:set ts=2 :
