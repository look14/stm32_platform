proc manchester_encode {listBuf isZero10} {
	set listRet	[];
	set length	[llength  $listBuf];
	set zero	[expr ($isZero10 ?2 :1)];  
	set one		[expr ($isZero10 ?1 :2)];  
	
	for {set i 0} {$i<$length} {incr i} {
		set dat8	[lindex $listBuf $i];
		
		set res8 0;
		for {set j 0} {$j<4} {incr j} {
			set res8 [expr ($res8<<2) | (($dat8&0x80) ?$one :$zero)];
			set dat8 [expr $dat8 << 1];
		}
		lappend listRet [format "0x%02X" $res8]
		
		set res8 0;
		for {set j 0} {$j<4} {incr j} {
			set res8 [expr ($res8<<2) | (($dat8&0x80) ?$one :$zero)];
			set dat8 [expr $dat8 << 1];
		}
		lappend listRet [format "0x%02X" $res8]
	}
	
	return $listRet;
}

proc manchester_decode {listBuf isZero10} {
	set listRet	[];
	set length	[llength  $listBuf];
	set zero	[expr ($isZero10 ?2 :1)];  
	set one		[expr ($isZero10 ?1 :2)];
	
	set length	[expr $length/2];
	
	for {set i 0} {$i<$length} {incr i} {
		set res8	0;
	
		set index	[expr $i*2]
		set dat8	[lindex $listBuf $index];
		for {set j 0} {$j<4} {incr j} {
			set res8 [expr ($res8<<1) | (($dat8>>6)==$one ?1 :0)];
			set dat8 [expr ($dat8 << 2) & 0xFF];	
		}
		
		set index	[expr $i*2+1]
		set dat8	[lindex $listBuf $index];
		for {set j 0} {$j<4} {incr j} {
			set res8 [expr ($res8<<1) | (($dat8>>6)==$one ?1 :0)];
			set dat8 [expr ($dat8 << 2) & 0xFF];	
		}
		
		lappend listRet [format "0x%02X" $res8]
	}
	
	return $listRet;
}

proc crc16_ccitt_calc { listBuff } {

#	set CRC_POLY 0x8408;
	set CRC_POLY 0x1021;
#	set crc 0;
	set crc 0xFFFF;
	set len [llength $listBuff];
	
	for {set i 0} {$i<$len} {incr i} {
		set dat8 [lindex $listBuff $i];
		set crc  [expr $crc ^ ($dat8 << 8)];
		
		for {set j 0} {$j<8} {incr j} {
			if {[expr $crc & 0x8000]} {
				set crc [expr ($crc << 1) ^ $CRC_POLY];
			} else {
				set crc [expr $crc << 1];
			}
		}
		
		set crc [expr $crc & 0xFFFF];
		
	}
	
	return $crc;
}

proc crc16_calc { listBuff len CRC_POLY crc_seed } {

#	set CRC_POLY 0x8408;
#	set CRC_POLY 0x1021(CCITT);
#	set CRC_POLY 0x8005(IBM)
#	set CRC_POLY 0x3D65(DNP)
	set crc $crc_seed;
#	set len [llength $listBuff];
	
	for {set i 0} {$i<$len} {incr i} {
		set dat8 [lindex $listBuff $i];
		set crc  [expr $crc ^ ($dat8 << 8)];
		
		for {set j 0} {$j<8} {incr j} {
			if {[expr $crc & 0x8000]} {
				set crc [expr ($crc << 1) ^ $CRC_POLY];
			} else {
				set crc [expr $crc << 1];
			}
		}
		
		set crc [expr $crc & 0xFFFF];
		
	}
	
	return [format "0x%04X" $crc];
}

proc data16_inverse { dat16 } {
	set tmp16 0;
	
	for {set i 0} {$i<16} {incr i} {
		set tmp16 [expr ($tmp16 >> 1) | ($dat16 & 0x8000)];
		set dat16 [expr $dat16 << 1];
	}
	
	return [format "0x%04X" $tmp16];
}

proc data8_inverse { dat8 } {
	set tmp8 0;
	
	for {set i 0} {$i<8} {incr i} {
		set tmp8 [expr ($tmp8 >> 1) | ($dat8 & 0x80)];
		set dat8 [expr $dat8 << 1];
	}
	
	return [format "0x%02X" $tmp8];
}

