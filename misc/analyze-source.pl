#!/usr/bin/perl -w
################################################################################
# misc/analyze-source.pl
#
# Part of CryptoTE v0.0.0, see http://panthema.net/2007/cryptote
#
# Copyright (C) 2014 Timo Bingmann <tb@panthema.net>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA 02111-1307 USA
################################################################################

# print multiple email addresses
my $email_multimap = 0;

# launch emacsen for each error
my $launch_emacs = 0;

# write changes to files (dangerous!)
my $write_changes = 0;

# function testing whether to uncrustify a path
sub filter_uncrustify($) {
    my ($path) = @_;

    return 1;
}

use strict;
use warnings;
use Text::Diff;
use File::stat;

my %includemap;
my %authormap;

sub expect_error($$$$) {
    my ($path,$ln,$str,$expect) = @_;

    print("Bad header line $ln in $path\n");
    print("Expected $expect\n");
    print("Got      $str\n");

    system("emacsclient -n $path") if $launch_emacs;
}

sub expect($$\$$) {
    my ($path,$ln,$str,$expect) = @_;

    if ($$str ne $expect) {
        expect_error($path,$ln,$$str,$expect);
        $$str = $expect;
    }
}
sub expect_re($$\$$) {
    my ($path,$ln,$str,$expect) = @_;

    if ($$str !~ m/$expect/) {
        expect_error($path,$ln,$$str,"/$expect/");
    }
}

# check equality of two arrays
sub array_equal {
    my ($a1ref,$a2ref) = @_;

    my @a1 = @{$a1ref};
    my @a2 = @{$a2ref};

    return 0 if scalar(@a1) != scalar(@a2);

    for my $i (0..scalar(@a1)-1) {
        return 0 if $a1[$i] ne $a2[$i];
    }

    return 1;
}

# run $text through a external pipe (@program)
sub filter_program {
    my $text = shift;
    my @program = @_;

    # fork and read output
    my $child1 = open(my $fh, "-|") // die("$0: fork: $!");
    if ($child1 == 0) {
        # fork and print text
        my $child2 = open(STDIN, "-|") // die("$0: fork: $!");
        if ($child2 == 0) {
            print $text;
            exit;
        }
        else {
            exec(@program) or die("$0: exec: $!");
        }
    }
    else {
        my @output = <$fh>;
        close($fh) or warn("$0: close: $!");
        return @output;
    }
}

sub process_cpp {
    my ($path,$opt) = @_;
    $opt or $opt = "";

    # check permissions
    my $st = stat($path) or die("Cannot stat() file $path: $!");
    if ($st->mode & 0133) {
        print("Wrong mode ".sprintf("%o", $st->mode)." on $path\n");
        if ($write_changes) {
            chmod(0644, $path) or die("Cannot chmod() file $path: $!");
        }
    }

    # read file
    open(F, $path) or die("Cannot read file $path: $!");
    my @data = <F>;
    close(F);

    # put all #include lines into the includemap
    foreach my $ln (@data)
    {
        if ($ln =~ m!\s*#\s*include\s*([<"]\S+[">])!) {
            $includemap{$1}{$path} = 1;
        }
    }

    # check source header
    my $i = 0;
    if ($data[$i] =~ m!// -.*- mode:!)
    {
        ++$i;
    }                       # skip emacs mode line
    expect($path, $i, $data[$i], "/".('*'x79)."\n"); ++$i;
    expect($path, $i, $data[$i], " * $path\n"); ++$i;
    expect($path, $i, $data[$i], " *\n"); ++$i;

    # skip over comment
    while ($data[$i] !~ /^ \* Part of CryptoTE/)
    {
        expect_re($path, $i, $data[$i], '^ \*( .*)?\n$');
        return unless ++$i < @data;
    }

    expect($path, $i-1, $data[$i-1], " *\n");
    expect($path, $i, $data[$i], " * Part of CryptoTE v0.0.0, see http://panthema.net/2007/cryptote\n"); ++$i;
    expect($path, $i, $data[$i], " ".('*'x79)."\n"); ++$i;

    # read authors
    while ($data[$i] =~ /^ \* Copyright \(C\) ([0-9-]+(, [0-9-]+)*) (?<name>[^0-9<]+)( <(?<mail>[^>]+)>)?\n/)
    {
        #print "Author: $+{name} - $+{mail}\n";
        $authormap{$+{name}}{$+{mail} || ""} = 1;
        return unless ++$i < @data;
    }

    # otherwise check license
    expect($path, $i, $data[$i], " *\n"); ++$i;

    my $license =<<EOF;
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
EOF
    foreach my $lic (split(/\n/, $license)) {
        expect($path, $i, $data[$i], $lic."\n"); ++$i;
    }
    expect($path, $i, $data[$i], " ".('*'x78)."/\n"); ++$i;

    # check include guard name
    if ($path =~ m!\.(h|h.in)$! && $opt ne "noguard")
    {
        expect($path, $i, $data[$i], "\n"); ++$i;

        # construct include guard macro name: CRYPTOTE_FILE_NAME_HEADER
        my $guard = $path;
        $guard =~ s!src/!!;
        $guard =~ s!cryptote/!src/!;
        $guard =~ tr!/-!__!;
        $guard =~ s!\.h(\.in)?$!!;
        $guard = "CRYPTOTE_".uc($guard)."_HEADER";

        expect($path, $i, $data[$i], "#ifndef $guard\n"); ++$i;
        expect($path, $i, $data[$i], "#define $guard\n"); ++$i;

        my $n = scalar(@data)-1;
        expect($path, $n-2, $data[$n-2], "#endif // !$guard\n");
    }

    # check terminating /****/ comment
    {
        my $n = scalar(@data)-1;
        if ($data[$n] !~ m!^/\*{78}/$!) {
            push(@data, "/".('*'x78)."/\n");
        }
    }

    # run uncrustify if in filter
    if (filter_uncrustify($path))
    {
        my $data = join("", @data);
        my @uncrust = filter_program($data, "uncrustify", "-q", "-c", "misc/uncrustify.cfg", "-l", "CPP");

        # manually add blank line after "namespace xyz {" and before "} // namespace xyz"
        my $namespace = 0;
        for(my $i = 0; $i < @uncrust-1; ++$i)
        {
            if ($uncrust[$i] =~ m!^namespace \S+ {!) {
                splice(@uncrust, $i+1, 0, "\n");
                ++$namespace;
            }
            if ($uncrust[$i] =~ m!^}\s+// namespace!) {
                splice(@uncrust, $i, 0, "\n"); ++$i;
                --$namespace;
            }
        }
        if ($namespace != 0) {
            print "$path\n";
            print "    NAMESPACE MISMATCH!\n";
            #system("emacsclient -n $path");
        }

        if (!array_equal(\@data,\@uncrust)) {
            print "$path\n";
            print diff(\@data, \@uncrust);
            @data = @uncrust;
            #system("emacsclient -n $path");
        }
    }

    if ($write_changes)
    {
        open(F, "> $path") or die("Cannot write $path: $!");
        print(F join("", @data));
        close(F);
    }
}

sub process_pl_cmake {
    my ($path) = @_;

    # check permissions
    if ($path !~ /\.pl$/) {
        my $st = stat($path) or die("Cannot stat() file $path: $!");
        if ($st->mode & 0133) {
            print("Wrong mode ".sprintf("%o", $st->mode)." on $path\n");
            if ($write_changes) {
                chmod(0644, $path) or die("Cannot chmod() file $path: $!");
            }
        }
    }

    # read file
    open(F, $path) or die("Cannot read file $path: $!");
    my @data = <F>;
    close(F);

    # check source header
    my $i = 0;
    if ($data[$i] =~ m/#!/) { ++$i; } # bash line
    expect($path, $i, $data[$i], ('#'x80)."\n"); ++$i;
    expect($path, $i, $data[$i], "# $path\n"); ++$i;
    expect($path, $i, $data[$i], "#\n"); ++$i;

    # skip over comment
    while ($data[$i] !~ /^# Part of CryptoTE/) {
        expect_re($path, $i, $data[$i], '^#( .*)?\n$');
        return unless ++$i < @data;
    }

    expect($path, $i-1, $data[$i-1], "#\n");
    expect($path, $i, $data[$i], "# Part of CryptoTE v0.0.0, see http://panthema.net/2007/cryptote\n"); ++$i;
    expect($path, $i, $data[$i], "#\n"); ++$i;

    # read authors
    while ($data[$i] =~ /^# Copyright \(C\) ([0-9-]+(, [0-9-]+)*) (?<name>[^0-9<]+)( <(?<mail>[^>]+)>)?\n/) {
        #print "Author: $+{name} - $+{mail}\n";
        $authormap{$+{name}}{$+{mail} || ""} = 1;
        return unless ++$i < @data;
    }

    # otherwise check license
    expect($path, $i, $data[$i], "#\n"); ++$i;

    my $license =<<EOF;
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA 02111-1307 USA
EOF
    foreach my $lic (split(/\n/, $license)) {
        expect($path, $i, $data[$i], $lic."\n"); ++$i;
    }
    expect($path, $i, $data[$i], ('#'x80)."\n"); ++$i;

    if ($write_changes)
    {
        open(F, "> $path") or die("Cannot write $path: $!");
        print(F join("", @data));
        close(F);
    }
}

### Main ###

foreach my $arg (@ARGV) {
    if ($arg eq "-w") { $write_changes = 1; }
    elsif ($arg eq "-e") { $launch_emacs = 1; }
    elsif ($arg eq "-m") { $email_multimap = 1; }
    else {
        print "Unknown parameter: $arg\n";
    }
}

(-e "src/cryptote/main.cpp")
    or die("Please run this script in the CryptoTE source base directory.");

use File::Find;
my @filelist;
find(sub { !-d && push(@filelist, $File::Find::name) }, ".");

foreach my $file (@filelist)
{
    $file =~ s!./!! or die("File does not start ./");

    if ($file =~ m!^b!) {
    }
    elsif ($file =~ /\.(am|pl)$/) {
        process_pl_cmake($file);
    }
    # skip art
    elsif ($file =~ m!^src/art/.*\.(png|pdf|xcf|svg|ico|icns)$!) {
    }
    elsif ($file =~ m!^libenctain/botan-1.6/!) {
    }
    elsif ($file =~ m!^src/cryptote/imaglbox\.(h|cpp)$!) {
    }
    elsif ($file =~ m!^src/common/myintl\.(h|cpp)$!) {
    }
    elsif ($file =~ m!^libstc!) {
    }
    elsif ($file =~ m!^src/(art|help|locale)/.*\.(h|cpp)$!) {
        process_cpp($file, "noguard");
    }
    elsif ($file =~ /\.(h|cpp)$/) {
        process_cpp($file);
    }
    elsif ($file =~ m!/Makefile(.in)?$!) {
    }
    elsif ($file =~ m!^(acscripts|desktop)/!) {
    }
    elsif ($file =~ m!^src/.*\.(wxg|xml)$!) {
    }
    elsif ($file =~ m!^src/help/.*\.(ini|tex|sty|css|hh[kpc]|cached|html|gif)$!) {
    }
    elsif ($file =~ m!^src/locale/.*\.(mo|po|pot)$!) {
    }
    elsif ($file =~ m!^win32/.*\.(in|rtf|bmp)$!) {
    }
    # generate object files
    elsif ($file =~ m!\.(o|Po|a)$!) {
    }
    # recognize further files
    elsif ($file =~ m!^[^/]*$!) { # files in source root
    }
    elsif ($file =~ m!^\.git/!) {
    }
    elsif ($file =~ m!^misc/uncrustify.cfg$!) {
    }
    elsif ($file =~ m!^doxygen-html!) {
    }
    elsif ($file =~ m!README$!) {
    }
    else {
        print "Unknown file type $file\n";
    }
}

# print includes to includemap.txt
if (0)
{
    print "Writing includemap:\n";
    foreach my $inc (sort keys %includemap)
    {
        print "$inc => ".scalar(keys %{$includemap{$inc}})." [";
        print join(",", sort keys %{$includemap{$inc}}). "]\n";
    }
}

# check includemap for C-style headers
{

    my @cheaders = qw(assert.h ctype.h errno.h fenv.h float.h inttypes.h
                      limits.h locale.h math.h signal.h stdarg.h stddef.h
                      stdlib.h stdio.h string.h time.h);

    foreach my $ch (@cheaders)
    {
        $ch = "<$ch>";
        next if !$includemap{$ch};
        print "Replace c-style header $ch in\n";
        print "    [".join(",", sort keys %{$includemap{$ch}}). "]\n";
    }
}

# print authors to AUTHORS
print "Writing AUTHORS:\n";
open(A, "> AUTHORS");
foreach my $a (sort keys %authormap)
{
    my $mail = $authormap{$a};
    if ($email_multimap) {
        $mail = join(",", sort keys %{$mail});
    }
    else {
        $mail = (sort keys(%{$mail}))[0]; # pick first
    }
    $mail = $mail ? " <$mail>" : "";

    print "  $a$mail\n";
    print A "$a$mail\n";
}
close(A);
