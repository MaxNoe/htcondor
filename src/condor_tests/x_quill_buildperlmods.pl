#!/usr/bin/env perl
use Cwd;
use CondorTest;

#my $dbinstalllog =  $installlog;
#print "Trying to open logfile... $dbinstalllog\n";
#open(OLDOUT, ">&STDOUT");
#open(OLDERR, ">&STDERR");
#open(STDOUT, ">>$dbinstalllog") or die "Could not open $dbinstalllog: $!";
#open(STDERR, ">&STDOUT");
#select(STDERR);
 #$| = 1;
#select(STDOUT);
 #$| = 1;

$topdir = getcwd();
$installdir = $topdir . "/perllib";
$ENV{PERL5LIB} = $installdir . "/lib/site_perl" . ":" . $installdir . "/lib/perl5/site_perl" . ":" . $installdir . "/lib64/perl5/site_perl" . ":" . $installdir . "/lib64/site_perl" . ":" . $installdir . "/Library/Perl";
print "Adjusted Perl Library search to $ENV{PERL5LIB}\n";

@modules = ( "IO-Tty-1.07", "Expect-1.20" );

foreach $module (@modules) {
	print "Building $module now\n";
	my $tarfile = "x_quill_" . $module . ".tar.gz";
	print "Extracting  $tarfile\n";
	$gettar = system("tar -zxvf $tarfile");
	if($gettar != 0) {
		die "Could not extract $module\n";
	}
	print "Module is <<$module>>\n";
	chdir "$module";
	system("pwd");
	$mkmake = system("perl Makefile.PL PREFIX=$installdir");
	if($mkmake != 0) {
		die "Could not make makefile for $module\n";
	}
	$domake = system("make");
	if($domake != 0) {
		die "Could not make $module\n";
	}
	$dotest = system("make test");
	if($dotest != 0) {
		die "Could not test $module\n";
	}
	$doinstall = system("make install");
	if($doinstall != 0) {
		die "Could not install $module\n";
	}
	chdir "$topdir" ;
}

exit(0);
