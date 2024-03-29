/*
 * flcards_info.h
 *
 * Copyright 2016 - 2020 Thierry Nuttens <tnut@nutyx.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */


#ifndef FLCARDS_INFO_H_
#define FLCARDS_INFO_H_ 1

#include <libcards.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Progress.H>

class Flcards_info : public Pkgdbh, public Pkgrepo {
public:
	Flcards_info (const std::string& configFile);
	~Flcards_info() {}
	std::set<std::string> getListOfInstalledPackages();
	std::set<std::string> getListOfAvailablePackages();

protected:
	void progressInfo();
	Fl_Window *m_window;
	Fl_Progress *m_progressBar;
};

#endif /* FLCARDS_INFO_H_ */
