#!/usr/bin/perl

use strict;
use warnings;
use CGI;

my $cgi = CGI->new;
my %param = map { $_ => scalar $cgi->param($_) } $cgi->param() ;
print $cgi->header( -type => 'text/html' );
print "<h1>" . qq{PARAM:} . "</h1>";
for my $k ( sort keys %param ) {
    print "<div>" . join ": ", $k, $param{$k} . "</div>";
}