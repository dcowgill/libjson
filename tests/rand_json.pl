#!/usr/bin/perl -w

# Prints random JSON code to standard output.

use strict;

my $MAX_DEPTH = 10;
my $MAX_MEMBERS = 10;
my @CHARS = (
    'a'..'z',
    'A'..'Z',
    '0'..'9',
    ' ',
    "\b",
    "\f",
    "\n",
    "\r",
    "\t",
    '"');

my $depth = 0;


#
# MAIN
#

print gen_array() . "\n";
exit(0);

#
# SUBROUTINES
#

sub gen_null {
    'null'
}

sub gen_boolean {
    rand(1) >= 0.5 ? 'true' : 'false'
}

sub gen_number {
    rand(100_000_000)
}

sub gen_string {
    my $length = 1 + int(rand(20));  # 1-20
    my $answer = "";
    for (1..$length) {
        $answer .= $CHARS[int(rand(@CHARS))];
    }
    $answer =~ s{"}{\\"}gxs;
    return '"' . $answer . '"';
}

sub gen_array {
    return '[]' unless $depth < $MAX_DEPTH;
    $depth++;
    my $answer = "";
    my $length = int(rand($MAX_MEMBERS+1));
    for (1..$length) {
        $answer .= gen_random() . ", ";
    }
    $depth--;
    return "[ $answer ]";
}

sub gen_object {
    return '{}' unless $depth < $MAX_DEPTH;
    $depth++;
    my $answer = "";
    my $length = int(rand($MAX_MEMBERS+1));
    for (1..$length) {
        $answer .= gen_string() . ' : ' . gen_random() . ", ";
    }
    $depth--;
    return "{ $answer }";

}

sub gen_random {
    my @generators = (
        \&gen_null,
        \&gen_boolean,
        \&gen_number,
        \&gen_string,
        \&gen_array,
        \&gen_object,
        );

    $generators[int(rand(@generators))]->();
}
