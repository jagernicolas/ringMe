###########################################################################
# Copyright (C) 2016 by Savoir-faire Linux                                #
# Author: Jäger Nicolas <nicolas.jager@savoirfairelinux.com>              #
# Author: Traczyk Andreas <traczyk.andreas@savoirfairelinux.com>          #
#                                                                         #
# This program is free software; you can redistribute it and/or modify    #
# it under the terms of the GNU General Public License as published by    #
# the Free Software Foundation; either version 3 of the License, or       #
# (at your option) any later version.                                     #
#                                                                         #
# This program is distributed in the hope that it will be useful,         #
# but WITHOUT ANY WARRANTY; without even the implied warranty of          #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           #
# GNU General Public License for more details.                            #
#                                                                         #
# You should have received a copy of the GNU General Public License       #
# along with this program.  If not, see <http://www.gnu.org/licenses/>.   #
###########################################################################

CPPFLAGS=-g -std=c++11 -I../ring-project/install/daemon/include/dring -I../ring-project/daemon/src
LDFLAGS=-std=c++11
LDLIBS=-pthread ../ring-project/daemon/src/libring.la -lm
BC=../ring-project/daemon/libtool link g++

ringMe: main.o
	$(BC) $(LDFLAGS) -o ringMe main.o $(LDLIBS)

main.o: main.cc
	g++ $(CPPFLAGS) -c main.cc

clean:
	rm -f main.o ringMe
