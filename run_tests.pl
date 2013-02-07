#!/usr/bin/perl -w

# Runs all regression tests in the current directory.

use strict;

my $tests_run = 0;
my $tests_failed = 0;

my @files = @ARGV;
unless (@files) {
    @files = <tests/*.in>;
}

my $out;
for my $in (@files) {
    ($out = $in) =~ s/\.in$/.out/;
    if (system("./filter <$in | diff -u $out - 2>&1") != 0) {
        print "FAILED: $in => $out\n";
        $tests_failed++;
    }
    $tests_run++;
}

printf("%d of %d tests passed.\n", $tests_run-$tests_failed, $tests_run);
