<!DOCTYPE html>                                                                                                        
<html>                                                                                                                 
                                                                                                                       
<body>                                                                                                                 
                                                                                                                       
    <?php                                                                                                              
    $dir = "motion/";
                                                                                                                       
    // $files = scandir("$dir");
                                                                                                                       
    foreach (scandir("$dir", SCANDIR_SORT_DESCENDING) as $year) {                                                                                        
        if (fnmatch("[0-9]*", $year)) {                                                                                 
	    
			echo "
			<a href=\"showYear.php?year=$year\" >
				$year
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
