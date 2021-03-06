# -*- coding: utf-8 -*-
#!/usr/bin/env python
#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: http://www.qt.io/licensing/
##
## This file is part of the Qt Installer Framework.
##
## $QT_BEGIN_LICENSE:LGPL21$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see http://www.qt.io/terms-conditions. For further
## information use the contact form at http://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 2.1 or version 3 as published by the Free
## Software Foundation and appearing in the file LICENSE.LGPLv21 and
## LICENSE.LGPLv3 included in the packaging of this file. Please review the
## following information to ensure the GNU Lesser General Public License
## requirements will be met: https://www.gnu.org/licenses/lgpl.html and
## http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
##
## As a special exception, The Qt Company gives you certain additional
## rights. These rights are described in The Qt Company LGPL Exception
## version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
##
## $QT_END_LICENSE$
##
#############################################################################


import optparse, sys
from testrunner import testrunner

from xml.sax.saxutils import XMLGenerator
from xml.sax.xmlreader import AttributesNSImpl

class Writer:
    def __init__( self, out ):
        self._out = out
        self.gen = XMLGenerator( out, 'utf-8' )
        self.gen.startDocument()
        self.gen.startElement( 'results', {} )
    
    def addResult( self, name, status, errstr ):
        self.gen.startElement('result', { 'name':name, 'status':status } )
        self.gen.characters( errstr )
        self.gen.endElement('result' )

    def addFailed( self, name, errstr ):
        self.addResult( name, "failed", errstr )

    def addPassed( self, status, errstr ):
        self.addResult( name, "passed", errstr )
        
    def finalize( self ):
        self.gen.endElement( 'results' )
        self.gen.endDocument
        self._out.write( '\n' )

optionParser = optparse.OptionParser(usage="%prog [options] testcaseDir", version="%prog 0.1")
optionParser.add_option("-p", "--omit-prefix", dest="prefix", help="for file checks, prefix relative paths with this prefix", metavar="PREFIX" )
optionParser.add_option("-o", "--output", dest="output", help="write results to file (instead of stdout)", metavar="OUTPUT" )
(options, args) = optionParser.parse_args()

try:
    testdir = args[0]
except IndexError:
    optionParser.print_usage( sys.stderr )
    sys.exit( 1 )

out = sys.stdout
if options.output != None:
    out = file( options.output, 'wb' )

writer = Writer( out )
try:
    runner = testrunner.TestRunner( testdir, options.prefix, writer )
    runner.run()
finally:
    writer.finalize()