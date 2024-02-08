[![Stand With Ukraine](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/banner2-direct.svg)](https://stand-with-ukraine.pp.ua)

pgmeminfo
=======

contains function `pgmeminfo`.

# Licence

Copyright (c) Pavel Stehule (pavel.stehule@gmail.com)

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

# Installation

You will probably need the "postgres.h" source file.  If you're using Debian
or many other Linux distributions that use aptitude, installation is easy:

    apt install postgres-server-dev-all

Installation instructions:

    git clone https://github.com/okbob/pgmeminfo/
    cd pgmeminfo
    make
    make install

Example usage (the "psql" command may require additional parameters such as a
username and a database name):

    psql
    INSTALL EXTENSION pgmeminfo;
    \x
    SELECT * FROM pgmeminfo();


    SELECT * FROM pgmeminfo_contexts(); -- show cummulative size
    SELECT * FROM pgmeminfo_contexts(deep => 1); -- show to deep 1

    -- show all without accumulation
    SELECT * FROM pgmeminfo_contexts(deep => -1, accum_mode => 'off');

# Note

If you like it, send a postcard to address

    Pavel Stehule
    Skalice 12
    256 01 Benesov u Prahy
    Czech Republic


I invite any questions, comments, bug reports, patches on mail address pavel.stehule@gmail.com
