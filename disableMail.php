

<?php
	// PHP program to delete a file named gfg.txt 
	// using unlike() function 

	$file_pointer = "sendMailEnable.txt"; 

	// Use unlink() function to delete a file 
	if (!unlink($file_pointer)) { 
		echo ("$file_pointer cannot be deleted due to an error"); 
	} 
	else { 
		echo ("$file_pointer has been deleted"); 
	} 

?>
