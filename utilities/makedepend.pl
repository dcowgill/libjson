#!/usr/bin/perl -w

use strict;
use IO::File;

my $MARKER_LINE = '# DO NOT DELETE THIS LINE -- make depend depends on it.';

# Determine what command we should use and generate the dependency rules.
my $make_depend = shift @ARGV;
my $dependency_rules = `$make_depend`;

# Open the current Makefile for reading.
my $oldfh = IO::File->new('Makefile', 'r') or
    die "couldn't open Makefile for reading: $!";

# Open a temporary file--the new Makefile--for writing.
my $newfh = IO::File->new('Makefile.new', 'w') or
    die "couldn't open Makefile.new for writing: $!";
flock($newfh, 2) or
    die "couldn't lock Makefile.new for writing: $!";

# Print lines from the current Makefile verbatim to the new Makefile, until:
while (<$oldfh>) {
    last if /^$MARKER_LINE/;  # ... we encounter the marker line.
    print $newfh $_;
}

# Add the marker line to the new Makefile, then the dependency rules.
print $newfh "$MARKER_LINE\n\n$dependency_rules";

# Close both files, checking for i/o errors.
$oldfh->close or die "i/o error reading from Makefile: $!";
$newfh->close or die "i/o error writing to Makefile.new: $!";

# Swap the files, then delete the obsolete Makefile.
rename('Makefile', 'Makefile.bak') or
    die "couldn't rename Makefile to Makefile.bak: $!";
rename('Makefile.new', 'Makefile') or
    die "couldn't rename Makefile.new to Makefile: $!";
unlink('Makefile.bak');

exit;
