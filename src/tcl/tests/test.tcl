source all.tcl

set time_sec 0;
set time_us  0;

proc timestamp_sub { sec2 us2 sec1 us1 } {

	set count 0;

	set count [expr 1000000*($sec2-$sec1) + $us2 - $us1 ];
	
	return $count;
}

for {set i 0} {$i < 100} {incr i} {

	set list      [get_timestamp];
	set time_sec2 [lindex $list 0];
	set time_us2  [lindex $list 1];
	
	set time_sub  [timestamp_sub $time_sec2 $time_us2 $time_sec $time_us];
	
	puts [format "%d %d %d" $time_sec2 $time_us2 $time_sub ];
	
	set time_sec $time_sec2;
	set time_us  $time_us2;
}