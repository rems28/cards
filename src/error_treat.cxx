//
//  error_treat.cxx
// 
//  Copyright (c) 2014-2017 by NuTyX team (http://nutyx.org)
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

#include "error_treat.h"

RunTimeErrorWithErrno::RunTimeErrorWithErrno(const std::string& msg) throw()
	: std::runtime_error(msg + std::string(": ") + strerror(errno))
{
}
RunTimeErrorWithErrno::RunTimeErrorWithErrno(const std::string& msg, int e) throw()
	: std::runtime_error(msg + std::string(": ") + strerror(e))
{
}
// vim:set ts=2 :
