
<?php
	function showDay($year, $month, $day, $lite) {
		$dir = "motion/$year/$month/$day";
		$files = scandir("$dir/");

		$date=strtotime("$year-$month-$day");

		$days = array('Dimanche', 'Lundi', 'Mardi', 'Mercredi','Jeudi','Vendredi', 'Samedi');
		$dayStr=$days[date('w', $date)];

		$months = array('Janvier', 'Février', 'Mars', 'Avril','Mai','Juin', 'Juillet', 'Août', 'Septembre', 'Octobre', 'Novembre', 'Décembre');
		$a=date('m', $date);
		// echo $a;
		$monthStr = $months[date('m', $date) - 1];


		echo "<hr>";
		echo "<div align=center><font size=6pt> $dayStr, $day $monthStr, $year </font><br</div>";
		echo "<hr>";

		// --------------------------- TIMELAPSE
		if (! $lite) {
			echo "<div align=left>";
			foreach ($files as $file) {
				// if (fnmatch("*.jpg", $file)) {
				if (fnmatch("timelapse_*", $file)) {
					// echo "
					//     <a href=\"$dir/$file\" >
					// 	<img src=\"$dir/$file\" width=320 height=240 />
					//     </a>
					//     ";
					// $cur_dir =exec("echo $file | cut -d. -f1");
					echo "
					<a href=\"displayTimelapsePhotos.php?year=$year&month=$month&day=$day&file=$file\" >
						<img src=\"$dir/$file/timelapse.gif\" width=320 height=240 />
					</a>
					";
				}
			}
			echo "</div>";
		}

		echo "<hr>";

		echo "<div align=left>";
		foreach ($files as $file) {
			if (fnmatch("timelapse_*", $file)) {
				// if (fnmatch("*.gif", $file)) {
				if ($lite) {
					echo "
					<a href=\"displayTimelapsePhotos.php?year=$year&month=$month&day=$day&file=$file\" >
						<img src=\"$dir/$file/latest.jpeg\" width=320 height=240 />
					</a>
					";
				} else {
					echo "
					<a href=\"$dir/$file/latest.jpeg\" >
						<img src=\"$dir/$file/latest.jpeg\" width=320 height=240 />
					</a>
					";

				}

				// $cur_dir =exec("echo $file | cut -d. -f1");
				// echo "
				// 	    <a href=\"displayAll.php?dir=$cur_dir\" >
				// 		    <img src=\"$dir/$file\" width=320 height=240 />
				// 	    </a>
				// 	    ";
			}
		}
		echo "</div>";

		// --------------------------- MOVEMENTS
		$heure = 24;
		$previousHeure = 24;
		foreach (array_reverse($files) as $file) {
			if ($file != '.' && $file != '..') {
				if (!fnmatch("timelapse_*", $file)) {
					// echo "$file";
					// if (file_exists("$dir/$file/complete.var")) {

					// if (file_exists("$dir/$file/objectDetected.var")) {
					// if (! $lite or file_exists("$dir/$file/objectDetected.var")) {
					if (! $lite or file_exists("$dir/$file/learningModelDone.var")) {

						// if (file_exists("$dir/$file/focusBrightnessDone.var")) {
						// if (file_exists("$dir/$file/learningModelDone.var")) {
							$heure = exec("echo $file | cut -c1-2");
							// $heure = exec("echo $file | cut -d_ -f2 | cut -c1-2");
							#$fileName =exec("echo $file | awk -F. '{print $1}'");

							if ($heure != $previousHeure) {
								$previousHeure = $heure;

								echo "<hr>";
								echo "<div><font size=6pt>$heure h</font><br</div>";
								echo "<hr>";
							}

							// $video =exec("echo $file | cut -d: -f2,3");
							// $video = "$dir/$file/clip.gif";
							// $video ="$video.mp4";

							#			echo "
							#			<video controls src=\"motion/$video\" poster=\"$file\" </video>
							#			";
							#				<source src=\"motion/$fileName.mp4\" type=\"video/mp4\" />
							#				<source src=\"motion/$fileName.avi\" type=\"video/avi\" />
							#			</video>
							#			";


							// $style = "";
							// $nbPhoto = exec("ls $dir/$file/cap_* | wc -l");
							// // echo "$nbPhoto";
							// if ($nbPhoto <= 4) {
							//     $style = "$style border-bottom:2px solid blue;";
							// }

							// if (file_exists("$dir/$file/nand.var")) {
							//     $style = "$style border-right:2px solid red;";
							// }

							// if (file_exists("$dir/$file/cap_1.jpg")) {
							//         if ($os_platform == "Android") {
							//             echo "
							// <a href=\"display.php?dir=$file\" >
							//     <img style=\"$style\" src=\"$dir/$file/diffMax.jpg\" width=320 height=240 />
							// </a>
							// ";
							//         } else {
							//     echo "
							// <a href=\"$video\" >
							//     <img style=\"$style\" src=\"$dir/$file/trace.jpeg\" width=320 height=240 />
							// </a>
							// ";
							// if (file_exists("$dir/$file/trace.jpg")) {

							// echo "
							//     <a href=\"displayAll.php?dir=$file\" >
							//         <img src=\"$dir/$file/trace.jpg\" width=320 height=240 />
							//     </a>
							//     ";

							echo "
								<a href=\"$dir/$file/video.webm\" >
									<img src=\"$dir/$file/trace.jpg\" width=320 height=240 />
								</a>
								";
							// }
							// }
							//         } else {
							//             echo "
							// <a href=\"$dir/$file/cap_0.jpg\" >
							//     <img src=\"$dir/$file/cap_0.jpg\" width=320 height=240 />
							// </a>
							// ";
							// }
						// }
					}
				}
			}
		}
		echo "<br>";
		echo "<br>";
		echo "<br>";
		echo "<br>";
		echo "<br>";
		echo "<br>";
		echo "<br>";
		echo "<br>";
		echo "<br>";
	}
?>
<?php

    // if (0 != filesize("log.txt")) {
    //     echo ("<object width=\"1700\" height=\"100\" type=\"text/plain\" data=\"log.txt\" border=\"0\" >
    //     </object>
    //     <br>");
    // }
    // echo file_get_contents('log.txt');


    // include('log.txt');
    // exec("cat /var/log/apache2/error.log | grep auth_basic:error");

    // $user_agent = $_SERVER['HTTP_USER_AGENT'];
    // $os_array     = array(
    //     '/windows nt 10/i'      =>  'Windows 10',
    //     '/windows nt 6.3/i'     =>  'Windows 8.1',
    //     '/windows nt 6.2/i'     =>  'Windows 8',
    //     '/windows nt 6.1/i'     =>  'Windows 7',
    //     '/windows nt 6.0/i'     =>  'Windows Vista',
    //     '/windows nt 5.2/i'     =>  'Windows Server 2003/XP x64',
    //     '/windows nt 5.1/i'     =>  'Windows XP',
    //     '/windows xp/i'         =>  'Windows XP',
    //     '/windows nt 5.0/i'     =>  'Windows 2000',
    //     '/windows me/i'         =>  'Windows ME',
    //     '/win98/i'              =>  'Windows 98',
    //     '/win95/i'              =>  'Windows 95',
    //     '/win16/i'              =>  'Windows 3.11',
    //     '/macintosh|mac os x/i' =>  'Mac OS X',
    //     '/mac_powerpc/i'        =>  'Mac OS 9',
    //     '/linux/i'              =>  'Linux',
    //     '/ubuntu/i'             =>  'Ubuntu',
    //     '/iphone/i'             =>  'iPhone',
    //     '/ipod/i'               =>  'iPod',
    //     '/ipad/i'               =>  'iPad',
    //     '/android/i'            =>  'Android',
    //     '/blackberry/i'         =>  'BlackBerry',
    //     '/webos/i'              =>  'Mobile'
    // );
    // foreach ($os_array as $regex => $value)
    //     if (preg_match($regex, $user_agent))
    // 	$os_platform = $value;






	if(isset($_GET['year']) and isset($_GET['month']) and isset($_GET['day'])) {
		$year=(string)$_GET['year'];
		$month=(string)$_GET['month'];
		$day=(string)$_GET['day'];
		echo "
<!DOCTYPE html>
<html>
<body>
";
		showDay($year, $month, $day);
		echo "
</body>
</html>";
	}
    // echo "
    // <hr>
    // <button type=\"submit\"  onClick=\"refreshPage()\" style=\"width: 100%;\"><img src=\"refresh.png\" width=\"50\" height=\"50\" style=\"background-color:light-gray;\" /></button>
    // <hr>
    // ";


?>
