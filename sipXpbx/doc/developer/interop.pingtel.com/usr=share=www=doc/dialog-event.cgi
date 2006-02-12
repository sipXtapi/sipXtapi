#! /usr/bin/perl

use CGI qw/:standard/;
use XML::Parser;
use strict;

# This must be the redirector's log, since the NOTIFY is in-dialog and bypasses
# the proxy.
my($log_file) = '/var/log/sipxpbx/sipregistrar.log';

# The un-escape table for backslash escapes.
my(%unescape) = ("r", "\r",
		 "n", "\n",
		 "\"", "\"",
		 "\\", "\\");

# Get the extension.
my($extension) = &param('extension');

# Start the HTML.
print &header,
    &start_html('Dialog event package analysis'), "\n",
    &h1("Dialog event package analysis for extension $extension"), "\n";

# Validate the extension.
if ($extension !~ /^1\d\d[1-9]$/) {
    print &p(&escapeHTML("Invalid extension '$extension'.")),
    "\n",
    &end_html,
    "\n";
    exit 0;
}

# Get the dialog event package.
my($notify) = &find_last_notify($extension);

# If we can't find a NOTIFY.
if ($notify eq '') {
    print &p(&escapeHTML("No dialog event with state other than " .
			 "\"terminated\" found for extension $extension.")),
	"\n",
	&end_html,
	"\n";
    exit 0;
}

# Separate the headers and body.
my($headers, $body) = $notify =~ /^([\000-\377]*?)\n\n([\000-\377]*)$/;

print &p(&escapeHTML("The latest dialog event package for extension " .
		     "$extension with a state other than \"terminated\" is:")),
    "\n";
print &pre(&escapeHTML($headers . "\n\n" . $body)),
    "\n";

# Pretty-print it.

# Insert line breaks.
$body =~ s/>\s*</>\n</g;
# Indent each line appropriately.
my(@body) = split(/\n/, $body);
my($depth) = 0;
my($line);
foreach $line (@body) {
    my($x) = $line;
    my($delta) = 0;
    # Find all the start and end tags and adjust the depth accordingly.
    $x =~ s%(<(/?)[a-z][^>]*?(/?)>)% ($2 ne '' ? $delta-- : $delta++),
                                     ($3 ne '' ? $delta-- : 0),
                                     $1 %egi;
    # If the line starts with an end tag, outdent it so it aligns with 
    # its open tag.
    my($outdent) = ($x =~ m%^</[a-z]%i);
    $line = ('  ' x ($depth - $outdent)) . $line;
    $depth += $delta;
}

print &p(&escapeHTML("Reformatted, it looks like this:")),
    "\n";
print &pre(&escapeHTML(join("\n", @body))),
    "\n";

# Parse the event body.
my($parser) = new XML::Parser(Style => 'Tree');
my($tree);
# &XML::Parse::parse dies if it can't parse the string.
eval { $parser->parse($body) };

if ($@ ne '') {
    print &p(&font({-color => 'red'},
		   &escapeHTML("Could not parse the event body.")) .
	     &br .
	     &escapeHTML("XML parser message was: " . $@)),
	  "\n",
	  &end_html,
	  "\n";
    exit 0;
}

print &p(&escapeHTML("Event body passes XML syntax check.")),
    "\n";

print &end_html,
    "\n";

exit 0;

sub find_last_notify {
    my($extension) = @_;

    # Read through the log file and find the last NOTIFY from this extension.
    my($log_line) = '';
    open(LOG, $log_file) ||
	die "Error opening file '$log_file' for input: $!\n";
    while (<LOG>) {
	next unless /:INCOMING:/;
	next unless /----\\nNOTIFY\s/i;
	next unless /\\r\\nFrom:\s*sip:$extension@/i;
	next unless /\\nEvent:\s+dialog\b/i;
	# Do not record "terminated" notices, as they may not have complete
	# information.
	next if m%<state>terminated</state>%;
	# This line passes the tests, save it.
	$log_line = $_;
    }
    close LOG;

    # Normalize the log line.
    $log_line =~ s/^.*?----\\n//;
    $log_line =~ s/====*END====*\\n"\n$//;
    $log_line =~ s/\\(.)/$unescape{$1}/eg;
    $log_line =~ s/\r\n/\n/g;

    return $log_line;
}

no strict;

# Read and parse the registrations file.
$parser = new XML::Parser(Style => 'Tree');
$tree = $parser->parsefile($registration_file);

$table_body = '';
if (${$tree}[0] eq 'items') {
    my $c = ${$tree}[1];
    my $i;
    $timestamp = ${${$c}[0]}{'timestamp'};
    for ($i = 1; $i < $#$c; $i += 2) {
	if (${$c}[$i] eq 'item') {
	    my $d = ${$c}[$i+1];
            my $callid, $cseq, $aor, $contact, $q, $expires, $instance_id,
               $gruu;
            my $i;
	    for ($i = 1; $i < $#$d; $i += 2) {
                $e = ${$d}[$i];
                $f = ${$d}[$i+1];
                if ($e eq 'callid') {
		    $callid = &text($f);
                } elsif ($e eq 'cseq') {
                    $cseq = &text($f);
                } elsif ($e eq 'uri') {
                    $aor = &text($f);
                } elsif ($e eq 'contact') {
                    $contact = &text($f);
                } elsif ($e eq 'qvalue') {
                    $q = &text($f);
                } elsif ($e eq 'expires') {
                    $expires = &text($f) - $timestamp;
                } elsif ($e eq 'instance_id') {
                    $instance_id = &text($f);
                } elsif ($e eq 'gruu') {
                    $gruu = &text($f);
		}
	    }
            $table_body .= 
                Tr(td([escapeHTML($callid), escapeHTML($cseq),
                       escapeHTML($aor), escapeHTML($contact),
                       escapeHTML($q), escapeHTML($expires),
                       escapeHTML($instance_id), escapeHTML($gruu)])) . "\n"
                if $expires > 0;
        }
    }
} else {
    exit 1;
}

# Beware that <tr> is generated by the Tr() function, because tr is a
# keyword.
print table({-border => 1, -align => 'left'},
	    Tr(th(['Call-Id', 'CSeq', 'AOR', 'Contact', 'q', 'Expires',
		   'Instance ID', 'GRUU'])), "\n",
	    $table_body);

print end_html,
    "\n";

exit 0;

sub text {
    my($tree) = @_;
    my($text) = '';
    my $i;
    for ($i = 1; $i < $#$tree; $i += 2) {
	if (${$tree}[$i] eq '0') {
	    $text .= ${$tree}[$i+1];
        }
    }
    return $text;
}
