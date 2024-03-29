//
//  pkgdbh.cxx
//
//  Copyright (c) 2000 - 2005 Per Liden
//  Copyright (c) 2006 - 2013 by CRUX team (http://crux.nu)
//  Copyright (c) 2013 - 2021 by NuTyX team (http://nutyx.org)
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

#include "pkgdbh.h"

using namespace std;

using __gnu_cxx::stdio_filebuf;

Pkgdbh::Pkgdbh(const std::string& name)
	: m_utilName(name), m_DB_Empty(true), m_miniDB_Empty(true)
{
	openlog(m_utilName.c_str(),LOG_CONS,LOG_LOCAL7);
}
Pkgdbh::~Pkgdbh()
{
#ifndef NDEBUG
		syslog(LOG_INFO,"Closing log...");
#endif
		runLastPostInstall();
		void closelog(void);
}
void Pkgdbh::parseArguments(int argc, char** argv)
{
	for (int i = 1; i < argc; i++) {
		string option(argv[i]);
		if (option == "-r" || option == "--root") {
			assertArgument(argv, argc, i);
			m_root = argv[i + 1];
			i++;
		} else if (option[0] == '-' || !m_packageName.empty()) {
			m_actualError = INVALID_OPTION;
			treatErrors(option);
		} else {
			m_packageName = option;
		}
	}
	if (m_root.empty())
		m_root="/";
	else
		m_root=m_root+"/";
}
void Pkgdbh::treatErrors(const std::string& s) const
{
	switch ( m_actualError )
	{
		case CANNOT_FIND_DEPOT:
			throw runtime_error(_("cannot find any depot, check your /etc/cards.conf file") + s);
			break;
		case CANNOT_CREATE_DIRECTORY:
		case CANNOT_GENERATE_LEVEL:
			break;
		case CANNOT_DOWNLOAD_FILE:
			throw runtime_error(_("could not download ") + s);
			break;
		case CANNOT_CREATE_FILE:
			throw RunTimeErrorWithErrno(_("could not created ") + s);
			break;
		case CANNOT_OPEN_FILE:
			throw RunTimeErrorWithErrno(_("could not open ") + s);
			break;
		case CANNOT_FIND_FILE:
			throw RunTimeErrorWithErrno(_("could not find ") + s);
			break;
		case CANNOT_READ_FILE:
			throw runtime_error(_("could not read ") + s);
			break;
		case CANNOT_PARSE_FILE:
			throw runtime_error(_("could not parse ") + s);
			break;
		case CANNOT_READ_DIRECTORY:
			throw RunTimeErrorWithErrno(_("could not read directory ") + s);
			break;
		case CANNOT_WRITE_FILE:
			throw RunTimeErrorWithErrno(_("could not write file ") + s);
			break;
		case CANNOT_SYNCHRONIZE:
			throw RunTimeErrorWithErrno(_("could not synchronize ") + s);
			break;
		case CANNOT_RENAME_FILE:
			throw RunTimeErrorWithErrno(_("could not rename ") + s);
			break;
		case CANNOT_COPY_FILE:
			throw RunTimeErrorWithErrno(_("could not copy ") + s);
			break;
		case CANNOT_DETERMINE_NAME_BUILDNR:
			throw RunTimeErrorWithErrno(_("could not determine name / build number ") + s);
			break;
		case WRONG_ARCHITECTURE:
			throw runtime_error(s + _(": wrong architecture") );
			break;
		case EMPTY_PACKAGE:
			throw RunTimeErrorWithErrno(_("empty package ") + s);
			break;
		case CANNOT_FORK:
			throw RunTimeErrorWithErrno(_("fork() failed ") + s);
			break;
		case WAIT_PID_FAILED:
			throw RunTimeErrorWithErrno(_("waitpid() failed ") + s);
			break;
		case DATABASE_LOCKED:
			throw RunTimeErrorWithErrno(_("Database  ") + s + _(" locked by another user"));
			break;
		case CANNOT_LOCK_DIRECTORY:
			throw RunTimeErrorWithErrno(_("could not lock directory ") + s);
			break;
		case CANNOT_REMOVE_FILE:
			throw RunTimeErrorWithErrno(_("could not remove file ") + s);
			break;
		case CANNOT_RENAME_DIRECTORY:
			throw RunTimeErrorWithErrno(_("could not rename directory ") + s);
			break;
		case OPTION_ONE_ARGUMENT:
			throw RunTimeErrorWithErrno(s + _(" require one argument"));
			break;
		case INVALID_OPTION:
			throw runtime_error(s + _(" invalid option") );
			break;
		case OPTION_MISSING:
			throw runtime_error(s + _(" option missing"));
			break;
		case TOO_MANY_OPTIONS:
			throw runtime_error(s+ _(": to many options"));
			break;
		case ONLY_ROOT_CAN_INSTALL_UPGRADE_REMOVE:
			throw runtime_error(s + _(" only root can install / upgrade / remove packages"));
			break;
		case PACKAGE_ALLREADY_INSTALL:
			throw runtime_error(_("package ") + s + _(" already installed (use -u to upgrade)"));
			break;
		case PACKAGE_NOT_INSTALL:
			throw runtime_error(_("package ") + s + _(" not yet installed"));
			break;
		case PACKAGE_NOT_PREVIOUSLY_INSTALL:
			throw runtime_error(_("package ") + s + _(" not previously installed (skip -u to install)"));
			break;
		case LISTED_FILES_ALLREADY_INSTALLED:
			throw runtime_error(s + _(": listed file(s) already installed (use -f to ignore and overwrite)"));
			break;
		case PKGADD_CONFIG_LINE_TOO_LONG:
			throw RunTimeErrorWithErrno(s + _(": line too long, aborting"));
			break;
		case PKGADD_CONFIG_WRONG_NUMBER_ARGUMENTS:
			throw RunTimeErrorWithErrno(s + _(": wrong number of arguments, aborting"));
			break;
		case PKGADD_CONFIG_UNKNOWN_ACTION:
			throw RunTimeErrorWithErrno(s + _("': config unknown action, should be YES or NO, aborting"));
			break;
		case PKGADD_CONFIG_UNKNOWN_EVENT:
			throw RunTimeErrorWithErrno(s + _("' unknown event, aborting"));
			break;
		case CANNOT_COMPILE_REGULAR_EXPRESSION:
			throw runtime_error(_("error compiling regular expression '") + s + _("', aborting"));
			break;
		case NOT_INSTALL_PACKAGE_NEITHER_PACKAGE_FILE:
			throw runtime_error(s + _(" is neither an installed package nor a package file"));
			break;
		case PACKAGE_NOT_FOUND:
			throw runtime_error(_("The package ") + s + _(" is not found"));
			break;
		case PACKAGE_NOT_EXIST:
			throw runtime_error(_("The package ") + s + _(" does not exist"));
			break;
		case PACKAGE_IN_USE:
			throw runtime_error(_("The package ") + s + _(" is in use"));
			break;
	}
}
void Pkgdbh::progressInfo()
{
  static int j=0;
  switch ( m_actualAction )
  {
		case PKG_DOWNLOAD_START:
			break;
		case PKG_DOWNLOAD_RUN:
			break;
		case PKG_DOWNLOAD_END:
			break;
		case PKG_MOVE_META_START:
			break;
		case PKG_MOVE_META_END:
			break;
		case DB_OPEN_START:
			cout << _("Retrieve info about the ")
			<< m_packageNamesList.size() << _(" packages: ");
			break;
		case DB_OPEN_RUN:
			j++;
			printf("%3u%%\b\b\b\b",
				( j * 100 ) / m_packageNamesList.size() );
			break;
		case DB_OPEN_END:
			printf("100 %%\n");
			j=0;
			break;
		case PKG_PREINSTALL_START:
			cout << _("pre-install: start") << endl;
			break;
		case PKG_PREINSTALL_END:
			cout << _("pre-install: finish") << endl;
			break;
		case PKG_INSTALL_START:
			j=0;
			cout << _("   ADD: (")
				<< m_packageArchiveCollection
				<< ") "
				<< m_packageName
				<< " "
				<< m_packageArchiveVersion
				<< "-"
				<< m_packageArchiveRelease
				<< ", "
				<< m_filesNumber << _(" files: ");
			break;
		case PKG_INSTALL_RUN:
			j++;
			printf("%3u%%\b\b\b\b",
				(m_installedFilesNumber * 100 ) /  m_filesNumber );
			break;
		case PKG_INSTALL_END:
			printf("100 %%\n");
			break;
		case PKG_POSTINSTALL_START:
			cout << _("post-install: start") << endl;
			break;
		case PKG_POSTINSTALL_END:
			cout << _("post-install: finish") << endl;
			break;
		case DB_ADD_PKG_START:
			break;
		case DB_ADD_PKG_END:
			break;
		case RM_PKG_FILES_START:
			j=0;
			cout << _("REMOVE: (")
				<< m_packageCollection
				<< ") "
				<< m_packageName
				<< " "
				<< m_packageVersion
				<< "-"
				<< m_packageRelease
				<< ", "
				<< m_filesList.size()
				<< _(" files: ");
			break;
		case RM_PKG_FILES_RUN:
			j++;
			printf("%3u%%\b\b\b\b",
				( j * 100 ) / m_filesList.size() );
			break;
		case RM_PKG_FILES_END:
			printf("100 %%\n");
			break;
		case LDCONFIG_START:
			break;
		case LDCONFIG_END:
			break;
  }
}
set<string> Pkgdbh::getListOfPackageName()
{
	return m_packageNamesList;
}
/* Append to the "DB" the number of packages founds
 * (directory containing a file named files
 * */
int Pkgdbh::getListOfPackageNames (const std::string& path)
{
	if (! m_packageNamesList.empty())
		return m_packageNamesList.size();

	const string pathdb =  m_root + PKG_DB_DIR;
#ifndef NDEBUG
	cerr << "pathdb: " << pathdb << endl;
#endif
	if ( findFile(m_packageNamesList, pathdb) != 0 ) {
		m_actualError = CANNOT_READ_FILE;
		treatErrors(pathdb);
	}
#ifndef NDEBUG
	cerr << "Number of Packages: "
		<< m_packageNamesList.size()
		<< endl;
#endif
	return m_packageNamesList.size();
}
/* get details info of a package */
std::pair<std::string, pkginfo_t> Pkgdbh::getInfosPackage
	(const std::string& packageName)
{
	pair<string, pkginfo_t> result;

	result.first = packageName;
	return result;
}
std::set<std::string> Pkgdbh::getFilesOfPackage
	( const std::string& packageName )
{
	string pkgName = m_listOfAlias[packageName];
	std::set<std::string> packageFiles;
	const string filelist = m_root
		+ PKG_DB_DIR
		+ pkgName
		+ PKG_FILES;
	int fd = open(filelist.c_str(), O_RDONLY);
	if (fd == -1) {
		m_actualError = CANNOT_OPEN_FILE;
		treatErrors(filelist);
	}
	stdio_filebuf<char> listbuf(fd, ios::in, getpagesize());
	istream in(&listbuf);
	if (!in)
		throw RunTimeErrorWithErrno("could not read " + filelist);
	while (!in.eof()){
	// read alls the files for alls the packages founds
	for (;;) {
			string file;
			getline(in, file);
			if (file.empty())
				break; // End of record
			packageFiles.insert(file);
		}
	}
	return packageFiles;
}
/**
 * Populate the database in following modes:
 * - if nothing specify only get the List of PackageNames
 *   and populate the alias list.
 * - if simple then only with name, version, release, collection
 *   build date, set list and group name
 * - if all then all the availables attributes
 * - if files then all the files of the package(s)
 * - if packageName size() > 0 then we do just for the packageName
 *
 */
void Pkgdbh::buildDatabase
	(const bool& progress, const bool& simple,
	const bool& all, const bool& files,
	const std::string& packageName)
{
	/*
	 * This part is done in every cases
	 */
	cleanupMetaFiles(m_root);
	if (progress) {
		m_actualAction = DB_OPEN_START;
		progressInfo();
	}
#ifndef NDEBUG
	cerr << "m_root: " << m_root << endl;
#endif
	if (m_packageNamesList.empty() )
		getListOfPackageNames (m_root);

	for ( auto pkgName : m_packageNamesList) {
		if (progress) {
			m_actualAction = DB_OPEN_RUN;
			progressInfo();
		}
		const string metaFile = m_root
		+ PKG_DB_DIR
		+ pkgName
		+ '/'
		+ PKG_META;
		std::set<std::string> fileContent;
		parseFile(fileContent,metaFile.c_str());
		m_listOfAlias[pkgName] = pkgName;
		for ( auto attribute : fileContent) {
			if ( attribute[0] != 'A' )
				break;
			m_listOfAlias[attribute.substr(1)] = pkgName;
		}
	}
	if ( !simple && !all && !files && (packageName.size() == 0) )
		return;
	if (packageName.size() > 0) {
		string pkgName = m_listOfAlias[packageName];
		pkginfo_t info;
		info.files = getFilesOfPackage(pkgName);
		m_listOfInstPackages[pkgName] = info;
	}
	if ( !simple && !all && !files ) {
		if (progress) {
			m_actualAction = DB_OPEN_END;
			progressInfo();
		}
		return;
	}
	if (simple) {
		pkginfo_t info;
		for ( auto pkgName : m_packageNamesList) {
			if (progress) {
				m_actualAction = DB_OPEN_RUN;
				progressInfo();
			}
			const string metaFile = m_root
			+ PKG_DB_DIR
			+ pkgName
			+ '/'
			+ PKG_META;
			std::set<std::string> fileContent;
			parseFile(fileContent,metaFile.c_str());
			m_listOfAlias[pkgName] = pkgName;
			unsigned short flags = 0;
			info.release = 1;
			for ( auto attribute : fileContent) {
				if ( attribute[0] == 'B' ) {
					info.build = strtoul(attribute.substr(1).c_str(),NULL,0);
					flags++;
				}
				if ( attribute[0] == 'V' ) {
					info.version = attribute.substr(1);
					flags=flags + 2;
				}
				if ( attribute[0] == 'c' ) {
					info.collection = attribute.substr(1);
					flags = flags + 4;
				}
				/* As a group is not always present we cannot
				 * depend on a found one to break */
				if ( attribute[0] == 'g' ) {
					info.group = attribute.substr(1);
				}
				/* As a set is not always present we cannot
				 * depen on a found one to break */
				if ( attribute[0] == 's' ) {
					info.set.insert(attribute.substr(1));
				}
				if ( attribute[0] == 'r' ) {
					info.release = atoi(attribute.substr(1).c_str());
					flags = flags + 8;
				}
				if ( flags == 15 ) {
					m_listOfInstPackages[pkgName] = info;
					break;
				}
			}
		}
	}
	if (progress)
	{
		m_actualAction = DB_OPEN_END;
		progressInfo();
	}
}
/**
 * Populate m_listOfInstalledPackagesWithDeps with:
 * - Name of the package
 * - Dependencies list of the package
 * */
void Pkgdbh::buildSimpleDependenciesDatabase()
{
	if (m_packageNamesList.empty() )
			getListOfPackageNames (m_root);
	for ( auto i : m_packageNamesList) {
		pair < string, set<string> > packageWithDeps;
		packageWithDeps.first=i;
		const string metaFile = m_root + PKG_DB_DIR + i + '/' + PKG_META;
		itemList * contentFile = initItemList();
		readFile(contentFile,metaFile.c_str());
		for (unsigned int li=0; li < contentFile->count ; ++li) {
			if ( contentFile->items[li][0] == 'R' ) {
				string s = contentFile->items[li];
				string dependency = s.substr(1,s.size()-11);
				packageWithDeps.second.insert(dependency);
			}
		}
		m_listOfInstalledPackagesWithDeps.insert(packageWithDeps);
	}
}
/**
 * Populate the database with:
 * - Name
 * - version
 * - release
 * - set
 * - collection
 * - Build date
 * - Group name
 * - Packager name
 * */
void Pkgdbh::buildSimpleDatabase()
{
	if (m_miniDB_Empty) {
		if (m_packageNamesList.empty() )
			getListOfPackageNames (m_root);
		for ( auto i : m_packageNamesList) {
			pkginfo_t info;
			const string metaFile = m_root + PKG_DB_DIR + i + '/' + PKG_META;
			itemList * contentFile = initItemList();
			readFile(contentFile,metaFile.c_str());
			info.release = 1;
			info.dependency = false;
			m_listOfAlias[i] = i;
			for (unsigned int li=0; li < contentFile->count ; ++li) {
				if ( contentFile->items[li][0] == 'c' ) {
					string s = contentFile->items[li];
					info.collection = s.substr(1);
				}
				if ( contentFile->items[li][0] == 's' ) {
					string s = contentFile->items[li];
					info.set.insert(s.substr(1));
				}
				if ( contentFile->items[li][0] == 'V' ) {
					string s = contentFile->items[li];
					info.version = s.substr(1);
				}
				if ( contentFile->items[li][0] == 'r' ) {
					string s = contentFile->items[li];
					info.release = atoi(s.substr(1).c_str());
				}
				if ( contentFile->items[li][0] == 'B' ) {
					string s = contentFile->items[li];
					info.build = strtoul(s.substr(1).c_str(),NULL,0);
				}
				if ( contentFile->items[li][0] == 'g' ) {
					string s = contentFile->items[li];
					info.group = s.substr(1);
				}
				if ( contentFile->items[li][0] == 'A' ) {
					string s = contentFile->items[li];
					m_listOfAlias[s.substr(1)] = i;
				}
				if ( contentFile->items[li][0] == 'P' ) {
					string s = contentFile->items[li];
					info.packager = s.substr(1);
				}
				if ( contentFile->items[li][0] == 'd' ) {
					string s = contentFile->items[li];
					if ( s == "d1" )
						info.dependency = true;
				}

			}
			m_listOfInstPackages[i] = info;
			freeItemList(contentFile);
		}
		m_miniDB_Empty=false;
	}
#ifndef NDEBUG
	for (auto i : m_listOfAlias)
		cerr << "Alias: " << i.first << ", Package: " << i.second << endl;
#endif
}
 /**
 * Populate the database with all details infos
 */
void Pkgdbh::buildCompleteDatabase(const bool& silent)
{
	cleanupMetaFiles(m_root);
	if (m_DB_Empty) {
		if (m_packageNamesList.empty() )
			getListOfPackageNames (m_root);

		if (!silent) {
			m_actualAction = DB_OPEN_START;
			progressInfo();
		}
#ifndef NDEBUG
		cerr << "m_root: " << m_root<< endl;
#endif

		for (auto i : m_packageNamesList) {
			if (!silent) {
				m_actualAction = DB_OPEN_RUN;
				progressInfo();
			}
			pkginfo_t info;
			const string metaFileDir = m_root + PKG_DB_DIR + i;
			const string metaFile = metaFileDir + '/' + PKG_META;
			info.install = getEpochModifyTimeFile(metaFileDir);
			itemList * contentFile = initItemList();
			readFile(contentFile,metaFile.c_str());
			info.release = 1;
			info.dependency = false;
			m_listOfAlias[i] = i;
			for (unsigned int li=0; li< contentFile->count ; ++li) {
				if ( contentFile->items[li][0] == 'C' ) {
					string s = contentFile->items[li];
					info.contributors = s.substr(1);
				}
				if ( contentFile->items[li][0] == 'D' ) {
					string s = contentFile->items[li];
					info.description = s.substr(1);
				}
				if ( contentFile->items[li][0] == 'B' ) {
					string s = contentFile->items[li];
					info.build = strtoul(s.substr(1).c_str(),NULL,0);
				}
				if ( contentFile->items[li][0] == 'U' ) {
					string s = contentFile->items[li];
					info.url = s.substr(1);
				}
				if ( contentFile->items[li][0] == 'M' ) {
					string s = contentFile->items[li];
					info.maintainer = s.substr(1);
				}
				if ( contentFile->items[li][0] == 'P' ) {
					string s = contentFile->items[li];
					info.packager = s.substr(1);
				}
				if ( contentFile->items[li][0] == 'V' ) {
					string s = contentFile->items[li];
					info.version = s.substr(1);
				}
				if ( contentFile->items[li][0] == 'r' ) {
					string s = contentFile->items[li];
					info.release = atoi(s.substr(1).c_str());
				}
				if ( contentFile->items[li][0] == 'a' ) {
					string s = contentFile->items[li];
					info.arch = s.substr(1);
				}
				if ( contentFile->items[li][0] == 'c' ) {
					string s = contentFile->items[li];
					info.collection = s.substr(1);
				}
				if ( contentFile->items[li][0] == 's' ) {
					string s = contentFile->items[li];
					info.set.insert(s.substr(1));
				}
				if ( contentFile->items[li][0] == 'g' ) {
					string s = contentFile->items[li];
					info.group = s.substr(1);
				}
				if ( contentFile->items[li][0] == 'd' ) {
					string s = contentFile->items[li];
					if ( s == "d1" )
						info.dependency = true;
				}
				if ( contentFile->items[li][0] == 'S' ) {
					string s = contentFile->items[li];
					info.size = atoi(s.substr(1).c_str());
				}
				if ( contentFile->items[li][0] == 'A' ) {
					string s = contentFile->items[li];
					info.alias.insert(s.substr(1));
					m_listOfAlias[s.substr(1)] = i;
				}
				if ( contentFile->items[li][0] == 'T' ) {
					string s = contentFile->items[li];
					info.categories.insert(s.substr(1));
				}
				if ( contentFile->items[li][0] == 'R' ) {
					string s = contentFile->items[li];
					std::pair<std::string,time_t > NameEpoch;
					NameEpoch.first=s.substr(1,s.size()-11);
					NameEpoch.second=strtoul((s.substr(s.size()-10)).c_str(),NULL,0);
					info.dependencies.insert(NameEpoch);
				}
			}
			freeItemList(contentFile);
			// list of files
			const string filelist = m_root + PKG_DB_DIR + i + PKG_FILES;
			int fd = open(filelist.c_str(), O_RDONLY);
			if (fd == -1) {
				m_actualError = CANNOT_OPEN_FILE;
				treatErrors(filelist);
			}
			stdio_filebuf<char> listbuf(fd, ios::in, getpagesize());
			istream in(&listbuf);
			if (!in)
				throw RunTimeErrorWithErrno("could not read " + filelist);

			while (!in.eof()){
				// read alls the files for alls the packages founds
				for (;;) {
					string file;
					getline(in, file);
					if (file.empty())
						break; // End of record
					info.files.insert(info.files.end(), file);
				}
				if (!info.files.empty())
					m_listOfInstPackages[i] = info;
			}
		}
#ifndef NDEBUG
		cerr << endl;
		cerr << m_listOfInstPackages.size()
		<< " packages found in database " << endl;
#endif
		if (!silent)
		{
			m_actualAction = DB_OPEN_END;
			progressInfo();
		}
		m_DB_Empty=false;
	}
}
void Pkgdbh::moveMetaFilesPackage(const std::string& name, pkginfo_t& info)
{
	set<string> metaFilesList;
	m_actualAction = PKG_MOVE_META_START;
	progressInfo();
	const string packagedir = m_root + PKG_DB_DIR ;
	const string packagenamedir = m_root + PKG_DB_DIR + name ;

	for (auto i: info.files)
	{
		if ( i[0] == '.' ) {
#ifndef NDEBUG
			cout << "i: " << i << endl;
#endif
			metaFilesList.insert(metaFilesList.end(), i );
		}
	}
	for ( auto i : metaFilesList) info.files.erase(i);
	removeFile ( m_root, "/.MTREE");
	metaFilesList.insert(".META");
	set<string> fileContent;
	if ( parseFile(fileContent,".META") == -1 ) {
		m_actualError = CANNOT_FIND_FILE;
		treatErrors(".META");
	}

	mkdir(packagenamedir.c_str(),0755);

	for (auto i : metaFilesList)
	{
		char * destFile = const_cast<char*>(i.c_str());
		destFile++;
		string file = packagenamedir + "/" + destFile;
		if (rename(i.c_str(), file.c_str()) == -1) {
			m_actualError = CANNOT_RENAME_FILE;
			treatErrors( i + " to " + file);
		}
		if ( i == ".POST" ) {
			if (copyFile(i.c_str(), file.c_str()) == -1) {
				m_actualError = CANNOT_COPY_FILE;
				treatErrors( file  + " to " + i);
			}
		}
	}
	if (m_dependency) {
		string file = packagenamedir + "/META";
		fileContent.insert("d1");
		std::ofstream out(file);
		for ( auto i: fileContent) out << i << endl;
		out.close();
		m_dependency=false;
	}
	m_actualAction = PKG_MOVE_META_END;
	progressInfo();
}
void Pkgdbh::addPackageFilesRefsToDB(const std::string& name, const pkginfo_t& info)
{

	m_listOfInstPackages[name] = info;
	const string packagedir = m_root + PKG_DB_DIR ;
	const string packagenamedir = m_root + PKG_DB_DIR + name ;
	mkdir(packagenamedir.c_str(),0755);
	const string fileslist = packagenamedir + PKG_FILES;
	const string fileslist_new = fileslist + ".imcomplete_transaction";
	int fd_new = creat(fileslist_new.c_str(),0644);
	if (fd_new == -1)
	{
		m_actualError = CANNOT_CREATE_FILE;
		treatErrors(fileslist_new);
	}

	stdio_filebuf<char> filebuf_new(fd_new, ios::out, getpagesize());

	ostream db_new(&filebuf_new);
	copy(info.files.begin(), info.files.end(), ostream_iterator<string>(db_new, "\n"));

	db_new.flush();
	if (!db_new)
	{
		rmdir(packagenamedir.c_str());
		m_actualError = CANNOT_WRITE_FILE;
		treatErrors(fileslist_new);
	}
	// Synchronize file to disk
	if (fsync(fd_new) == -1)
	{
		rmdir(packagenamedir.c_str());
		m_actualError = CANNOT_SYNCHRONIZE;
		treatErrors(fileslist_new);
	}
	// Move new database into place
	if (rename(fileslist_new.c_str(), fileslist.c_str()) == -1)
	{
		rmdir(packagenamedir.c_str());
		m_actualError = CANNOT_RENAME_FILE;
		treatErrors(fileslist_new + " to " + fileslist);
	}
}
bool Pkgdbh::checkPackageNameExist(const std::string& name) const
{
	return (m_listOfAlias.find(name) != m_listOfAlias.end());
}
bool Pkgdbh::checkDependency(const std::string& name)
{
	if  ( checkPackageNameExist(name) )
		return m_listOfInstPackages[name].dependency;
	return false;
}
void Pkgdbh::setDependency()
{
	m_dependency=true;
}
void Pkgdbh::resetDependency()
{
	m_dependency=false;
}
bool Pkgdbh::checkPackageNameUptodate(const std::pair<std::string, pkginfo_t>& archiveName)
{
	set<string>::iterator it = m_packageNamesList.find(archiveName.first);
	if (it == m_packageNamesList.end())
		return false;
	if (m_listOfInstPackages[archiveName.first].version !=  archiveName.second.version)
		return false;
	if (m_listOfInstPackages[archiveName.first].release !=  archiveName.second.release)
		return false;
	if (m_listOfInstPackages[archiveName.first].build < archiveName.second.build)
		return false;
	if (m_listOfInstPackages[archiveName.first].collection == "")
		return true;
	if (archiveName.second.collection == "" )
		return true;
	if (m_listOfInstPackages[archiveName.first].collection != archiveName.second.collection)
		return false;
	return true;
}
bool Pkgdbh::checkPackageNameBuildDateSame(const std::pair<std::string,time_t>& dependencieNameBuild)
{
	if (dependencieNameBuild.second == 0)
		return false;
	set<string>::iterator it = m_packageNamesList.find(dependencieNameBuild.first);
	if (it == m_packageNamesList.end())
		return false;
	if (m_listOfInstPackages[dependencieNameBuild.first].build < dependencieNameBuild.second)
		return false;
	return true;
}
/* Remove meta data about the removed package */
void Pkgdbh::removePackageFilesRefsFromDB(const std::string& name)
{
	if ( checkPackageNameExist(name)){
		m_packageVersion = m_listOfInstPackages[name].version;
		m_packageRelease = itos(m_listOfInstPackages[name].release);
		m_packageCollection = m_listOfInstPackages[name].collection;
	}
	set<string> metaFilesList;
	const string packagedir = m_root + PKG_DB_DIR ;
	const string arch = m_listOfInstPackages[name].arch;
	const string packagenamedir = m_root + PKG_DB_DIR + name;

	if ( findFile(metaFilesList, packagenamedir) != 0 ) {
		m_actualError = CANNOT_READ_FILE;
		treatErrors(packagenamedir);
	}
	if (metaFilesList.size() > 0) {
		for (set<string>::iterator i = metaFilesList.begin(); i != metaFilesList.end();++i) {
			const string filename = packagenamedir + "/" + *i;
			if (checkFileExist(filename) && remove(filename.c_str()) == -1) {
				const char* msg = strerror(errno);
					cerr << m_utilName << ": could not remove " << filename << ": " << msg << endl;
				}
#ifndef NDEBUG
				cerr  << "File: " << filename << " is removed"<< endl;
#endif
			}
	}
	if( remove(packagenamedir.c_str()) == -1) {
		const char* msg = strerror(errno);
		cerr << m_utilName << ": could not remove " << packagenamedir << ": " << msg << endl;
	}
#ifndef NDEBUG
	cerr  << "Directory: " << packagenamedir << " is removed"<< endl;
#endif
}

/* Remove the physical files after followings some rules */
void Pkgdbh::removePackageFiles(const std::string& name)
{
	m_filesList = m_listOfInstPackages[name].files;
	m_listOfInstPackages.erase(name);
	m_packageName =  name ;

#ifndef NDEBUG
	cerr << "Removing package phase 1 (all files in package):" << endl;
	copy(m_filesList.begin(), m_filesList.end(), ostream_iterator<string>(cerr, "\n"));
	cerr << endl;
#endif

	// Don't delete files that still have references
	for (packages_t::const_iterator i = m_listOfInstPackages.begin(); i != m_listOfInstPackages.end(); ++i)
		for (set<string>::const_iterator j = i->second.files.begin(); j != i->second.files.end(); ++j)
			m_filesList.erase(*j);

#ifndef NDEBUG
	cerr << "Removing package phase 2 (files that still have references excluded):" << endl;
	copy(m_filesList.begin(), m_filesList.end(), ostream_iterator<string>(cerr, "\n"));
	cerr << endl;
#endif

	m_actualAction = RM_PKG_FILES_START;
	progressInfo();
	// Delete the files from bottom to up to make sure we delete the contents of any folder before
	for (set<string>::const_reverse_iterator i = m_filesList.rbegin(); i != m_filesList.rend(); ++i) {
		m_actualAction = RM_PKG_FILES_RUN;
		progressInfo();
		const string filename = m_root + *i;
		if (checkFileExist(filename) && remove(filename.c_str()) == -1) {
			const char* msg = strerror(errno);
			cerr << m_utilName << ": could not remove " << filename << ": " << msg << endl;
		}
	}
	m_actualAction = RM_PKG_FILES_END;
	progressInfo();
}

void Pkgdbh::removePackageFiles(const std::string& name, const std::set<std::string>& keep_list)
{
	m_filesList = m_listOfInstPackages[name].files;
	m_listOfInstPackages.erase(name);
	m_packageName =  name ;
#ifndef NDEBUG
	cerr << "Removing package phase 1 (all files in package):" << endl;
	copy(m_filesList.begin(), m_filesList.end(), ostream_iterator<string>(cerr, "\n"));
	cerr << endl;
#endif

	// Don't delete files found in the keep list
	for (auto i : keep_list) m_filesList.erase(i);

#ifndef NDEBUG
	cerr << "Removing package phase 2 (files that is in the keep list excluded):" << endl;
	copy(m_filesList.begin(), m_filesList.end(), ostream_iterator<string>(cerr, "\n"));
	cerr << endl;
#endif

	// Don't delete files that still have references
	for (packages_t::const_iterator i = m_listOfInstPackages.begin(); i != m_listOfInstPackages.end(); ++i)
		for (set<string>::const_iterator j = i->second.files.begin(); j != i->second.files.end(); ++j)
			m_filesList.erase(*j);

#ifndef NDEBUG
	cerr << "Removing package phase 3 (files that still have references excluded):" << endl;
	copy(m_filesList.begin(), m_filesList.end(), ostream_iterator<string>(cerr, "\n"));
	cerr << endl;
#endif

	// Delete the files
	m_actualAction = RM_PKG_FILES_START;
	progressInfo();
	for (set<string>::const_reverse_iterator i = m_filesList.rbegin(); i != m_filesList.rend(); ++i) {
		m_actualAction = RM_PKG_FILES_RUN;
		progressInfo();
		const string filename = m_root + *i;
		if (checkFileExist(filename) && remove(filename.c_str()) == -1) {
			if (errno == ENOTEMPTY)
				continue;
			const char* msg = strerror(errno);
			cerr << m_utilName << ": could not remove " << filename << ": " << msg << endl;
		}
	}
	m_actualAction = RM_PKG_FILES_END;
	progressInfo();
}

void Pkgdbh::removePackageFilesRefsFromDB(std::set<std::string> files, const std::set<std::string>& keep_list)
{
	if ( checkPackageNameExist(m_packageName)){
		m_packageVersion = m_listOfInstPackages[m_packageName].version;
		m_packageRelease = itos(m_listOfInstPackages[m_packageName].release);
		m_packageCollection = m_listOfInstPackages[m_packageName].collection;
	}
	// Remove all references
	for (packages_t::iterator i = m_listOfInstPackages.begin(); i != m_listOfInstPackages.end(); ++i) {
		for ( auto j : files ) {
			size_t s = i->second.files.size();
			i->second.files.erase(j);
			// If number of references have change, we refresh the disk copy
			if ( s != i->second.files.size())
				addPackageFilesRefsToDB(i->first,i->second);
		}
	}
#ifndef NDEBUG
	cerr << "Removing files:" << endl;
	copy(files.begin(), files.end(), ostream_iterator<string>(cerr, "\n"));
	cerr << endl;
#endif

	// Don't delete files found in the keep list
	for (set<string>::const_iterator i = keep_list.begin(); i != keep_list.end(); ++i)
		files.erase(*i);

	// Delete the files
	for (set<string>::const_reverse_iterator i = files.rbegin(); i != files.rend(); ++i) {
		const string filename = m_root + *i;
		if (checkFileExist(filename) && remove(filename.c_str()) == -1) {
			if (errno == ENOTEMPTY)
				continue;
			const char* msg = strerror(errno);
			cerr << m_utilName << ": could not remove " << filename << ": " << msg << endl;
		}
	}
}

std::set<std::string> Pkgdbh::getConflictsFilesList
	(const std::string& name, const pkginfo_t& info)
{
	set<string> files;

	// Find conflicting files in database
	for (packages_t::const_iterator i = m_listOfInstPackages.begin(); i != m_listOfInstPackages.end(); ++i) {
		if (i->first != name) {
			set_intersection(info.files.begin(), info.files.end(),
					 i->second.files.begin(), i->second.files.end(),
					 inserter(files, files.end()));
		}
	}

#ifndef NDEBUG
	cerr << "Conflicts phase 1 (conflicts in database):" << endl;
	copy(files.begin(), files.end(), ostream_iterator<string>(cerr, "\n"));
	cerr << endl;
#endif

	// Find conflicting files in filesystem
	for (set<string>::iterator i = info.files.begin(); i != info.files.end(); ++i) {
		const string filename = m_root + *i;
		if (checkFileExist(filename) && files.find(*i) == files.end())
			files.insert(files.end(), *i);
	}

#ifndef NDEBUG
	cerr << "Conflicts phase 2 (conflicts in filesystem added):" << endl;
	copy(files.begin(), files.end(), ostream_iterator<string>(cerr, "\n"));
	cerr << endl;
#endif

	// Exclude directories
	set<string> tmp = files;
	for (set<string>::const_iterator i = tmp.begin(); i != tmp.end(); ++i) {
		if ((*i)[i->length() - 1] == '/')
			files.erase(*i);
	}

#ifndef NDEBUG
	cerr << "Conflicts phase 3 (directories excluded):" << endl;
	copy(files.begin(), files.end(), ostream_iterator<string>(cerr, "\n"));
	cerr << endl;
#endif

	// If this is an upgrade, remove files already owned by this package
	if (m_listOfInstPackages.find(name) != m_listOfInstPackages.end()) {
		for (set<string>::const_iterator i = m_listOfInstPackages[name].files.begin(); i != m_listOfInstPackages[name].files.end(); ++i)
			files.erase(*i);

#ifndef NDEBUG
		cerr << "Conflicts phase 4 (files already owned by this package excluded):" << endl;
		copy(files.begin(), files.end(), ostream_iterator<string>(cerr, "\n"));
		cerr << endl;
#endif
	}

	return files;
}
std::pair<std::string, pkginfo_t> Pkgdbh::openArchivePackage(const std::string& filename)
{
	string packageArchiveName;
	pair<string, pkginfo_t> result;
	ArchiveUtils packageArchive(filename.c_str());
#ifndef NDEBUG
	cerr << "Number of files: " << packageArchive.size() << endl;
#endif
	string basename(filename, filename.rfind('/') + 1);
	m_filesNumber = packageArchive.size();
	if (m_filesNumber == 0 ) {
		m_actualError = EMPTY_PACKAGE;
		treatErrors(basename);
	}
	packageArchiveName = packageArchive.name();
	if ( ( packageArchive.arch() != getMachineType() ) && ( packageArchive.arch() != "any" ) ) {
		if ( m_root.size() > 1 ) { // means it's in a chroot environment ... probably
			cerr << basename << ": Architecture is different. If "
				<<  m_root << " filesystem is a "
				<< getMachineType()
				<< " architecture, everything is OK..."
				<< endl;
		} else {
			m_actualError = WRONG_ARCHITECTURE;
			treatErrors(basename);
		}
	}
	m_packageArchiveCollection = packageArchive.collection();
	m_packageArchiveVersion = packageArchive.version();
	m_packageArchiveRelease = itos(packageArchive.release());
	if (packageArchiveName.empty() ) {
		m_actualError = CANNOT_DETERMINE_NAME_BUILDNR;
		treatErrors(basename);
	}
	result.first = packageArchiveName;
	result.second.description = packageArchive.description();
	result.second.url = packageArchive.url();
	result.second.maintainer = packageArchive.maintainer();
	result.second.packager = packageArchive.packager();
	result.second.version = packageArchive.version();
	result.second.release = packageArchive.release();
	result.second.arch = packageArchive.arch();
	result.second.build = packageArchive.buildn();
	result.second.group = packageArchive.group();

	set<string> fileList =  packageArchive.setofFiles();
	for (set<string>::iterator i = fileList.begin();i != fileList.end();++i) {
		result.second.files.insert(*i);
	}
	result.second.dependencies = packageArchive.listofDependenciesBuildDate();
	m_packageName = packageArchiveName;
	return result;
}
std::set< std::pair<std::string,time_t> > Pkgdbh::getPackageDependencies
	(const std::string& filename)
{
	pair<string, pkginfo_t> packageArchive;
	set< pair<string,time_t> > packageNameDepsBuildTime;
	set< pair<string,time_t> > packageNameDepsBuildTimeTemp;
	packageArchive = openArchivePackage(filename);
#ifndef NDEBUG
	cerr << "----> Begin of Direct Dependencies of " << packageArchive.first << endl;
#endif
	if ( checkPackageNameUptodate(packageArchive ) ) {
#ifndef NDEBUG
		cerr << packageArchive.first << " already installed and Up To Dated" << endl
			<< "----> NO Direct Dependencies" << endl
			<< "----> End of Direct Dependencies" << endl;
#endif
		return packageNameDepsBuildTime;
	}
	if (! packageArchive.second.dependencies.empty() )
		m_listOfDepotPackages[packageArchive.first] = packageArchive.second;
#ifndef NDEBUG
	cerr << "----> End of Direct Dependencies" << endl;
#endif
	packageNameDepsBuildTimeTemp = packageArchive.second.dependencies;
#ifndef NDEBUG
	cerr << "----> Before cleanup: " << packageArchive.first << endl;
	for (auto it : packageNameDepsBuildTimeTemp ) cerr << it.first << it.second<< " ";
	cerr << endl;
	int i=1;
#endif
	for (auto it : packageNameDepsBuildTimeTemp ) {
#ifndef NDEBUG
		cerr << it.first << " " <<  it.second << endl;
		cerr << "packageArchiveName:" <<packageArchive.first << endl;
#endif
		/*
		 * If actual and already present don't add the dep
		 */
		if ( checkPackageNameBuildDateSame(it)  )
			continue;
		packageNameDepsBuildTime.insert(it);
			//packageNameDepsBuildTime.erase(it);
#ifndef NDEBUG
		cerr << "----> " << it.first << " deleted" << endl;
		cerr << i << endl;
		i++;
#endif
	}
	if (! packageNameDepsBuildTime.empty() )
		m_listOfDepotPackages[packageArchive.first].dependencies = packageNameDepsBuildTime;
#ifndef NDEBUG
	cerr << "----> Number of remains direct deps: " << packageArchive.first << ": " << packageNameDepsBuildTime.size() << "/" << packageArchive.second.dependencies.size() << endl;
	for ( auto it : packageNameDepsBuildTime ) cerr << it.first << " " ;
	cerr << endl;
#endif
	return packageNameDepsBuildTime;
}
void Pkgdbh::extractAndRunPREfromPackage(const std::string& filename)
{
	char buf[PATH_MAX];
	struct archive* archive;
	struct archive_entry* entry;
	archive = archive_read_new();
	INIT_ARCHIVE(archive);
	if (archive_read_open_filename(archive,
		filename.c_str(),
		DEFAULT_BYTES_PER_BLOCK) != ARCHIVE_OK)
	{
		m_actualError = CANNOT_OPEN_FILE;
		treatErrors(filename);
	}

	getcwd(buf, sizeof(buf));
	chdir(m_root.c_str());

	for (m_installedFilesNumber = 0; archive_read_next_header(archive, &entry) ==
		ARCHIVE_OK; ++m_installedFilesNumber)
	{
		string archive_filename = archive_entry_pathname(entry);
		if ( strcmp(archive_filename.c_str(),PKG_PRE_INSTALL) == 0)
		{
			unsigned int flags = ARCHIVE_EXTRACT_OWNER | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_UNLINK;
			if (archive_read_extract(archive, entry, flags) != ARCHIVE_OK)
			{
				const char* msg = archive_error_string(archive);
				cerr << m_utilName << _(": could not install ") + archive_filename << " : " << msg << endl;
				exit(EXIT_FAILURE);
			}
			break;

		}

	}
	FREE_ARCHIVE(archive);
	if (checkFileExist(PKG_PRE_INSTALL))
	{
		m_actualAction = PKG_PREINSTALL_START;
		progressInfo();
		process preinstall(SHELL,PKG_PRE_INSTALL, 0 );
		if (preinstall.executeShell()) {
			exit(EXIT_FAILURE);
		}
		removeFile(m_root,PKG_PRE_INSTALL);
		m_actualAction = PKG_PREINSTALL_END;
		progressInfo();
	}
	chdir(buf);
}
void Pkgdbh::installArchivePackage
	(const std::string& filename, const std::set<std::string>& keep_list,
	const std::set<std::string>& non_install_list)
{
	struct archive* archive;
	struct archive_entry* entry;
	char buf[PATH_MAX];
	string absm_root;

	archive = archive_read_new();
	INIT_ARCHIVE(archive);

	if (archive_read_open_filename(archive,
	    filename.c_str(),
	    DEFAULT_BYTES_PER_BLOCK) != ARCHIVE_OK)
		{
			m_actualError = CANNOT_OPEN_FILE;
			treatErrors(filename);
		}
	chdir(m_root.c_str());
	absm_root = getcwd(buf, sizeof(buf));
#ifndef NDEBUG
	cout << "absm_root: " <<  absm_root  << " and m_root: " << m_root<< endl;
#endif
	m_actualAction = PKG_INSTALL_START;
	progressInfo();
	for (m_installedFilesNumber = 0; archive_read_next_header(archive, &entry) ==
	     ARCHIVE_OK; ++m_installedFilesNumber) {

		m_actualAction = PKG_INSTALL_RUN;
		progressInfo();
		string archive_filename = archive_entry_pathname(entry);
		string reject_dir = trimFileName(absm_root + string("/") + string(PKG_REJECTED));
		string original_filename = trimFileName(absm_root + string("/") + archive_filename);
		string real_filename = original_filename;

		// Check if file is filtered out via INSTALL
		if (non_install_list.find(archive_filename) != non_install_list.end()) {
			mode_t mode;

			cout << m_utilName << ": ignoring " << archive_filename << endl;

			mode = archive_entry_mode(entry);

			if (S_ISREG(mode))
				archive_read_data_skip(archive);

			continue;
		}

		// Check if file should be rejected
		if (checkFileExist(real_filename) && keep_list.find(archive_filename) != keep_list.end())
			real_filename = trimFileName(reject_dir + string("/") + archive_filename);

		archive_entry_set_pathname(entry, const_cast<char*>
		                           (real_filename.c_str()));

		// Extract file
		unsigned int flags = ARCHIVE_EXTRACT_OWNER | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_UNLINK;
		if (archive_read_extract(archive, entry, flags) != ARCHIVE_OK) {
			// If a file fails to install we just print an error message and
			// continue trying to install the rest of the package.
			const char* msg = archive_error_string(archive);
			cerr << m_utilName << ": could not install " + archive_filename << ": " << msg << endl;
			continue;

		}

		// Check rejected file
		if (real_filename != original_filename) {
			bool remove_file = false;
			mode_t mode = archive_entry_mode(entry);

			// Directory
			if (S_ISDIR(mode))
				remove_file = checkPermissionsEqual(real_filename, original_filename);
			// Other files
			else
				remove_file = checkPermissionsEqual(real_filename, original_filename) &&
					(checkFileEmpty(real_filename) || checkFilesEqual(real_filename, original_filename));

			// Remove rejected file or signal about its existence
			if (remove_file)
				removeFile(reject_dir, real_filename);
			else
				cout << m_utilName << ": rejecting " << archive_filename << ", keeping existing version" << endl;
		}
	}
	m_actualAction = PKG_INSTALL_END;
	progressInfo();
	if (m_installedFilesNumber == 0) {
		if (archive_errno(archive) == 0)
		{
			m_actualError = EMPTY_PACKAGE;
			treatErrors(filename);
		}
		else
		{
			m_actualError = CANNOT_READ_FILE;
			treatErrors(filename);
		}
	}
	FREE_ARCHIVE(archive);
}
void Pkgdbh::readRulesFile()
{
	m_actionRules.clear();
	unsigned int linecount = 0;
	const string filename = m_root + PKGADD_CONF;
	ifstream in(filename.c_str());

	if (in) {
		while (!in.eof()) {
			string line;
			getline(in, line);
			linecount++;
			if (!line.empty() && line[0] != '#') {
				if (line.length() >= PKGADD_CONF_MAXLINE) {
					m_actualError = PKGADD_CONFIG_LINE_TOO_LONG;
					treatErrors(filename + ":" + itos(linecount));
				}
				char event[PKGADD_CONF_MAXLINE];
				char pattern[PKGADD_CONF_MAXLINE];
				char action[PKGADD_CONF_MAXLINE];
				char dummy[PKGADD_CONF_MAXLINE];
				if (sscanf(line.c_str(), "%s %s %s %s", event, pattern, action, dummy) != 3) {
					m_actualError = PKGADD_CONFIG_WRONG_NUMBER_ARGUMENTS;
					treatErrors(filename + ":" + itos(linecount));
				}
				if (!strcmp(event, "UPGRADE") || !strcmp(event, "INSTALL") \
					|| !strcmp(event, "LDCONF") || !strcmp(event, "MIME_DB") \
					|| !strcmp(event, "INFO") || !strcmp(event, "ICONS") \
					|| !strcmp(event, "FONTS") \
					|| !strcmp(event, "QUERY_PIXBUF") || !strcmp(event, "GIO_QUERY") \
					|| !strcmp(event, "QUERY_IMOD3") || !strcmp(event, "QUERY_IMOD2") \
					|| !strcmp(event, "SCHEMAS") || !strcmp(event, "DESKTOP_DB")) {

					rule_t rule;
					if (!strcmp(event, "UPGRADE"))
						rule.event = UPGRADE;
					if (!strcmp(event, "INSTALL"))
						rule.event = INSTALL;
					if (!strcmp(event, "INFO"))
						rule.event = INFO;
					if (!strcmp(event, "LDCONF"))
						rule.event = LDCONF;
					if (!strcmp(event, "FONTS"))
						rule.event = FONTS;
					if (!strcmp(event, "ICONS"))
						rule.event = ICONS;
					if (!strcmp(event, "SCHEMAS"))
						rule.event = SCHEMAS;
					if (!strcmp(event, "DESKTOP_DB"))
						rule.event = DESKTOP_DB;
					if (!strcmp(event, "MIME_DB"))
						rule.event = MIME_DB;
					if (!strcmp(event, "QUERY_PIXBUF"))
						rule.event = QUERY_PIXBUF;
					if (!strcmp(event, "GIO_QUERY"))
						rule.event = GIO_QUERY;
					if (!strcmp(event, "QUERY_IMOD3"))
						rule.event = QUERY_IMOD3;
					if (!strcmp(event, "QUERY_IMOD2"))
						rule.event = QUERY_IMOD2;


					rule.pattern = pattern;
					if (!strcmp(action, "YES")) {
						rule.action = true;
					} else if (!strcmp(action, "NO")) {
						rule.action = false;
					} else {
						m_actualError = PKGADD_CONFIG_UNKNOWN_ACTION;
						treatErrors(filename + ":" + itos(linecount) + ": '" +
							string(action));
					}
					m_actionRules.push_back(rule);
				} else {
					m_actualError = PKGADD_CONFIG_UNKNOWN_EVENT;
					treatErrors(filename + ":" + itos(linecount) + ": '" +
						string(event));
				}
			}
		}
		in.close();
	}
#ifndef NDEBUG
	cerr << "Configuration:" << endl;
	for (auto j : m_actionRules )
		cerr << j.event << "\t" << j.pattern << "\t" << j.action << endl;
	cerr << endl;
#endif
}
void Pkgdbh::runLastPostInstall()
{
	if ( m_postInstallList.size() > 0 ) {
		m_actualAction = PKG_POSTINSTALL_START;
		progressInfo();
#ifndef NDEBUG
		for (auto i : m_postInstallList)
			cerr << i.second << " " << i.first << endl;
#endif
		process p;
		string args;
		for (auto i : m_postInstallList)
		switch ( i.second )
		{
			case LDCONF:
				if (checkFileExist(m_root + LDCONFIG_CONF)) {
					args = LDCONFIG_CONF_ARGS + m_root;
					p.execute(m_root + LDCONFIG, args,0);
				}
			break;

			case INFO:
				if (checkFileExist(m_root + INSTALL_INFO)) {
					args = INSTALL_INFO_ARGS + i.first;
					p.execute(m_root + INSTALL_INFO, args,0);
				}
			break;

			case ICONS:
				if (checkFileExist(m_root + UPDATE_ICON)) {
					args = UPDATE_ICON_ARGS + i.first;
					p.execute(m_root + UPDATE_ICON,args,0);
				}
			break;

			case FONTS:
				if (checkFileExist(m_root + MKFONTSCALE)) {
					args = MKFONTSCALE_ARGS + i.first;
					p.execute(m_root + MKFONTSCALE, args,0);
				}
				if (checkFileExist(m_root + MKFONTDIR)) {
					args = MKFONTDIR_ARGS + i.first;
					p.execute(m_root + MKFONTDIR, args,0);
				}
				if (checkFileExist(m_root + FC_CACHE)) {
					args = FC_CACHE_ARGS + i.first;
					p.execute(m_root + FC_CACHE, args,0);
				}
				break;

			case SCHEMAS:
				if (checkFileExist(m_root + COMPILE_SCHEMAS)) {
					args = COMPILE_SCHEMAS_ARGS + i.first;
					p.execute(m_root + COMPILE_SCHEMAS, args,0);
				}
			break;

			case DESKTOP_DB:
				if (checkFileExist(m_root + UPDATE_DESKTOP_DB)) {
					args = UPDATE_DESKTOP_DB_ARGS + i.first;
					p.execute(m_root + UPDATE_DESKTOP_DB, args,0);
				}
			break;

			case MIME_DB:
				if (checkFileExist(m_root + UPDATE_MIME_DB)) {
					args = UPDATE_MIME_DB_ARGS + i.first;
					p.execute(m_root + UPDATE_MIME_DB, args,0);
				}
			break;

			case QUERY_PIXBUF:
				if (checkFileExist(m_root + GDK_PIXBUF_QUERY_LOADER)) {
					args = GDK_PIXBUF_QUERY_LOADER_ARGS;
					p.execute(m_root + GDK_PIXBUF_QUERY_LOADER, args,0);
				}
			break;

			case GIO_QUERY:
				if (checkFileExist(m_root + GIO_QUERYMODULES)) {
					args = GIO_QUERYMODULES_ARGS;
					p.execute(m_root + GIO_QUERYMODULES, args,0);
				}
			break;

			case QUERY_IMOD3:
				if (checkFileExist(m_root + QUERY_IMMODULES_3)) {
					args = QUERY_IMMODULES_3_ARGS;
					p.execute(m_root + QUERY_IMMODULES_3, args,0);
				}
			break;

			case QUERY_IMOD2:
				if (checkFileExist(m_root + QUERY_IMMODULES_2)) {
					args = QUERY_IMMODULES_2_ARGS;
					p.execute(m_root + QUERY_IMMODULES_2, args,0);
				}
			break;
		}
		m_actualAction = PKG_POSTINSTALL_END;
		progressInfo();
	}
}
bool Pkgdbh::checkRuleAppliesToFile
	(const rule_t& rule, const std::string& file)
{
	regex_t preg;
	bool ret;

	if (regcomp(&preg, rule.pattern.c_str(), REG_EXTENDED | REG_NOSUB)) {
		m_actualError = CANNOT_COMPILE_REGULAR_EXPRESSION;
		treatErrors(rule.pattern);
	}
	ret = !regexec(&preg, file.c_str(), 0, 0, 0);
	regfree(&preg);

	return ret;
}
void Pkgdbh::getFootprintPackage(std::string& filename)
{
	unsigned int i;
	struct archive* archive;
	struct archive_entry* entry;

	map<string, mode_t> hardlink_target_modes;

	// We first do a run over the archive and remember the modes
	// of regular files.
	// In the second run, we print the footprint - using the stored
	// modes for hardlinks.
	//
	// FIXME the code duplication here is butt ugly
	archive = archive_read_new();
	INIT_ARCHIVE(archive);

	if (archive_read_open_filename(archive,
	    filename.c_str(),
	    DEFAULT_BYTES_PER_BLOCK) != ARCHIVE_OK)
			{
				m_actualError = CANNOT_OPEN_FILE;
				treatErrors(filename);
			}
      //         throw RunTimeErrorWithErrno("could not open " + filename, archive_errno(archive));

	for (i = 0; archive_read_next_header(archive, &entry) ==
	     ARCHIVE_OK; ++i) {

		mode_t mode = archive_entry_mode(entry);

		if (!archive_entry_hardlink(entry)) {
			const char *s = archive_entry_pathname(entry);

			hardlink_target_modes[s] = mode;
		}

		if (S_ISREG(mode) && archive_read_data_skip(archive))
		{
			m_actualError = CANNOT_READ_FILE;
			treatErrors(filename);
		//	throw RunTimeErrorWithErrno("could not read " + filename, archive_errno(archive));
		}
	}
	FREE_ARCHIVE(archive);

	// Too bad, there doesn't seem to be a way to reuse our archive
	// instance
	archive = archive_read_new();
	INIT_ARCHIVE(archive);

	if (archive_read_open_filename(archive,
	    filename.c_str(),
	    DEFAULT_BYTES_PER_BLOCK) != ARCHIVE_OK)
			{
				m_actualError = CANNOT_OPEN_FILE;
				treatErrors(filename);
        // throw RunTimeErrorWithErrno("could not open " + filename, archive_errno(archive));
			}
	for (i = 0; archive_read_next_header(archive, &entry) ==
	     ARCHIVE_OK; ++i) {
		mode_t mode = archive_entry_mode(entry);

		// Access permissions
		if (S_ISLNK(mode)) {
			// Access permissions on symlinks differ among filesystems, e.g. XFS and ext2 have different.
			// To avoid getting different footprints we always use "lrwxrwxrwx".
			cout << "lrwxrwxrwx";
		} else {
			const char *h = archive_entry_hardlink(entry);

			if (h)
				cout << mtos(hardlink_target_modes[h]);
			else
				cout << mtos(mode);
		}

		cout << '\t';

		// User
		uid_t uid = archive_entry_uid(entry);
		struct passwd* pw = getpwuid(uid);
		if (pw)
			cout << pw->pw_name;
		else
			cout << uid;

		cout << '/';

		// Group
		gid_t gid = archive_entry_gid(entry);
		struct group* gr = getgrgid(gid);
		if (gr)
			cout << gr->gr_name;
		else
			cout << gid;

		// Filename
		cout << '\t' << archive_entry_pathname(entry);

		// Special cases
		if (S_ISLNK(mode)) {
			// Symlink
			cout << " -> " << archive_entry_symlink(entry);
		} else if (S_ISCHR(mode) ||
		           S_ISBLK(mode)) {
			// Device
			cout << " (" << archive_entry_rdevmajor(entry)
			     << ", " << archive_entry_rdevminor(entry)
			     << ")";
		} else if (S_ISREG(mode) &&
		           archive_entry_size(entry) == 0) {
			// Empty regular file
			cout << " (EMPTY)";
		}

		cout << '\n';

		if (S_ISREG(mode) && archive_read_data_skip(archive))
		{
			m_actualError = CANNOT_READ_FILE;
			treatErrors(filename);
		//	throw RunTimeErrorWithErrno("could not read " + filename, archive_errno(archive));
		}
	}

	if (i == 0) {
		if (archive_errno(archive) == 0)
		{
			m_actualError = EMPTY_PACKAGE;
			treatErrors(filename);
		}
		else
		{
			m_actualError = CANNOT_READ_FILE;
			treatErrors(filename);
		}
	}
	FREE_ARCHIVE(archive);
}

void Pkgdbh::print_version() const
{
	cout << m_utilName << " (cards) " << VERSION << endl;
}

unsigned int Pkgdbh::getFilesNumber()
{
    return m_filesNumber;
}

unsigned int Pkgdbh::getInstalledFilesNumber()
{
    return m_installedFilesNumber;
}
set<string> Pkgdbh::getFilesList()
{
	return m_filesList;
}
Db_lock::Db_lock(const std::string& m_root, bool exclusive)
	: m_dir(0)
{
  // Ignore signals
  memset(&m_sa, 0, sizeof(m_sa));
  m_sa.sa_handler = SIG_IGN;
  sigaction(SIGHUP, &m_sa, NULL);
  sigaction(SIGINT, &m_sa, NULL);
  sigaction(SIGQUIT, &m_sa, NULL);
  sigaction(SIGTERM, &m_sa, NULL);

	const string dirname = trimFileName(m_root + string("/") + PKG_DB_DIR);
#ifndef NDEBUG
	cerr << "Lock the database " << dirname << endl;
#endif
	if (!(m_dir = opendir(dirname.c_str())))
		throw RunTimeErrorWithErrno("could not read directory " + dirname);

	if (flock(dirfd(m_dir), (exclusive ? LOCK_EX : LOCK_SH) | LOCK_NB) == -1) {
		if (errno == EWOULDBLOCK)
			throw runtime_error("package database is currently locked by another process");
		else
			throw RunTimeErrorWithErrno("could not lock directory " + dirname);
	}
}

Db_lock::~Db_lock()
{
	m_sa.sa_handler = SIG_DFL;
	signal(SIGHUP,SIG_DFL);
	signal(SIGINT,SIG_DFL);
	signal(SIGQUIT,SIG_DFL);
	signal(SIGTERM,SIG_DFL);
	if (m_dir) {
		flock(dirfd(m_dir), LOCK_UN);
		closedir(m_dir);
	}
#ifndef NDEBUG
	cerr << "Unlock the database " << m_dir << endl;
#endif
}
/*******************     End of Members *********************/

/*******************   Various fonctions ********************/

void assertArgument(char** argv, int argc, int index)
{
	if (argc - 1 < index + 1)
		throw runtime_error("option " + string(argv[index]) + " requires an argument");
}
void rotatingCursor() {
  static int pos=0;
  char cursor[4]={'/','-','\\','|'};
  fprintf(stderr,"\r [ %c ] ", cursor[pos]);
  fflush(stderr);
  pos = (pos+1) % 4;
}
// vim:set ts=2 :
