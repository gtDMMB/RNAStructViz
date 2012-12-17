package paul_specials;

use strict;
use warnings;

require      Exporter;
our @ISA       = qw(Exporter);
our @EXPORT    = qw( table_to_bracket table_to_bracket5 make_pair_table);

#input is base-pair table, output is dot-bracket notation. NOte: $table[0]=seq_length. 
#ADD PKNOTS SUPPORT!
sub table_to_bracket {
    
    my $ref = shift;
    my $dotb = ".";
    my $temp = 0;

    my @tbl = @$ref; #
    my $len = $tbl[0];

    for (my $j = 0; $j < $len; $j++ ) {
	substr($dotb, $j, 1) = ".";
    }
    #print "\n$dotb\n\n";
    
    for (my $j = 1; $j < $len+1; $j++ ) {
	
	if ($tbl[$j]>$j) {
	    substr($dotb, $j-1, 1) = "\(";
	    $temp = $tbl[$j] - 1;
	    substr($dotb, $temp, 1) = "\)";
	}

    }
    
    return $dotb;
}

sub table_to_bracket5 {
    
    my $ref = shift;
    my $dotb = ".";
    my $temp = 0;

    my @tbl = @$ref; 
    my $len = $tbl[0]+1;

    #print "table_to_bracket: len = $len\n";
    
    for (my $j = 0; $j < $len; $j++ ) {
	substr($dotb, $j, 1) = ".";
    }
    
    for (my $j = 0; $j < $len; $j++ ) {
	
	if ($tbl[$j+1]>$j+1) {
	    substr($dotb, $j, 1) = "\(";
	    $temp = $tbl[$j+1] - 2;
	    substr($dotb, $temp, 1) = "\)";
	}
	#elsif ($tbl[$j+1]== 0) {
	#    substr($dotb, $j, 1) = "\.";
	#}
    }
    
    return $dotb;
}



sub make_pair_table {
    my $str = shift;
    my $i;
    my @bps = ();
    my @bps_square = ();
    my @bps_curly = ();
    my @bps_angle = ();
    my @pair_table;
    my $prime5;
    my $prime3;
    my $count = 0;
    
    my $len = length($str);
    
    #print "make_pair_table: len = $len\n";

    $pair_table[0] = $len;

    for (my $j = 0; $j < $len; $j++ ) {
	$pair_table[$j+1] = 0;
	if ( substr($str,$j,1) =~ '\(' ){
	    push(@bps, $j);
	    ++$count;
	}	
	elsif ( substr($str,$j,1) =~ '\)' ){
	    $prime5 = pop(@bps);
	    $prime3 = $j;
	    $pair_table[$prime3+1] = $prime5+1;
	    $pair_table[$prime5+1] = $prime3+1;
	    --$count;
	    #print "base-pair: $pair_table[$i][$prime3] . $pair_table[$i][$prime5]\n"
	}
	elsif ( substr($str,$j,1) =~ '\[' ){
	    push(@bps_square, $j);
	    ++$count;
	}	
	elsif ( substr($str,$j,1) =~ '\]' ){
	    $prime5 = pop(@bps_square);
	    $prime3 = $j;
	    $pair_table[$prime3+1] = $prime5+1;
	    $pair_table[$prime5+1] = $prime3+1;
	    --$count;
	}
	elsif ( substr($str,$j,1) =~ '\{' ){
	    push(@bps_curly, $j);
	    ++$count;
	}	
	elsif ( substr($str,$j,1) =~ '\}' ){
	    $prime5 = pop(@bps_curly);
	    $prime3 = $j;
	    $pair_table[$prime3+1] = $prime5+1;
	    $pair_table[$prime5+1] = $prime3+1;
	    --$count;
	}
	elsif ( substr($str,$j,1) =~ '\<' ){
	    push(@bps_angle, $j);
	    ++$count;
	}	
	elsif ( substr($str,$j,1) =~ '\>' ){
	    $prime5 = pop(@bps_angle);
	    $prime3 = $j;
	    $pair_table[$prime3+1] = $prime5+1;
	    $pair_table[$prime5+1] = $prime3+1;
	    --$count;
	}
    }
    
#({[<.>]})
    
#print "pair table:\n";
#for ($i = 1; $i < $len+1; $i++ ) {
#    print "$pair_table[$i] ";
#}
    
#print "\n";
#
#print "pair_table = $#pair_table\n";    

if ($count){
    print "Unbalanced brackets in:
$str\n";
    die;
}    


    return @pair_table;
    
}




1;
