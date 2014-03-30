//  config_parser.h
//
//  Copyright (c) 2002-2005 by Johannes Winkelmann jw at tks6 dot net
//  Copyright (c) 2014 by NuTyX team (http://nutyx.org)
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

#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <string>
#include <vector>
#include <map>

typedef std::map<std::string, std::string> dir_url;

struct Config
{
	Config() {}
	dir_url dirUrl;
};

class ConfigParser
{
	public:
		static std::string stripWhiteSpace(const std::string& input);
		static int parseConfig(const std::string& fileName,
			Config& config);
};
#endif /* CONFIGPARSER_H */
// vim:set ts=2 :