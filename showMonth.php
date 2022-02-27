<!DOCTYPE html>                                                                                                        
<html>                                                                                                                 
                                                                                                                       
<body>                                                                                                                 
                                                                                                                       
    <?php                                                                                                              
                                                                                                                       
    // $files = scandir("$dir");
	$year=(string)$_GET['year'];
	$month=(string)$_GET['month'];
    $dir = "motion/$year/$month/";
                                                                                                                       
    foreach (scandir("$dir", SCANDIR_SORT_ASCENDING) as $day) {                                                                                        
        if (fnmatch("[0-9]*", $day)) {                                                                                 
	    
			echo "
			<a href=\"showDay.php?year=$year&month=$month&day=$day&lite=0\" >
				$day
			</a>
			";
			// foreach (scandir("$dir" + $year, SCANDIR_SORT_DESCENDING) as $month) {
			//     if (fnmatch("[0-9]*", $month)) {
            //
			//         foreach (scandir("$dir" + $year, SCANDIR_SORT_DESCENDING) as $month) {
			//             if (fnmatch("[0-9]*", $month)) {
            //
			//                 // <a href=\"$dir/$year/$month/mean.png\" >
			//                 echo "
			//                 <a href=\"showDay.php?year=$year&month=$month\" >
			//                     <img src=\"$dir/$year/$month/mean.png\" />
			//                 </a>
			//                 ";
			//             }
            //
			//         }
			//     }
			// }
		}

        // if (fnmatch("*.png", $file)) {                                                                                 
        //     echo "                                                                                                     
        //         <a href=\"$dir/$file\" >                                                                               
        //             <img src=\"$dir/$file\" />                                                    
        //         </a>                                                                                                   
        //         ";                                                                                                     
        // }                                                                                                              
                                                                                                                       
                                                                                                                       
        // if (fnmatch("*.ogv", $file)) {                                                                              
        //     echo "                                                                                                  
        //         <video controls>                                                                                    
        //             <source src=\"$dir/$file\" type=video/ogg>                                                      
        //         </video>                                                                                            
        //         ";                                                                                                  
        // }                                                                                                           
    }                                                                                                                  
                                                                                                                       
    ?>                                                                                                                 
                                                                                                                       
</body>                                                                                                                
                                                                                                                       
</html>                                                                                                                
